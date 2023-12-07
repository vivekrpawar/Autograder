#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <filesystem>
#include <cassert> 
#include <deque>
#include <unordered_map> 
#include "file_utils.h"
#include "server_utils.h"

using namespace std;
namespace fs = std::filesystem;

deque<string> grader_queue;
deque<string> status_queue;
unordered_map<string, int> map;

pthread_mutex_t grader_queue_lock;
pthread_mutex_t status_queue_lock;
pthread_mutex_t map_lock;
pthread_cond_t grader_queue_ready; 

int get_queue_pos(string req_id) {
    int count = 0;
    for(auto it = grader_queue.cbegin();it!= grader_queue.cend();it++)
    {
        cout << *it << " ";
        if(req_id.compare(*it) == 0){
            return count+1;
        }
        count ++;
    }
    return -1;
}

int status_req_handler(int newsockfd) {
    char req_id_buff[BUFFER_SIZE];
    bzero(req_id_buff, sizeof(req_id_buff));
    if (recv(newsockfd, req_id_buff, sizeof(req_id_buff), 0) == -1)
    {
        perror("Error receiving file size");
        close(newsockfd);
        return -1;
    }   
    string req_id(req_id_buff); 
    int req_status;
    pthread_mutex_lock(&map_lock); 
    if(map.find(req_id) == map.end()) req_status = -1;
    else req_status = map[req_id];
    pthread_mutex_unlock(&map_lock);
    string res_msg;
    string res_file;
    int queuePos;
    switch(req_status) {
        case 1:
            queuePos = get_queue_pos(req_id);
            res_msg = "Your grading request ID " + req_id +" has been accepted. It is currently at  position " + to_string(queuePos) + " in the queue.\n";
            break;
        case 2:
            res_msg =  "Your grading request ID " + req_id + " has been accepted and is currently being processed.\n";
            break;
        case 3:
            res_msg = "Your grading request ID " + req_id + " processing is done, here are the results\n";
            res_file = RESULT_DIR+req_id+"result.txt";
            break;
        case -1:
            res_msg = "Grading request " + req_id + " not found. Please check and resend your request ID or re-send your original grading request.\n";
            break;
        default:
            break;
    }

    if (send(newsockfd, res_msg.c_str(), strlen(res_msg.c_str()), 0) == -1) {
        perror("Error sending result message");
        close(newsockfd);
        return -1;
    } 
    if(res_file.size() == 0) {
        close(newsockfd);
        return 0;
    }
    ifstream result_file(res_file);
    if(result_file.is_open()) { 
        string file_output;
        while(getline(result_file, file_output)) { 
            file_output = file_output+"\n"; 
            if(send(newsockfd, file_output.c_str(), strlen(file_output.c_str()), 0) == -1) {
                perror("Error sending result details");
                break;
            } 
        } 
        result_file.close();
    } else {
        perror("Error opening file");
        close(newsockfd);
        return -1;
    }
    string log_entry = req_id+" "+to_string(-1)+"\n";
    pthread_mutex_lock(&map_lock);
    map.erase(req_id);
    ofstream log_file;
    log_file.open(LOG_FILE, ios::app);  
    if(log_file.is_open()) { 
        log_file << log_entry;
        log_file.close(); 
    } else {
        perror("Error writting logs");
    }
    pthread_mutex_unlock(&map_lock);
    close(newsockfd);
    return 0;
}

void *grader_thread(void *args) {
    while(true) {
        int newsockfd;
        pthread_mutex_lock(&grader_queue_lock);
        while(grader_queue.empty()) {
            pthread_cond_wait(&grader_queue_ready, &grader_queue_lock);
        }
        string req_id = grader_queue.front();
        grader_queue.pop_front();
        pthread_mutex_unlock(&grader_queue_lock);

        // Change the request status in map and logfile
        string log_entry;
        log_entry = req_id+" "+to_string(2)+"\n";
        pthread_mutex_lock(&map_lock);
        map[req_id] = 2;
        ofstream log_file;
        log_file.open(LOG_FILE, ios::app);  
        if(log_file.is_open()) {   
            log_file << log_entry;
            log_file.close(); 
        } else {
            perror("Error writting logs");
        }
        pthread_mutex_unlock(&map_lock);
        string filename = SUBMISSION_DIR + req_id +"file.cpp";
        string executable = EXECUTABLE_DIR + req_id+ "exec.o";
        string output_file = OUTPUT_DIR + req_id + "output.txt";
        string compiler_err_file = COMPILER_ERR_DIR + req_id + "comp_err.txt";
        string runtime_err_file = RUNTIME_ERR_DIR + req_id + "run_err.txt"; 
        string comp_opt_file = COMPARE_OPT_DIR + req_id + "comp_opt.txt";

        string compile_cmd = "g++    -o "+executable + " " + filename + "> /dev/null 2> "+compiler_err_file;
        string run_cmd = executable +" > "+output_file+" 2> " +runtime_err_file;
        string comp_opt_cmd = "diff " + output_file + " " + EXPECTED_OUTPUT +" > " + comp_opt_file;
        string grader_result = "";
        string grader_message = "";
        if(system(compile_cmd.c_str()) != 0) {
            grader_result = COMPILER_ERROR_MSG;
            grader_message = compiler_err_file;
        }else if(system(run_cmd.c_str()) != 0) {
            grader_result = RUNTIME_ERROR_MSG;
            grader_message = runtime_err_file;
        }else if(system(comp_opt_cmd.c_str()) != 0) {
            grader_result = OUTPUT_ERROR_MSG;
            grader_message = comp_opt_file;
        }else {
            grader_result = PASS_MSG;
        }
        string resp_file_name = RESULT_DIR+req_id+"result.txt";
        ofstream resp_file(resp_file_name);
        resp_file << grader_result;
        if(grader_message.size() != 0) {
            ifstream result_file(grader_message);
            if(result_file.is_open()) {
                string file_output;
                while(getline(result_file, file_output)) {
                    resp_file << file_output;
                }
                result_file.close();
                resp_file.close();
            } else {
                perror("Error opening file");
            }
        }
        log_entry = req_id+" "+to_string(3)+"\n";
        pthread_mutex_lock(&map_lock);
        map[req_id] = 3;
        ofstream log_file2;
        log_file2.open(LOG_FILE, ios::app);  
        if(log_file2.is_open()) { 
            log_file2 << log_entry;
            log_file2.close(); 
        } else {
            perror("Error writting logs");
        }
        pthread_mutex_unlock(&map_lock);
    }
    return (void *)NULL;
}

void  worker(int newsockfd) { 
    char req_type_char[MAX_FILE_SIZE_BYTES];
    bzero(req_type_char, sizeof(req_type_char));
    // Accept request type (new request/ status check request) from the user
    if (recv(newsockfd, req_type_char, sizeof(req_type_char),0) == -1)
    {   
        perror("Error receiving file size");
        close(newsockfd);
        return;
    }
    // Convert Request type from character array to int
    int req_type;
    memcpy(&req_type, req_type_char, sizeof(req_type_char));
    if(req_type == 0) {
        // Handle new request
        // Generate a unique request id
        string req_id = generateUniqueFileName();
        string filename = SUBMISSION_DIR + req_id +"file.cpp"; // Submission file where the client program will be stores
        
        //Receive file from the client
        if(recv_file(filename, newsockfd) != 0) {
            close(newsockfd);
            return;
        } 

        // after acception the file push the request int the grader queue, since the queue is share 
        // we need to hangle concurrency so we will use lock and signal mechanism, signal will wake up the
        // grader thread waiting on grader_queu_lock
        pthread_mutex_lock(&grader_queue_lock);
        grader_queue.push_back(req_id);
        pthread_mutex_unlock(&grader_queue_lock);
        pthread_cond_signal(&grader_queue_ready);

        // Update the request status in the map as request accepted and send responce to the client with
        // request id 
        string log_entry = req_id+" "+to_string(1)+"\n";
        pthread_mutex_lock(&map_lock);
        map[req_id] = 1;
        ofstream log_file;
        log_file.open(LOG_FILE, ios::app); 
        if(log_file.is_open()) { 
            log_file << log_entry;
            log_file.close(); 
        } else {
            perror("Error writtng logs");
        }
        pthread_mutex_unlock(&map_lock);

        // Send request id and responce ot client
        string accept_msg = "Your grading request ID "+ req_id + " has been accepted.";
        if(send(newsockfd, accept_msg.c_str(), strlen(accept_msg.c_str()), 0) == -1) {
            perror("Error sending result details");
            close(newsockfd);
            return;
        }
        close(newsockfd);
    } else {
        // Handle status request
        if(status_req_handler(newsockfd) == -1) {
            perror("Error checking status!");
        }
    }
}


int main(int argc, char* argv[]) {
        // create socket for communication with client
        // and check if number of arguments equal to 2
        if(argc != 3) {
            cout << "Usage: " << argv[0] << " <PORT> <thread_pool_size>" << endl;
            return 0;
        }
        cout << "Server is booting up...\n";
        int socket_fd; // Listen socket descriptor (half-socket)
        
        int PORT = atoi(argv[1]); // Port at which server listens

        // Socket address structure to hold the ip's of server and client
        struct sockaddr_in server_addr, client_addr;
         
        // Creating a socket and of address famity inet (ip) and and socket type tcp
        if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation failed!");
            exit(EXIT_FAILURE);
        } 
        
        bzero((char *)&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET; // Address family of interner
        server_addr.sin_addr.s_addr = INADDR_ANY; // Any ip address
        server_addr.sin_port = htons(PORT); // port number conveted from host order to network order
        
        // Bind Socket
        if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }
         

        // Create respective directoreis if not present
        try
        {
            if (!fs::exists(SUBMISSION_DIR))
                fs::create_directory(SUBMISSION_DIR);
            if (!fs::exists(EXECUTABLE_DIR))
                fs::create_directory(EXECUTABLE_DIR);
            if (!fs::exists(OUTPUT_DIR))
                fs::create_directory(OUTPUT_DIR);
            if (!fs::exists(COMPILER_ERR_DIR))
                fs::create_directory(COMPILER_ERR_DIR);
            if (!fs::exists(RUNTIME_ERR_DIR))
                fs::create_directories(RUNTIME_ERR_DIR);
            if (!fs::exists(COMPARE_OPT_DIR))
                fs::create_directories(COMPARE_OPT_DIR);
            if (!fs::exists(RESULT_DIR))
                fs::create_directories(RESULT_DIR);
            if (!fs::exists(LOG_DIR)){
                fs::create_directories(LOG_DIR);
                ofstream ofs(LOG_FILE);
                ofs.close();
            }
        }
        catch (fs::filesystem_error &e)
        {
            cerr << "Error creating directories: " << e.what() << "\n";
            close(socket_fd);
            return -1;
        }

        pthread_mutex_init(&grader_queue_lock, NULL);
        pthread_mutex_init(&map_lock, NULL);
        pthread_cond_init(&grader_queue_ready, NULL);

        pthread_mutex_lock(&map_lock);
        ifstream log_file(LOG_FILE); 
        if(log_file.is_open()) {
            string log_entry;
            while(getline(log_file, log_entry)) {
                int spaceInd = log_entry.find(" ");
                string req_id = log_entry.substr(0, spaceInd);
                int req_status = atoi(log_entry.substr(spaceInd+1).c_str());
                if(req_status != -1) map[req_id] = req_status;
                else map.erase(req_id);
            }
            log_file.close(); 
        } else {
            perror("Error reading logs");
        }
        pthread_mutex_unlock(&map_lock);
        
        int num_of_threads = atoi(argv[2]);

        // Creating a threadpool of size equal to num_of_threads
        pthread_t worker_threads[num_of_threads];

        for(int i = 0; i < num_of_threads; i++) {
            if((pthread_create(&worker_threads[i], NULL, grader_thread, NULL)) < 0) {
                cerr << "Error creating thread\n";
            }
        }


        // Listen for incoming connection
        if (listen(socket_fd, MAX_QUEUE) < 0) {
            perror("Listening to Socket failed");
            exit(EXIT_FAILURE);
        }
        sleep(3);
        cout << "Server Listening on port " << PORT << "\n";
        socklen_t client_addr_len = sizeof(client_addr);
        int req_count = 0;
        while(true) {
            int client_socket_fd; // Client socket descript (full-socket)
            if ((client_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len))<0) {
                perror("Error Accepting connection.");
                continue;
            }  
            worker(client_socket_fd);
        }
        close(socket_fd);
	return 0;
}

