/************* UDP CLIENT CODE *******************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>


int n;
char **storedIP;
char **storedPort;
int sendokay=0;
int okayrcvd =0;
char *myipaddress;
int myindex;
void *Server()
{
  int udpSocket, nBytes,lines=0;
  char buffer[1024];
  struct sockaddr_in serverAddr, clientAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;
  int i;

  
  //get IP address of machine
  int n1;
  struct ifreq ifr;
  char array[] = "eth0";
  n1 = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
  ioctl(n1, SIOCGIFADDR, &ifr);
  close(n1);
  //display result
  myipaddress = inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
  printf(" my IP Address is %s - %s\n" , array , myipaddress );


  /*Create UDP socket*/
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = 0;
  serverAddr.sin_addr.s_addr = inet_addr(myipaddress);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverStorage;

  //get port number
  int portno;
  struct sockaddr_in sin;
  socklen_t length = sizeof(sin);
  if (getsockname(udpSocket, (struct sockaddr *)&sin, &length) == -1)
      perror("getsockname");
  else
  {
      portno = ntohs(sin.sin_port);
      printf("port number %d\n", portno);
  }

//writing my ip address and port to the end points file
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  fp  = fopen("endpoints.txt","a+");
    fprintf(fp,"%s %d\n",myipaddress,portno );
  fclose(fp);

  //allocating memory for storedip and storedport
  storedIP = malloc(n*sizeof(char*));
  storedPort = malloc(n*sizeof(char*));
  int k;
  for(k=0; k<n; k++)
  {
    storedIP[k] = malloc(50*sizeof(char));
    storedPort[k] = malloc(50*sizeof(char));
  }

  //getting myindex and coping ip and port till myindex
  fp = fopen("endpoints.txt", "r");
  while ((read = getline(&line, &len, fp)) != -1) {
    char find = ' ';
    const char *ptr = strchr(line, find);
    int index;
    if(ptr) 
    {
        index = ptr - line;
    }
    memcpy( storedPort[lines],line+index+1, read-index-2); // printf("original length %d",strlen(storedPort[lines]));    // storedPort[lines]+='\0';     // printf("original length %d",strlen(storedPort[lines]));
    memcpy( storedIP[lines],line, index );    // printf("ip address:%s and length is %d\n",storedIP[lines],strlen(storedIP[lines]));    // printf("port is:%s and length is %d\n",storedPort[lines],strlen(storedPort[lines]) );
    lines++;
  }
  fclose(fp);


  myindex = lines;
  if(myindex==n)
    sendokay = 1;

  while(1){
    printf("server waiting for input\n");
    nBytes = recvfrom(udpSocket,buffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);
    printf("input received\n");
    if(buffer[0] == 'Q')
    {
      printf("closing server\n");
      close(udpSocket);
      break;
    }
    printf("%s\n",buffer);
    if(strcmp(buffer,"okay")==0)
    {
      okayrcvd = 1;
      printf("okay Received\n");
    }
    // else if()
    /*Send uppercase message back to client, using serverStorage as the address*/
    // sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
  }

  //return 0;
  pthread_exit(NULL);
}

int main(int argc, char* argv[]){


  n= atoi(argv[1]);
  printf("argument : %d\n",n );
  pthread_t thread;
  int rc;
  rc = pthread_create(&thread, NULL, Server, NULL);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }

  printf("going to busy waiting\n");
  while( sendokay==0 && okayrcvd==0);
  printf("OUT of busywaiting with sendokay:%d and okayrcvd:%d\n",sendokay,okayrcvd);

  FILE *fp;
   fp = fopen("endpoints.txt", "r");
  while ((read = getline(&line, &len, fp)) != -1) {
    char find = ' ';
    const char *ptr = strchr(line, find);
    int index;
    if(ptr) 
    {
        index = ptr - line;
    }
    memcpy( storedPort[lines],line+index+1, read-index-2); // printf("original length %d",strlen(storedPort[lines]));    // storedPort[lines]+='\0';     // printf("original length %d",strlen(storedPort[lines]));
    memcpy( storedIP[lines],line, index );    // printf("ip address:%s and length is %d\n",storedIP[lines],strlen(storedIP[lines]));    // printf("port is:%s and length is %d\n",storedPort[lines],strlen(storedPort[lines]) );
    lines++;
  }
  fclose(fp);


  // printf("the ip and port in endpoints are:\n");

  int clientSocket, portNum, nBytes;
  char buffer[1024];
  struct sockaddr_in serverAddr[n];
  socklen_t addr_size[n];

  /*Create UDP socket*/
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

  int l;
  for(l=0;l<n;l++)
  {
    // if(l==myindex) continue;
    printf("%d\tIP address:%s\tPort:%s\n",l,storedIP[l],storedPort[l] );
    serverAddr[l].sin_family = AF_INET;
    serverAddr[l].sin_port = htons(atoi(storedPort[l]));
    // printf("port %d\n",atoi(storedPort[0]) );
    serverAddr[l].sin_addr.s_addr = inet_addr(storedIP[l]);
    // printf("ip \n");
    memset(serverAddr[l].sin_zero, '\0', sizeof serverAddr[l].sin_zero);  
    addr_size[l] = sizeof serverAddr[l];
  }

  int m;
  char okay[5] = {'o','k','a','y','\0'};
  printf("array okay is %s\n",okay );
  if(sendokay ==1)
    for(m=0;m<myindex-1;m++)
      // if(l==myindex) continue;
  {
      printf("sending okay msg \n");
      ssize_t rhythm =sendto(clientSocket,okay,5,0,(struct sockaddr *)&serverAddr[m],addr_size[m]);
      printf("rhythm:%zu\n",rhythm);
      printf("oaky message sent\n");
  }
  // while(1){
    // printf("Type a sentence to send to server:\n");
    // fgets(buffer,1024,stdin);
    // printf("You typed: %s",buffer);

    // printf("%c\n",buffer[0] );
    // if(buffer[0] == 'q')
  //   {
  //     printf("closing client\n");
  //     close(clientSocket);
  //     break;
  //   }
  //   else printf("fdsjnfkdj\n");
    // nBytes = strlen(buffer) + 1;
    // sendto(clientSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
  //               // nBytes = recvfrom(clientSocket,buffer,1024,0,NULL, NULL);
  //   printf("Received from server: %s\n",buffer);
  // }

  pthread_join(thread,NULL);
  return 0;
}