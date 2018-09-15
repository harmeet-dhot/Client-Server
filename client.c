#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#define MAX_LENGTH 1000000
// maximum length of each command and each argument
#define MAX_WORD_LENGTH 40
int mySplit(char** res, char* s, char* delim);
//split the arguments provided by keyboard

int main(int argc, char* argv[])
{
	char message[MAX_LENGTH];
	int server, portNumber, pid, n, status;
	socklen_t len;
	struct sockaddr_in servAdd;
	char buffer[256];
  int fd,outputfd;   //file descriptor
  long int fileSize;
	char **res;
	char * sendmessage;
	FILE *fp;
	// ue the ip address of the server with the port number you wish to connect
	if (argc != 3)
	{
		fprintf(stderr, "Call model:%s <IP> <Port>\n", argv[0]);
		exit(0);
	}

	if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Cannot create a socket\n");
		exit(1);
	}

	//configure the server socket
	servAdd.sin_family = AF_INET;
	sscanf(argv[2], "%d", &portNumber);
	servAdd.sin_port = htons((uint16_t)portNumber);
	if (inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0)
	{
		fprintf(stderr, " inet_pton() has failed\n");
		exit(2);
	}
	//connect to the server
	if (connect(server, (struct sockaddr*) &servAdd, sizeof(servAdd)) < 0)
	{
		fprintf(stderr, "Connection refused\n");
		exit(3);
	}

	while (1)
	{
		pid = fork();
		if (!pid)
		{
			//prompt the user to enter commands
			write(STDOUT_FILENO, ">>", 2);
			n = read(STDIN_FILENO, message, MAX_LENGTH);
			message[n] = '\0';
			//send commands to the server using space" " to seperate arguments
			write(server, message, strlen(message) + 1);
			//user types 'quit' to close the connection
			if (!strcasecmp(message, "quit\n"))
			{
				close(server);
				kill(getppid(), SIGTERM);
				exit(0);
			 }
			 // user types get to get a file from server
			else if (strstr(message, "get ") != NULL)
			{
				n=read(server,message, MAX_LENGTH);
			  if(strcmp(message,"$")==0)
			   {
			     fprintf(stderr,"File not found\n");
					 exit(0);
			   }
				 else
				 {
				   outputfd=open("ClientCopy.txt", O_CREAT| O_TRUNC|O_RDWR,0777);
				   if(outputfd==-1)
				   {
						perror("Unable to open file for copying");
						exit(0);
				   }

			  	  message[n] = '\0';
	           //write(STDOUT_FILENO, message, n + 1);
					     write(outputfd,message, n+1);  //get the output stream for the file
					     fprintf(stderr,"File downloaded successfully\n");
							 exit(0);
			    }
			 }
		  else if (strstr(message, "put ") != NULL)
		  {
				res = (char**)malloc(sizeof(char*) * MAX_WORD_LENGTH);
				int num = mySplit(res, message, " ");
				//fprintf(stderr,"%s\n",res[1]);
				if((fp=fopen(res[1],"r"))==NULL)
				{
					perror("File not found");
					sendmessage="$";
					write(server,sendmessage,strlen(sendmessage)+1);
					exit(0);
				}
				else
				{
			   	fd=open(res[1],O_RDWR,0777);
				  if(fd==-1)
				  {
						perror("Unable to open file");
						exit(0);
					}
				 fileSize =lseek(fd, 0L, SEEK_END);
				 lseek(fd, 0L, SEEK_SET);
				 while(read(fd,&buffer,fileSize))
				 {
						 write(server,&buffer,fileSize);
				}
				 fprintf(stderr,"File uploaded successfully\n");
			 }
	    }
	}
	else
	{
			wait(&status);
	}
	}
}
int mySplit(char** res, char* s, char* delim)
{

	s[strlen(s) - 1] = 0;
	char * token;
	token = strtok(s, delim);
	int i = 0;
  while (token != NULL)
	{
		res[i] = (char*)malloc(sizeof(char) * strlen(token));
		strcpy(res[i++], token);
		token = strtok(NULL, delim);
	}
	return i;
}
