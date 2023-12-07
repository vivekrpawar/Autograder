/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <assert.h>
#include <pthread.h>

const int BUFFER_SIZE = 1024;
const int MAX_FILE_SIZE_BYTES = 4;


pthread_mutex_t queueMutex;
pthread_cond_t queueReady;
const int maxQueueSize = 50;
int* gradingqueue;  // Declare a pointer

void error(char *msg) {
  perror(msg);
  exit(1);
}


int front = -1, rear = -1;

// Check if the queue is full
int isFull() {
  if ((front == rear + 1) || (front == 0 && rear == maxQueueSize - 1)) return 1;
  return 0;
}

// Check if the queue is empty
int isEmpty() {
  if (front == -1) return 1;
  return 0;
}

// Adding an element
void enQueue(int element) {
  if (!isFull())
  {
    if (front == -1) front = 0;
    rear = (rear + 1) % maxQueueSize;
    gradingqueue[rear] = element;
  }
}

// Removing an element
int deQueue() {
  int element;
  if (!isEmpty()) {
    element = gradingqueue[front];
    if (front == rear) {
      front = -1;
      rear = -1;
    } 
    // Q has only one element, so we reset the 
    // queue after dequeing it. ?
    else {
      front = (front + 1) % maxQueueSize;
    }
    return (element);
  }
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

/*Utility Function that takes some number as an ID, and source code file name (programFile) and executable file name (execFile) as arguments, and creates a compile command such that executable file gets the name in execFile. and compiler error output if any goes into a file compiler_err<id>.txt

E.g. if the call is :

comp_comm = compile_command (2123, file2123.cpp, prog2123)

then it till return the string (comp_comm will get this string):

"gcc -o prog2123 file2123.cpp 2> compiler_err2123.txt"

YOU SHOULD NOT TRY TO CHANGE THIS FUNCTION. JUST USE IT AS IS.

*/
char* compile_command(int id, char* programFile, char* execFile) {

  char *s;
  char s1[20];
  
  s = malloc (200*sizeof(char));
  memset(s, 0, sizeof(s));
  memset(s1, 0, sizeof(s1));
  strcpy(s, "g++ -o ");
  strcat(s, execFile);
  strcat(s, "  ");
  strcat(s, programFile);
  strcat(s, " 2> compiler_err");
 	sprintf(s1, "%d", id);	
 	strcat(s, s1);	
  strcat(s, ".txt");
  printf("%s\n",s);
  return s;
}
    
/*Utility Function that takes some number as an ID, and executable file name (execFile) as arguments, and creates a run command such that output file gets a name such as out<id>.txt. and runtime error output if any goes into a file runtime_err<id>.txt

E.g. if the call is :

run_comm = run_command (2123, prog2123)

then it till return the string (run_comm will get this string):

"./prog2123 > out2123.txt 2> runtime_err2123.txt"

YOU SHOULD NOT TRY TO CHANGE THIS FUNCTION. JUST USE IT AS IS.

*/

char* run_command(int id, char* execFileName) {

  char *s;
  char s1[20];
  
  s = malloc (200*sizeof(char));
  memset(s, 0, sizeof(s));
  memset(s1, 0, sizeof(s1));
 	sprintf(s1, "%d", id);	  

  strcpy(s, "./");
  strcat(s, execFileName);
  strcat(s, " > out");
 	strcat(s, s1);	
  strcat(s, ".txt");
  strcat(s, " 2> runtime_err");
 	strcat(s, s1);	
 	strcat(s, ".txt");	
  printf("%s\n",s);
  return s;
}

/* Utility function that given a number as an argument returns a  source code file name of the format file<id>.cpp

e.g. if the call is

fname = makeProgramFileName (2123);

then fname will get set to 'file2123.cpp'

YOU SHOULD NOT TRY TO CHANGE THIS FUNCTION. JUST USE IT AS IS.
*/

char *makeProgramFileName(int id) {

  char *s;
  char s1[20];	
  
  s = malloc (200*sizeof(char));
  memset(s, 0, sizeof(s));
  memset(s1, 0, sizeof(s1));

  sprintf(s1, "%d", id);	  
  strcpy (s, "file");
  strcat (s, s1);
  strcat (s, ".cpp");
  return s;
}  
  
/* Utility function that given a number as an argument returns an executable file name of the format prog<id>

e.g. if the call is

execName = makeExecFileName (2123);

then execName will get set to 'prog2123'

YOU SHOULD NOT TRY TO CHANGE THIS FUNCTION. JUST USE IT AS IS.
*/
char *makeExecFileName(int id) {

  char *s;
  char s1[20];	
  
  s = malloc (200*sizeof(char));
  memset(s, 0, sizeof(s));
  memset(s1, 0, sizeof(s1));
  sprintf(s1, "%d", id);	  
  strcpy (s, "prog");
  strcat (s, s1);
  return s;
} 

/* The following should be your worker thread. Fill in the blank (marked ____) and also the whole function code to complete it. */

void *grader() { // ***FIB

/* ***FIB, all of your main grading code will go here. 
 Just copy-paste the relevant part from gradingSTserver.c and make relevant changes. Especially add and modify  parts related to creating unique file names. All the utility functions are given to you above. Study the usage given above of the functions (read comments above the functions) BEFORE WRITING ANY CODE HERE. The relevant functions are:
 
 makeProgramFileName, makeExecFileName, compile_command, run_command. 
 
 The availability of these functions should make your coding super-trivial.
 
See codefragments.txt for a hint of what you can use to get a unique number. 

Finally, all the strings returned by the utility functions have been allocated on the heap. Take care to 'free' them before the thread function exits.
*/ 
    while (true) {
        pthread_mutex_lock(&queueMutex);
        while (isEmpty()) {
            pthread_cond_wait(&queueReady, &queueMutex);
        }
       
	int sockfd=deQueue();
	pthread_mutex_unlock(&queueMutex);
	
	int n;
	int id=(int)pthread_self()+rand();
	char *file_name=makeProgramFileName(id);
	char *exec_file=makeExecFileName(id);
	char *compile_string=compile_command(id,file_name,exec_file);
	char *run_string=run_command(id,exec_file);
	if (recv_file(sockfd, file_name) != 0)
    	{
         close(sockfd);
    	}
    	if (system(compile_string) !=0 ) {
	    	n = send(sockfd, "COMPILER ERROR\n", 16, 0);
	    	if (n < 0)
	    		error("ERROR writing to socket");	
	    	close(sockfd);	
	}
	else if (system(run_string) !=0 ) { 			
 				
	    	n = send(sockfd, "RUNTIME ERROR\n", 15, 0);
	    	if (n < 0)
		      error("ERROR writing to socket");	
 		close(sockfd);	
 	}  //here if no runtime error
  //Write a message "PROGRAM RAN" to client 
  //NO NEED TO SEND PROGRAM OUTPUT BACK TO CLIENT. 
 	else { 		 
 		n = send(sockfd, "PROGRAM RAN\n", 13, 0);
	    	if (n < 0)
		      error("ERROR writing to socket");	
 		close(sockfd);	
	}
    
    
    	close(sockfd);
    	if (n < 0)
      		error("ERROR writing to socket");
        free(file_name);
        free(exec_file);
        free(compile_string);
        free(run_string);
    }
    return (void *)NULL;
}

int main(int argc, char *argv[]) {


/*You should not need to change any code FROM HERE (till the line marked TILL HERE)*/

  int sockfd, //the listen socket descriptor (half-socket)
   newsockfd, //the full socket after the client connection is made
   portno; //port number at which server listens

  socklen_t clilen; //a type of an integer for holding length of the socket address
  char buffer[256]; //buffer for reading and writing the messages
  struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
  int n;
  int threadPoolSize=10;
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
  gradingqueue = (int*)malloc(maxQueueSize * sizeof(int));
  pthread_t p[threadPoolSize];
  for (int i = 0; i < threadPoolSize; i++) {
        if (pthread_create(&p[i], NULL, grader, NULL) != 0) {
            error("Failed to create a worker thread.");
            return 1;
        }
    }


  /* accept a new request, now the socket is complete.
  Create a newsockfd for this socket.
  First argument is the 'listen' socket, second is the argument 
  in which the client address will be held, third is length of second
  */
  int reqID = 0;
  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
      
 // NO CHANGE REQUIRED IN MAIN FROM THE LINE MARKED 'FROM HERE', TILL HERE    

	//***FIB add code (maybe multiple lines) HERE to create a thread and give this grading request to it
	
  
        pthread_mutex_lock(&queueMutex);
        if (!isFull()){
          enQueue(newsockfd); 
          pthread_cond_signal(&queueReady);
        }
        else {
          error("Queue full packet dropped.");
          close(newsockfd);	
        }
	pthread_mutex_unlock(&queueMutex);
  }
    
  return 0;
}

