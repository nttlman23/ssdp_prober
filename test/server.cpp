//#include <windows.h>
#include <stdio.h>
//#include <conio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
  
#define MYPORT 1900   
//#define MYPORT 3589
  
char buff[]="HTTP/1.1 200 OK\r\n"\
"CACHE-CONTROL: max-age=172800\r\n"\
"DATE: Wed Jul 11 05:55:53 2012 GMT\r\n"\
"EXT: \r\n"\
"LOCATION: http://192.168.1.110:8080/udap/api/data?target=smartText.xml\r\n"\
"SERVER: Linux/2.6.18-308.4.1.el5 UDAP/2.0 HP 55UB950V/2.0\r\n"\
"ST: urn:schemas-udap:service:smartText:1\r\n"\
"USN: uuid:33068e81-3306-0633-619b-9b61818e0633::urn:schemas-udap:service:smartText:1\r\n\r\n";
  
  
int main()
{
  int Ret = 2;
  socklen_t len = 0;
  socklen_t lenServ = 0;
  char rcvdbuff[400];
     
  //WSADATA wsaData;
  //SOCKET sock;
  int sock;
  struct sockaddr_in my_addr;            // my address information
  struct sockaddr_in their_addr;         // connector's address information  
  //WSAStartup(MAKEWORD(2,2), &wsaData);
   
  sock = socket(AF_INET,SOCK_DGRAM,0);
     
  my_addr.sin_family = AF_INET;         // host byte order
  my_addr.sin_port = htons(MYPORT);     // short, network byte order
//   my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  my_addr.sin_addr.s_addr = inet_addr("239.255.255.250");
  len = sizeof(my_addr);
  their_addr.sin_family      = AF_INET;
  their_addr.sin_addr.s_addr = inet_addr("239.255.255.250");
  their_addr.sin_port        = htons(MYPORT);
  lenServ = sizeof(struct sockaddr_in);

  while(1)
  {         
      if (bind(sock,(struct sockaddr*)&my_addr, sizeof(my_addr)) < 0)
      {
         printf("Error in BINDING");
         return 0;
      }
       
      printf("BINDING successfull ...\n");
      while(1)
      {
         fprintf(stderr, "Waiting to receive ...\n");
               
         //Receiving Text from Client     
        //  Ret = recvfrom(sock,rcvdbuff,sizeof(rcvdbuff),0,(struct sockaddr *)&their_addr,&len);         
        //  if(Ret < 0)
        //  {
            // printf("Error in Receiving");
            // return 0;
        //  }
        //  rcvdbuff[Ret] = '\0'; 
        //  printf("\nReq:-\n%s",rcvdbuff);
          
        //  printf("\n\nReceived packet of length %i from %s:%d\n\n",
          // Ret, inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
                         
         Ret = sendto(sock,buff,strlen(buff),0,(struct sockaddr*)&their_addr,lenServ);
         if( Ret < 0)
         {
           printf("error in SENDTO() function %s\n", strerror(errno));
           //closesocket(sock);
           close(sock);
           return 0;
         }

         usleep(2 * 1000);
      }
      //closesocket(sock);
      close(sock);
  }  
  //WSACleanup();
}
