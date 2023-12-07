
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <chrono>
#include <pthread.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace std::chrono;

const int BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 4;
const int MAX_TRIES = 5;

struct sockaddr_in serv_addr;
char file_path[256];
char requestID[50];
int error_count = 0;


bool isInteger(const std::string& s) {
    try {
        size_t pos;
        std::stoll(s, &pos);
        return pos == s.length();
    } catch (std::invalid_argument&) {
        return false;
    } catch (std::out_of_range&) {
        return false;
    }
}


// Utility Function to send a file of any size to the grading server
int send_file(int clientSocket, char *file_path)
// Arguments: socket fd, file name (can include path)
{
    char buffer[BUFFER_SIZE];            // buffer to read  from  file
    bzero(buffer, BUFFER_SIZE);          // initialize buffer to all NULLs
    FILE *file = fopen(file_path, "rb"); // open the file for reading, get file descriptor
    if (!file)
    {
        printf("Error opening file");
        return -1;
    }

    // for finding file size in bytes
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    cout << "File size is: " << file_size << "\n";

    // Reset file descriptor to beginning of file
    fseek(file, 0L, SEEK_SET);

    // buffer to send file size to server
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    // copy the bytes of the file size integer into the char buffer
    memcpy(file_size_bytes, &file_size, sizeof(file_size));

    // send file size to server, return -1 if error
    if (send(clientSocket, &file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        printf("Error sending file size");
        fclose(file);
        return -1;
    }

    // now send the source code file
    while (!feof(file)) // while not reached end of file
    {

        // read buffer from file
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);

        // send to server
        if (send(clientSocket, buffer, bytes_read + 1, 0) == -1)
        {
            printf("Error sending file data");
            fclose(file);
            return -1;
        }

        // clean out buffer before reading into it again
        bzero(buffer, BUFFER_SIZE);
    }
    // close file
    fclose(file);
    return 0;
} 

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("Usage: submit  <new=0|status=1> <serverIP> <port> <sourceCodeFileTobeGraded|requestID>\n");
        return -1;
    }
    char server_ip[40];
    int server_port;

    // get the arguments into the corresponding variables
    strcpy(server_ip, argv[2]);
    server_port = atoi(argv[3]);


    // timeout only while sending file
    int timeout_seconds = 10;

    // setup the server side variables
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr.s_addr);

    int choice = atoi(argv[1]);
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if ((choice == 0 && isInteger(argv[4])) ||  (choice == 1 && !isInteger(argv[4]))){
        printf("Usage: submit  <new=0|status=1> <serverIP> <port> <sourceCodeFileTobeGraded|requestID>\n");
        return -1;
    }
    if (clientSocket == -1)
    {
        printf("Socket creation failed");
        return 0;
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        cerr << "Error connecting to server." << endl;
        close(clientSocket);
        return 1;
    }

    //send choice to the server 
    char reqtype[MAX_FILE_SIZE_BYTES];
    memcpy(reqtype, &choice, sizeof(choice));
    if (send(clientSocket, &choice, sizeof(int), 0) == -1) {
        printf("Error sending choice\n");
        close(clientSocket);
        return -1;
    }


    if (choice == 0)
    {
        
        //choice = '0'; 
        strcpy(file_path, argv[4]);
        
        // send the file by calling the send file utility function
        if (send_file(clientSocket, file_path) != 0)
        {
            cout << "Error sending source file\n";
            close(clientSocket);
            return -1;
        }

        cout << "Code sent for grading, waiting for Request ID\n";

        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int bytes_read = recv(clientSocket, &buffer, sizeof(buffer), 0);
        if (bytes_read <= 0) {
            cerr << "Error receiving requestID or connection closed\n";
        } else {
            cout << buffer << "\n";
        }

    }
    else if (choice == 1)
    {
        string requestID = argv[4]; 
        cout << requestID << "\n";
        if (send(clientSocket, requestID.c_str(), strlen(requestID.c_str()), 0) == -1) {
            printf("Error sending request id\n");
            close(clientSocket);
            return -1;
        }
        
        //read server response
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE); 
        printf("Server Response: ");
        while(true) {
            size_t bytes_read = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if(bytes_read == 0) {
                break;
            } else if(bytes_read == -1) {
                perror("Error receiving result details");
                break;
            }
            cout << buffer;
            bzero(buffer, BUFFER_SIZE);
        }        
    }
    close(clientSocket);
    return 0;
}
