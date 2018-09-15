#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LENGTH 1000000
#define MAX_WORD_LENGTH 40

void serverClient(int s);
int mySplit(char** res, char* s, char* delim);

int main(int argc, char* argv[])
 {
	int sd, client, portNumber, status;
	socklen_t len;
	struct sockaddr_in servAdd;

	if (argc != 2) {
		printf("Call model: %s <Port:#>\n", argv[0]);
		exit(0);
	}

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Wrong creating new socket");
		exit(1);
	}

	//config the server socket
	servAdd.sin_family = AF_INET;
	servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	sscanf(argv[1], "%d", &portNumber);
	servAdd.sin_port = htons((uint16_t)portNumber);
	bind(sd, (struct sockaddr*) &servAdd, sizeof(servAdd));

	// start listening to the assigned port for at most 5 clients
	listen(sd, 5);
	while (1) {
		// wait for all clients from any IP address
		client = accept(sd, (struct sockaddr*) NULL, NULL);
		printf("Got a client\n");
		if (!fork())
			serverClient(client);
		close(client);
		waitpid(0, &status, WNOHANG);
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
void serverClient(int sd)
{
	char message[MAX_LENGTH];
	int n, pid, status;
	char **res;
	char buffer[256];
  int fd,outputfd;   //file descriptor
  long int fileSize;
	FILE  *fp;
	char *sendmessage;
//	dup2(sd, STDOUT_FILENO);
while (1)
{
		pid = fork();
		if (!pid)
		{
			//read from client
			if (n = read(sd, message, MAX_LENGTH))
			{
				// quit if the client sends 'quit'
				message[n] = '\0';
				if (!strcasecmp(message, "quit\n"))
				{
					fprintf(stderr, "%s\n", "\n Client disconnected, waiting for a new client...");
					close(sd);
					kill(getppid(), SIGTERM);
					exit(0);
				}
				else if(strstr(message, "get ") != NULL)
				{
				  res = (char**)malloc(sizeof(char*) * MAX_WORD_LENGTH);
				  int num = mySplit(res, message, " ");
			    if((fp =fopen(res[1],"r"))==NULL)
				  {
			     sendmessage="$";
				   write(sd,sendmessage,strlen(sendmessage)+1);
				  }
					else
					{
					 fd=open(res[1],O_RDWR,0777);
				   if(fd==-1)
					  {
              perror("File not opening");
							exit(0);
						}
				   fileSize =lseek(fd, 0L, SEEK_END);
				 	 lseek(fd, 0L, SEEK_SET);
					 while(read(fd,&buffer,fileSize))
					 {
               write(sd,&buffer,fileSize);
           }
				  }

				}
			else if(strstr(message, "put ") != NULL)
				{
            n = read(sd, message, MAX_LENGTH);
						if(strcmp(message,"$")==0)
					   {
					     fprintf(stderr,"File not found\n");
							 exit(0);
					   }
						else
						{
					    outputfd=open("ServerCopy.txt", O_CREAT|O_TRUNC|O_RDWR,0777);
				     	if(outputfd==-1)
					    {
 		          perror("Unable to open file for copying");
 		          exit(0);
				      }
 			  	     message[n] = '\0';
 			  	    // write(STDOUT_FILENO, message, n + 1);
					    write(outputfd,message, n+1);
 			      }

				}

			}
		}
		else
		{
			wait(&status);
		}
	}
}
