#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

using namespace std;

string generateUniqueFileName() {
    int randomNum = rand() % 10000;
    time_t t;
    struct tm* now;
    char timestamp[20];
    time(&t);
    now = localtime(&t);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", now);
    return string(timestamp) + to_string(randomNum)  ;
}

void *grader(void *sockfd) {
            cout << "Inside Thread\n";
            int client_socket_fd = *(int *)sockfd;
            char buffer[2048];
            memset(buffer, 0, sizeof(buffer));
            string uniqueName = generateUniqueFileName();
            string sub_file_name = "./submission-dir/"+uniqueName+"program.cpp";
            string executable_name = "./executable-dir/"+uniqueName+"program.o";
            string run_error_name = "./run-error-dir/"+uniqueName+"run-error.txt";
            string comp_error_name = "./comp-error-dir/"+uniqueName+"comp-error.txt";
            string output_name = "./output-dir/"+uniqueName+"output.txt";
            string comparison_name = "./comparison-dir/"+uniqueName+"comparison.txt";
            string expected_output_name = "./expected-output-dir/expected-output.txt";
            ofstream outputFile(sub_file_name);
            if(!outputFile.is_open()) {
              perror("Error while evaluating submission.");
              pthread_exit(NULL);
            } 
            
            // Read data from network
            int valread = read(client_socket_fd , buffer, sizeof(buffer));
            if(valread > 0) {
              outputFile << buffer;
            }
            outputFile.close(); 
            
            // Compile the received program
            string compile_command = "g++ "+sub_file_name+" -o "+executable_name+" 2>"+comp_error_name;
            string executaion_command = executable_name+" 2>"+run_error_name+" 1>"+output_name;
            string evaluation_command = "diff "+expected_output_name+" "+output_name+" 1>"+comparison_name; 
            
            int compile_status = system(compile_command.c_str());
            if(compile_status != 0) {
              ifstream errorFile(comp_error_name);
              string compError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
              string result = "COMPILER ERROR \n"+compError;
              send(client_socket_fd, result.c_str(), result.size(), 0);
              close(client_socket_fd);
              pthread_exit(NULL);
            } 
            
            // Execute the compiled program
            int run_status = system(executaion_command.c_str());
            if(run_status != 0) {
              ifstream errorFile(run_error_name);
              string runError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
              string result = "RUNTIME ERROR \n"+runError;
              send(client_socket_fd, result.c_str(), result.size(), 0);
              close(client_socket_fd);
              pthread_exit(NULL);
            } 
            
            // Execute the diff command to actual output and expected output
            int result_status = system(evaluation_command.c_str());
	    ifstream resultFile(comparison_name); 
	    
	    if(result_status < 0 || !resultFile.is_open()) {
	      perror("Error while evaluating submission");
	      close(client_socket_fd);
	      pthread_exit(NULL);
	    }
	    string result;
	    
	    // If there is no difference between actual output and expected output return PASS else return output of diff
	    if(resultFile.peek() == ifstream::traits_type::eof()) {
		    result = "PASS";
	    } else {
	            string outputError((istreambuf_iterator<char>(resultFile)), istreambuf_iterator<char>());
		    result = "OUTPUT ERROR \n" + outputError;
		    
	    } 
	    send(client_socket_fd, result.c_str(), result.size(), 0);
	    close(client_socket_fd);
	    cout << "Thread completed\n";
	    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
        // create socket for communication with client
        if(argc != 2) {
            cout << "Usage: " << argv[0] << " <PORT> " << endl;
        }
        
        int socket_fd; 
        struct sockaddr_in address;
        int address_length;
        
        //Create Socket
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("Socket creation failed!");
            exit(EXIT_FAILURE);
        } 
        int PORT = atoi(argv[1]);
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );
        address_length = sizeof(address); 
        
        // Bind Socket
        if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address))<0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }
         
        // Listen for incoming connection
        if (listen(socket_fd, 1000) < 0) {
            perror("Listening to Socket failed");
            exit(EXIT_FAILURE);
        }
        
        cout << "Listening on port number: "<< PORT << endl;;
        while(true) {
            int client_socket_fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&address_length);
            if (client_socket_fd <0) {
                perror("Error Accepting connection.");
                continue;
            }
            pthread_t grader_thread;
            cout << "Starting new Thread" << endl;
	    if (pthread_create(&grader_thread, NULL, &grader, &client_socket_fd) != 0)
            {
                    perror("Failed to create thread");
                    return -1;
            }
            
        }
	return 0;
}
