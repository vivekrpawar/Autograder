#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>
#include <stdlib.h>
#include <stdbool.h>
#include <chrono>
#include <pthread.h>
#include <iostream>

using namespace std;
using namespace std::chrono;

const int BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 4;
const int MAX_TRIES = 5;

struct sockaddr_in serv_addr;
char file_path[256];
int error_count = 0;
// Utility Function to send a file of any size to the grading server
int send_file(int sockfd, char *file_path)
// Arguments: socket fd, file name (can include path)
{
    char buffer[BUFFER_SIZE];            // buffer to read  from  file
    bzero(buffer, BUFFER_SIZE);          // initialize buffer to all NULLs
    FILE *file = fopen(file_path, "rb"); // open the file for reading, get file descriptor
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }

    // for finding file size in bytes
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file); 

    // Reset file descriptor to beginning of file
    fseek(file, 0L, SEEK_SET);

    // buffer to send file size to server
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    // copy the bytes of the file size integer into the char buffer
    memcpy(file_size_bytes, &file_size, sizeof(file_size));

    // send file size to server, return -1 if error
    if (send(sockfd, &file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error sending file size");
        fclose(file);
        return -1;
    }

    // now send the source code file
    while (!feof(file)) // while not reached end of file
    {

        // read buffer from file
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);

        // send to server
        if (send(sockfd, buffer, bytes_read + 1, 0) == -1)
        {
            perror("Error sending file data");
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

void *submit(void *args)
{

    // create the socket file descriptor
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error_count++;
        perror("Socket creation failed");
        return (void *)NULL;
    }

    int tries = 0;
    while (true)
    {
        // connect to the server using the socket fd created earlier
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
            break;
        sleep(1);
        tries += 1;
        if (tries == MAX_TRIES)
        {
            cout << "Server not responding\n";
            return (void *)NULL;
        }
    }

    // send the file by calling the send file utility function
    if (send_file(sockfd, file_path) != 0)
    {
        cout << "Error sending source file\n";
        error_count++;
        close(sockfd);
        return (void *)NULL;
    };

    cout << "Code sent for grading, waiting for response\n";
    size_t bytes_read;
    // buffer for reading server response
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    while (true)
    {
        // read server response
        bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0)
            break;
        cout << buffer;
        memset(buffer, 0, BUFFER_SIZE);
    }

    // close socket file descriptor
    close(sockfd);
    return (void *)NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        perror("Usage: ./submit <serverIP> <port>  <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds> <timeout-seconds>\n");
        return -1;
    }

    char server_ip[40], ip_port[40];
    int server_port;

    // get the arguments into the corresponding variables
    strcpy(server_ip, argv[1]);
    server_port = atoi(argv[2]);

    strcpy(file_path, argv[3]);
    int loopNum = atoi(argv[4]);
    int sleepTime = atoi(argv[5]);
    int timeout_seconds = atoi(argv[6]);

    // setup the server side variables
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr.s_addr);

    int succ_response = 0;
    int timeoutcount = 0;
    uint64_t total_res_time = 0;
    uint64_t loop_st = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    for (int i = 0; i < loopNum; i++)
    {
        struct timespec ts;
        pthread_t worker;

        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            continue;
        }
        ts.tv_sec += timeout_seconds;
        uint64_t req_st = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

        pthread_create(&worker, NULL, submit, NULL);
        if (pthread_timedjoin_np(worker, NULL, &ts) != 0)
        {
            pthread_detach(worker);
            timeoutcount++;
            continue;
        }
        else
        {
            uint64_t req_et = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            total_res_time += (req_et - req_st); 
            succ_response++;
        }

        sleep(sleepTime);
    }
    uint64_t loop_et = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    uint64_t total_loop_time = loop_et - loop_st;
    float total_errors = loopNum - succ_response - timeoutcount;

    float avg_rt = (float)total_res_time / succ_response;
    float request_sent_rt = (float)loopNum * 1000 / total_loop_time;
    float goodput = (float)succ_response * 1000 / total_loop_time;
    float timeout_rt = (float)timeoutcount * 1000 / total_loop_time;
    float error_rt = (float)total_errors * 1000 / total_loop_time;

    cout << "Total response time: " << total_res_time << " ms.\n";
    cout << "Average response time: " << avg_rt << " ms.\n";
    cout << "Successful responses: " << succ_response << "\n";
    cout << "Time to complete loop: " << total_loop_time << " ms.\n";
    cout << "Request sent rate: " << request_sent_rt << "\n";
    cout << "Throughput: " << goodput << "\n";
    cout << "Timeout rate: " << timeout_rt << "\n";
    cout << "Total timeouts: " << timeoutcount << "\n";
    cout << "Error rate: " << error_rt << "\n";
    return 0;
}
