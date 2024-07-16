#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#define SA struct sockaddr
#define PORT1 6000
#define PORT2 7000

struct FN_SD {
    int sock;
    char File[1000];
};

struct FD_SD {
    int conn;
    int FD;
};



void* PServer(){

//CREATING SERVER SOCKET

int sockfd, connfd, len;
struct sockaddr_in servaddr, cli;
   
// socket create and verification
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
}
else
  	printf("Socket successfully created..\n");


bzero(&servaddr, sizeof(servaddr));
   
// assign IP, PORT
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
servaddr.sin_port = htons(PORT1);
   
// Binding newly created socket to given IP and verification
if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
	printf("socket bind failed...\n");
        exit(0);
}
else
        printf("Socket successfully binded..\n");
   
// Now server is ready to listen and verification
if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
}
else
        printf("Server listening..\n");

len = sizeof(cli);
   
// Accept the data packet from client and verification
connfd = accept(sockfd, (SA*)&cli, &len);
if (connfd < 0) {
	//printf("server accept failed...\n");
        exit(0);
}
else
        //printf("server accept the client...\n");


ReceiveAndSend(connfd);

close(sockfd);

}

void* PClient(){

char IP[20];

printf("Enter IP : ");

if(!fgets(IP, 20, stdin))
{
	perror("Error: \n");

}

printf("IP: %s", IP);

//CREATING CLIENT SOCKET 

int sockfd, connfd;
struct sockaddr_in servaddr, cli;
   
// socket create and verification
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd == -1) {
	printf("socket creation failed...\n");
        exit(0);
}
else
        printf("Socket successfully created..\n");

bzero(&servaddr, sizeof(servaddr));
   
// assign IP, PORT
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = inet_addr(IP);
servaddr.sin_port = htons(PORT1);
   
// connect the client socket to server socket
if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
}
else
	printf("connected to the server..\n");


char Input[1000];
char* Command;

printf("Press 1 to act as Server or Press 2 to act as Client. \n");

if(!fgets(Input, 1000, stdin))
{
	perror("Error: \n");

}

Input[strcspn(Input, "\n")] = 0;

if(strcmp(Input,"1") == 0){

printf("Acting as Server \n");

}

else{

printf("Acting as Client \n");

printf("Enter Command: \n");

if(!fgets(Input, 1000, stdin))
{
	perror("Error: \n");

}

Input[strcspn(Input, "\n")] = 0;

printf("Input: %s\n", Input);

SendAndReceive(sockfd, Input);

close(sockfd);

}

//close(sockfd);

}

void *forClient(void *ptr){
	printf("Reached Client thread\n");
	struct FN_SD *my_ptr = (struct FN_SD*)ptr;
        char buff[1024];
    int open_st = open((char*)my_ptr->File, O_RDONLY,S_IRWXU);
    if(open_st == -1)
        perror("error open");
    int read_chk=1;
	while(read_chk != 0) {
        read_chk = read(open_st,buff,1024);
            write(my_ptr->sock,buff,read_chk);
	}
}


void SendAndReceive(int sockfd, char* Input){

char Recv[20000];
char InputCopy[1000];
char* Command;
char* FileName;

strcpy (InputCopy, Input);
Command = strtok(Input,"@");

write(sockfd, InputCopy, sizeof(InputCopy));

if (strcmp(Command,"ls") == 0){

read(sockfd, Recv, sizeof(Recv));

printf("From Server : \n");

printf(Recv);

}
else if(strcmp(Command,"cp") == 0){

printf("Cp Command Detected\n");

FileName = strtok(NULL,"@");
FileName = strtok(FileName,"\n");

printf("FileName: %s\n",FileName);

struct FN_SD* Info;
Info = malloc(sizeof(struct FN_SD));
Info->sock = sockfd;
strcpy (Info->File,FileName);

pthread_t t[0];
pthread_create(&t[0],NULL,&forClient,(void*)Info);
pthread_join(t[0],NULL);


}


}

void *forServer(void* ptr){
	printf("Receiving File through server thread\n");
	struct FD_SD *my_ptr = (struct FD_SD*)ptr;
	char buf[1024];
        int rval;
        int i;
	 do {
		bzero(buf, sizeof(buf));
		if ((rval = read(my_ptr->conn, buf, 1024)) < 0)
			perror("reading stream message");
		i = 0;
		if (rval == 0){
			write(STDOUT_FILENO,"Ending connection\n",sizeof("Ending connection"));
		}
		else 
               	write(my_ptr->FD,buf,rval);
		} while (rval != 0);
}


void ReceiveAndSend(int connfd){

char Input[1000];
char* LinuxCommand;
char* Directory;
char Complete[20000];

read(connfd, Input, sizeof(Input));

printf("From Client: %s\n", Input);

LinuxCommand = strtok(Input,"@");



if(strcmp(LinuxCommand,"ls") == 0){

Directory = strtok(NULL, "@");
Directory = strtok(Directory, "\n");

DIR  *dp;
struct dirent *ep;

        dp = opendir (Directory);
        if (dp != NULL)
        {
			while (ep = readdir (dp))
            {
				char type[10];
				if(ep->d_name[0] == '.'){
				}
				else{

					if(ep->d_type==DT_DIR) strcpy(type,"Dir"); else strcpy(type,"File");
					printf("%s : %s \n",type, ep->d_name);
					strcat(Complete, type);
					strcat(Complete, " : ");
					strcat(Complete, ep->d_name);
					strcat(Complete, "\n");

				}



             }
             closedir (dp);
        }
        else{
			puts ("Couldn't open the directory.");
        }

        write(connfd, Complete, sizeof(Complete));


}
else if(strcmp(LinuxCommand,"cp") == 0){

printf("CP detected\n");

pthread_t t[0];
int open_st;
open_st = open("serverFile",O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, S_IRWXU);
if(open_st == -1)
	perror("error in open");

struct FD_SD* Info;
Info = malloc(sizeof(struct FD_SD));
Info->conn = connfd;
Info->FD = open_st;

pthread_create(&t[0],NULL,&forServer,(void*)Info);
pthread_join(t[0],NULL);


}

}


int main() {

pthread_t ParentServer_thread;
pthread_t ParentClient_thread;

pthread_create(&ParentServer_thread, NULL, &PServer, NULL);
pthread_create(&ParentClient_thread, NULL, &PClient, NULL);

pthread_join(ParentServer_thread, NULL);
pthread_join(ParentClient_thread, NULL);


}
