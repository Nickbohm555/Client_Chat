#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

// This part is completed: pack and send message to server

int threadCondition = 1;

int packNSendCNCT(int sockfd, char** argv) {
 uint16_t length = strlen(argv[3]);
 uint8_t high = (length >> 8)& 0xff;
 uint8_t low = length & 0xff;

 char buf[6 + length];
 buf[0] = 'C';
 buf[1] = 'N';
 buf[2] = 'C';
 buf[3] = 'T';
 buf[4] = high;
buf[5] = low;
int curr = 6;
for(int i=0; i<length; i++) {
buf[curr] = argv[3][i];
curr++;
}
int s = send(sockfd,buf,6+length, 0);
return 1;
}

// This part is done: recieved ACKC message

int PackNRec(int sockfd,char** argv) {
        char buf[4];
        char cmp[5];
        strcpy(cmp, "ACKC");
        buf[4] = '\0';
        int bytesR = recv(sockfd, buf, 4, 0);
        
        if(bytesR==4 && strncmp(buf, cmp, 4)==0){
                printf("Connected to the server as %s\n", argv[3]);
                return 1;
        }

return 0;
}


// ---------------
int quit(int sockfd, char username[]){
char buf[4];
 buf[0] = 'Q';
 buf[1] = 'U';
 buf[2] = 'I';
 buf[3] = 'T';
 
int s = send(sockfd,buf, 4, 0);
return 1;

}
// ----------



// this part is done: pack clientInfo to pass for thread

struct clientInfo {
        int sockfd;
        char username[256];
};

// void pointer for creating my SEND thread
void* SThread( void* clnt  ){


        struct clientInfo *client;

        client = (struct clientInfo*)clnt;
        int sockfd = client->sockfd;
        
        int UserLength = strlen(client->username);
        
        while(1) {

uint16_t length = UserLength;
 uint8_t high = (length >> 8)& 0xff;
 uint8_t low = length & 0xff;

 char buf[256];
 buf[0] = 'M';
 buf[1] = 'E';
 buf[2] = 'S';
 buf[3] = 'G';
 buf[4] = high;
buf[5] = low;
int curr = 6;
for(int i=0; i<UserLength; i++) {
buf[curr] = client->username[i];
curr++;
}

char msg[256];
printf("\n > ");
fgets(msg, 256, stdin);


// ------------
if( strncmp(msg, "\\quit", 5)==0 ) {
quit(sockfd, client->username);
threadCondition = 0;
return (void*)1;
}
// --------------


length = strlen(msg);
high = (length >> 8)& 0xff;
low = length & 0xff;
buf[curr] = high;
buf[curr+1] = low;
curr = curr+2;
for(int i=0; i< strlen(msg); i++) {
buf[curr] = msg[i];
curr++;
}
int bytesSent = send(sockfd,buf,8+UserLength+strlen(msg), 0);

if(bytesSent != 8+UserLength+strlen(msg)) {
  printf("Could not send message correctly \n");
}
}
}

void* RThread( void* clnt  ){

      struct clientInfo *client;
      client = (struct clientInfo*)clnt;
      int sockfd = client->sockfd;
while(threadCondition){
       char msg[256];
      //char cmp[4];    
      
      int bytesR = recv(sockfd, msg, 256, 0);
      int lengthOfUsername = msg[5];
      char username[lengthOfUsername];
      
      if(threadCondition) {
      for(int i=0; i<lengthOfUsername; i++){
        username[i] = msg[6+i];
	printf("%c", username[i]);
      } 
      printf(" >> ");
      int lengthMessage = msg[5+lengthOfUsername+2];
      char Message[lengthMessage-1];
      
      for(int i=0; i<lengthMessage; i++){
        Message[i] = msg[8+lengthOfUsername+i];
	printf("%c", Message[i]);
      }
	
      }
}


return 0;





}



int msg(int sockfd, char **argv, int result){
        if(result!=1){
                printf("ACKC did not work right");
        }
        pthread_t sender;
        pthread_t reciever;
        struct clientInfo client;
        client.sockfd = sockfd;
        strcpy(client.username, argv[3]);


        int err;
        err = pthread_create(&sender, NULL, &SThread, (void*)&client);
        err = pthread_create(&reciever, NULL, &RThread, (void*)&client);
        err = pthread_join(sender, NULL);
        err = pthread_join(reciever, NULL);
	

        return 1;

}
int main(int argc, char** argv)
{
// first create socket
int sockfd; // socket to be connected to the server

struct sockaddr_in servaddr; // this is the server it is connected to

// Ask the OS for a socket


sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("Can't get a socket from OS");
        return 1;
    }

    // pack the structure to define server location
        bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

   int port = atoi(argv[2]);
    servaddr.sin_port = htons(port);


    // Ask the OS to make the wire to the other machine
    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0 ) {
        printf("Can't make the cable to server...");
        return 1;
    }

    packNSendCNCT(sockfd, argv);
    int result = PackNRec(sockfd, argv);
    msg(sockfd, argv, result);
    close(sockfd);

    return 0; }

