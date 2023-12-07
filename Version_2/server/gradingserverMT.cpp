#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <filesystem>
#include <cassert>

using namespace std;
namespace fs = std::filesystem;

const int BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 4;
const int MAX_QUEUE = 50;

const char SUBMISSION_DIR[] = "./submission_dir/";
const char EXECUTABLE_DIR[] = "./executable_dir/";
const char OUTPUT_DIR[] = "./output_dir/";
const char COMPILER_ERR_DIR[] = "./compiler_err_dir/";
const char RUNTIME_ERR_DIR[] = "./runtime_err_dir/";
const char EXPECTED_OUTPUT[] = "./expected_output_dir/expected_output.txt";
const char COMPARE_OPT_DIR[] = "./comp_opt_dir/";

const char PASS_MSG[] = "PROGRAM RAN\n";
const char COMPILER_ERROR_MSG[] = "COMPILER ERROR\n";
const char RUNTIME_ERROR_MSG[] = "RUNTIME ERROR\n";
const char OUTPUT_ERROR_MSG[] = "OUTPUT ERROR\n";


int recv_file(string filename, int newsockfd) {
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    FILE *file = fopen(filename.c_str(), "wb");
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }

    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    if (recv(newsockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error receiving file size");
        fclose(file);
        return -1;
    }
    int file_size;
    memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));

    size_t bytes_read = 0;
    while (true)
    {
        size_t bytes_recvd = recv(newsockfd, buffer, BUFFER_SIZE, 0);
        bytes_read += bytes_recvd;
        if (bytes_read <= 0)
        {
            perror("Error receiving file data");
            fclose(file);
            return -1;
        }
        fwrite(buffer, 1, bytes_recvd, file);
        bzero(buffer, BUFFER_SIZE);
        if (bytes_read >= file_size)
            break;
    }
    fclose(file);
    return 0;
}

int grader(int newsockfd) { 
    int id = rand();
    string filename = SUBMISSION_DIR + to_string(id) +"file.cpp";
    string executable = EXECUTABLE_DIR + to_string(id) + "exec.o";
    string output_file = OUTPUT_DIR + to_string(id) + "output.txt";
    string compiler_err_file = COMPILER_ERR_DIR + to_string(id) + "comp_err.txt";
    string runtime_err_file = RUNTIME_ERR_DIR + to_string(id) + "run_err.txt"; 
    string comp_opt_file = COMPARE_OPT_DIR + to_string(id) + "comp_opt.txt";

    string compile_cmd = "g++    -o "+executable + " " + filename + "> /dev/null 2> "+compiler_err_file;
    string run_cmd = executable +" > "+output_file+" 2> " +runtime_err_file;
    string comp_opt_cmd = "diff " + output_file + " " + EXPECTED_OUTPUT +" > " + comp_opt_file;
    
    if(recv_file(filename, newsockfd) != 0) {
        close(newsockfd);
        return 0;
    } 

    int n = send(newsockfd, "File is received!", 17, 0);
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
    if (send(newsockfd, grader_result.c_str(), strlen(grader_result.c_str()), 0) == -1) {
        perror("Error sending result message");
        close(newsockfd);
        return 0;
    }

    if(grader_message.size() != 0) {
        ifstream result_file(grader_message);
        if(result_file.is_open()) {
        string file_output;
        while(getline(result_file, file_output)) {
            if(send(newsockfd, file_output.c_str(), strlen(file_output.c_str()), 0) == -1) {
            perror("Error sending result details");
            break;
            }
        }
        result_file.close();
        } else {
        perror("Error opening file");
        }
    }
    close(newsockfd);
    return 0;
}

void *worker(void *args) {
    int newsockfd = *(int *)args;
    grader(newsockfd);
    return (void *)NULL;
}
int main(int argc, char* argv[]) {
        // create socket for communication with client
        // and check if number of arguments equal to 2
        if(argc != 2) {
            cout << "Usage: " << argv[0] << " <PORT> " << endl;
            return 0;
        }
        
        int socket_fd; // Listen socket descriptor (half-socket)
        int client_socket_fd; // Client socket descript (full-socket)
        int PORT = atoi(argv[1]); // Port at which server listens

        // Socket address structure to hold the ip's of server and client
        struct sockaddr_in server_addr, client_addr;
        
        //Create Socket
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
         
        // Listen for incoming connection
        if (listen(socket_fd, MAX_QUEUE) < 0) {
            perror("Listening to Socket failed");
            exit(EXIT_FAILURE);
        }
        cout << "Server Listening on port " << PORT << "\n";

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
        }
        catch (fs::filesystem_error &e)
        {
            cerr << "Error creating directories: " << e.what() << "\n";
            close(socket_fd);
            return -1;
        }

        socklen_t client_addr_len = sizeof(client_addr);
        while(true) {
            if ((client_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len))<0) {
                perror("Error Accepting connection.");
                continue;
            }  
            int *sockhandler=(int *)malloc(sizeof(int));
            *sockhandler=client_socket_fd;
            pthread_t worker_thread;
            int rc = pthread_create(&worker_thread, NULL, worker, (void *)sockhandler);
            assert(rc == 0);
            rc = pthread_detach(worker_thread);
        }
        close(socket_fd);
	return 0;
}