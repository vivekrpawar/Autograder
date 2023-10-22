#include<iostream>
#include<fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]){
    int MAX_TRIES = 5;
    struct sockaddr_in address;
    int address_length;
    
    // Check whether input matches the specific format
    
    if(argc != 6) { 
      cout << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds> <timeout-seconds>" << endl;
      return 0;
    }
    string socket_addr(argv[1]);
    int loopNum = atoi(argv[3]);
    int sleep_duration = atoi(argv[4]);
    // Extract server port number and ip address
    int pos = socket_addr.find(':'); 
    string port = socket_addr.substr(pos+1);
    string server_address = socket_addr.substr(0, pos); 
    
    // Extract source program file name
    string program_name(argv[2]);
    address_length = sizeof(address);
    int PORT = stoi(port);
    address.sin_family = AF_INET;
    address.sin_port = htons( PORT );
    address_length = sizeof(address);
    uint64_t total_rt = 0;
    int i = loopNum;
    int successful_response = 0;
    uint64_t loop_st = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    int timeoutcount=0;
    loop1:
    while(i-- > 0) {
    // Socket creation
    
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) { 
          perror("Socket failed.\n");
          exit(EXIT_FAILURE);
        } 
        
        // Convert server ip address to binary and store it to sin_add field of address structure
        if(inet_pton(AF_INET, server_address.c_str(), &address.sin_addr)<=0) { 
          cout<<"Invalid address\n";
          exit(EXIT_FAILURE);
        }
        
        struct timeval timeout;
        timeout.tv_sec = atoi(argv[5]);  // 10-second timeout
        timeout.tv_usec = 0;
        // Set the receive timeout
        setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); 
	if(socket_fd == -1) {
		perror("Socket reading failed");
		return -1;
	}
        cout << "loop count " << i << endl;
        // Connecting with socket
        int tries=0;
        while(true) { 
		if(connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) == 0) {
			break;
		}
		sleep(1);
		tries +=1;
		cout << "Tries: " << tries << endl;
		if(tries == MAX_TRIES) {
			cout << "Server not responding\n";
			if (errno == EWOULDBLOCK || errno == EAGAIN){ // Handle the timeout error
                                  timeoutcount++;
			          cerr<<"Timeout error. No response received"<<endl;
	                }else{
                                  cerr << "Error receiving data." << endl;
                        }
                        close(socket_fd);
                        continue loop1;
			return -1;
		}

	}
	ifstream programFile(program_name);
    	if(!programFile.is_open()){
      		cout<<"Unable to load file.\n";
      		exit(EXIT_FAILURE);
    	}
    	string sendProgram((istreambuf_iterator<char>(programFile)), istreambuf_iterator<char>());
    	uint64_t st_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    	int bytes_sent = send(socket_fd, sendProgram.c_str(), sendProgram.size(), 0);  
    	if (bytes_sent == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN){ // Handle the timeout error
                        timeoutcount++;
			cerr<<"Timeout error. No response received"<<endl;
	    }else{
                        cerr << "Error receiving data." << endl;
            }
            close(socket_fd);
            continue;
        }
        
    	ifstream file(program_name);
    
    	// Receiving the response from server
    	char buffer[1024] = {};
    	int valread = 0;  
    	while((valread = read(socket_fd , buffer, 1024)) > 0) {
      	  cout <<buffer << endl;
    	}
   	uint64_t et_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count(); 
    	if (valread == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN){ // Handle the timeout error
                        timeoutcount++;
			cerr<<"Timeout error. No response received"<<endl;
	    }else{
                        cerr << "Error receiving data." << endl;
            }
            close(socket_fd);
            continue;
        }
        else
        {
          // Check for successful response 
        // If it's successful, increment the counter
            successful_response++;
        }
         
    	cout << "bytes read: " << valread << endl;
    	total_rt += et_ms - st_ms;
    	close(socket_fd);
	sleep(sleep_duration);
    }
    uint64_t loop_et = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    float total_loop_time = loop_et - loop_st; 
    float total_errors = loopNum - successful_response - timeoutcount;
    
    // Calculation all the rates
    float avg_rt = (float)total_rt/loopNum; 
    float request_sent_rt = (float)loopNum*1000/total_loop_time;
    float goodput = (float)successful_response*1000/total_loop_time;
    float timeout_rt = (float)timeoutcount*1000/total_loop_time;
    float error_rt = (float)total_errors*1000/total_loop_time;
     
    cout << "Average response time: "<< avg_rt << " ms."<< endl;
    cout << "Successful responses: "<< successful_response << endl;
    cout << "Time to complete loop: "<< loop_et - loop_st << " ms."<< endl;
    cout << "Request sent rate: " << request_sent_rt << endl;
    cout << "Successful request rate: " << goodput << endl;
    cout << "Timeout rate: " << timeout_rt << endl;
    cout << "Error rate: " << error_rt << endl;
    return 0;
}
