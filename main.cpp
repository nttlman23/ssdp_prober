#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

using namespace std;

#define BUFF_SIZE   20000

int main()
{
    struct in_addr localInterface;
    struct sockaddr_in groupSock;
    struct sockaddr_in localSock;
    struct ip_mreq group;

    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    int reuse = 1;

    setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    memset((char *) &groupSock, 0, sizeof(groupSock));

    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr("239.255.255.250");
    groupSock.sin_port = htons(1900);

    char loopch = 0;

    setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch));

    localInterface.s_addr = inet_addr("0.0.0.0");

    setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface));

    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(1900);
    localSock.sin_addr.s_addr = INADDR_ANY;

    bind(udpSocket, (struct sockaddr*)&localSock, sizeof(localSock));

    group.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    group.imr_interface.s_addr = inet_addr("0.0.0.0");

    setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));

    struct sockaddr_in si_other;
    socklen_t slen = sizeof(si_other);
    char buffer[BUFF_SIZE];

    for (int i = 0;i < 10;i++)
    {
        memset(buffer, 0, sizeof(buffer));

        recvfrom(udpSocket, buffer, BUFF_SIZE, 0, (struct sockaddr *) &si_other, &slen);

        char *ptr1 = nullptr;
        char *ptr2 = nullptr;

        ptr1 = strstr(buffer, "Location:");
        std::cout << "Recv: " << buffer << std::endl;

        if (ptr1)
        {
            std::cout << "Recv: " << buffer << std::endl;
            std::cout << "New device" << std::endl;

            ptr2 = strstr(ptr1,":");

            if (ptr2)
            {
                ptr2++; // pointer to http:

                ptr1 = strstr(ptr2,"\r");

                if (ptr1)
                {
                    std::string res;

                    res = std::string(ptr2, ptr1 - ptr2);
                    std::cout << "http res: " << res << std::endl;
                }
            }

            break;
        }
    }

    close(udpSocket);

    return 0;
}
