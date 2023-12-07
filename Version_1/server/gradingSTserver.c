/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>

const int BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 4;

void error(char *msg) {
  perror(msg);
  exit(1);
}

//Utility Function to receive a file of any size to the grading server
int recv_file(int sockfd, char* file_path)
//Arguments: socket fd, file name (can include path) into which we will store the received file
{
    char buffer[BUFFER_SIZE]; //buffer into which we read  the received file chars
    bzero(buffer, BUFFER_SIZE); //initialize buffer to all NULLs
    FILE *file = fopen(file_path, "wb");  //Get a file descriptor for writing received data into file
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }

	
	//buffer for getting file size as bytes
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    //first receive  file size bytes
    if (recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error receiving file size");
        fclose(file);
        return -1;
    }
   
    int file_size;
    //copy bytes received into the file size integer variable
    memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));
    
    //some local printing for debugging
    printf("File size is: %d\n", file_size);
    
    //now start receiving file data
    size_t bytes_read = 0, total_bytes_read =0;;
    while (true)
    {
    	  //read max BUFFER_SIZE amount of file data
        bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);

        //total number of bytes read so far
        total_bytes_read += bytes_read;

        if (bytes_read <= 0)
        {
            perror("Error receiving file data");
            fclose(file);
            return -1;
        }

		//write the buffer to the file
        fwrite(buffer, 1, bytes_read, file);

	// reset buffer
        bzero(buffer, BUFFER_SIZE);
        
       //break out of the reading loop if read file_size number of bytes
        if (total_bytes_read >= file_size)
            break;
    }
    fclose(file);
    return 0;
}




int main(int argc, char *argv[]) {
  int sockfd, //the listen socket descriptor (half-socket)
   newsockfd, //the full socket after the client connection is made
   portno; //port number at which server listens

  socklen_t clilen; //a type of an integer for holding length of the socket address
  char buffer[256]; //buffer for reading and writing the messages
  struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
 // This is just a holder right now, note no port number either. It needs a 'bind' call


  if (sockfd < 0)
    error("ERROR opening socket");

 
  bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros
  
  serv_addr.sin_family = AF_INET; // Address Family of INTERNET
  serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address. 

//Port number is the first argument of the server command
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

  /* bind the socket created earlier to this port number on this machine 
 First argument is the socket descriptor, second is the address structure (including port number).
 Third argument is size of the second argument */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 1); // 1 means 1 connection requests can be in queue. 
  //now server is listening for connections


  clilen = sizeof(cli_addr);  //length of struct sockaddr_in


  /* accept a new request, now the socket is complete.
  Create a newsockfd for this socket.
  First argument is the 'listen' socket, second is the argument 
  in which the client address will be held, third is length of second
  */
  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

 
 //Call the recv_file utility function, argument is the filename that the received file will be stored as
		
		if (recv_file(newsockfd, "file.cpp") != 0)
    {
         close(newsockfd);
         continue;
    }

    
  //some local printing for debugging 
    printf("Here is the file: \n");
    system("cat file.cpp");

    /* send reply to client 
    First argument is the full socket, second is the string, third is   the
  number of characters to write. Fourth are some flags set to 0 */

// Some progress response
    n = send(newsockfd, "I got your code file for grading\n", 33, 0);
    
  //Compile the code file, send back 'COMPILER ERROR' message to client if compile failed
  //Write compiler output to a local file compiler_err.txt
  //NO NEED TO SEND COMPILER OUTPUT BACK TO CLIENT. 
  
    if (system("g++ -o prog file.cpp 2> compiler_err.txt") !=0 ) {
	
	    	n = send(newsockfd, "COMPILER ERROR\n", 16, 0);
	    	if (n < 0)
		      error("ERROR writing to socket");	
 		    close(newsockfd);	
 		} //here if no compiler error
  //Run the executable, send back 'RUNTIME ERROR' message to client  if runtime error
  //Write runtime error message to a local file runtime_err.txt, program output to a file out.txt
  //NO NEED TO SEND RUNTIME ERROR MESSAGE BACK TO CLIENT. 
  
 		else if (system("./prog > out.txt 2> runtime_err.txt") !=0 ) { 			
 				
	    	n = send(newsockfd, "RUNTIME ERROR\n", 15, 0);
	    	if (n < 0)
		      error("ERROR writing to socket");	
 		    close(newsockfd);	
 		 }  //here if no runtime error
  //Write a message "PROGRAM RAN" to client 
  //NO NEED TO SEND PROGRAM OUTPUT BACK TO CLIENT. 
 		 else { 		 
 		 		n = send(newsockfd, "PROGRAM RAN\n", 13, 0);
	    	if (n < 0)
		      error("ERROR writing to socket");	
 		    close(newsockfd);	
		}
    
    
    close(newsockfd);
    if (n < 0)
      error("ERROR writing to socket");
    }
    
    return 0;
  }

