//#include <windows.h>
#include <stdio.h>
//#include <conio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
  
#define SERVERPORT 1900
// #define SERVERPORT 3589
  
  
char buff[]= "M-SEARCH * HTTP/1.1\r\n"\
"HOST: 239.255.255.250:1900\r\n"\
"MAN: \"ssdp:discover\"\r\n"\
"MX: 3\r\n"\
"ST: udap:rootservice\r\n"\
"USER-AGENT: RTLINUX/5.0 UDAP/2.0 printer/4\r\n\r\n";
  
int main()
{    
   //char buff[400]; 
   char rcvdbuff[1000];
   int Ret = 2;
   socklen_t len;
    
//    WSADATA wsaData;
   struct sockaddr_in their_addr;
   struct hostent *he;    
   int sock;
//    SOCKET sock;
//    WSAStartup(MAKEWORD(2,2), &wsaData);
    
   sock = socket(AF_INET,SOCK_DGRAM,0);
    
   their_addr.sin_family      = AF_INET;  
     
   their_addr.sin_addr.s_addr = inet_addr("239.255.255.250");   
//    their_addr.sin_addr.s_addr = inet_addr("10.0.1.146"); 
   their_addr.sin_port        = htons(SERVERPORT);    
   len = sizeof(struct sockaddr_in);
  
   while(1)
   {   
      printf("buff:\n%s\n",buff);
       
      Ret = sendto(sock,buff,strlen(buff),0,(struct sockaddr*)&their_addr,len);
      if( Ret < 0)
      {
         printf("error in SENDTO() function");
         close(sock);
        //  closesocket(sock);
         return 0;
      }
       
      //Receiving Text from server
      printf("\n\nwaiting to recv:\n");
      memset(rcvdbuff,0,sizeof(rcvdbuff));
      Ret = recvfrom(sock,rcvdbuff,sizeof(rcvdbuff),0,(struct sockaddr *)&their_addr,&len);
      if(Ret < 0)
      {
         printf("Error in Receiving");
         return 0;
      }
      rcvdbuff[Ret-1] = '\0';
      printf("RECEIVED MESSAGE FROM SERVER\t: %s\n",rcvdbuff);
       
      //Delay for testing purpose
    //   Sleep(3*1000);
        usleep(1000);
    }          
    close(sock);
    // closesocket(sock);
    // WSACleanup();
}