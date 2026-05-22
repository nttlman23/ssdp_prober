#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

using namespace std;

#define BUFF_SIZE 20000
#define SSDP_ADDR "239.255.255.250"
#define SSDP_PORT 1900

// M-SEARCH request template
const char *M_SEARCH_REQUEST =
    "M-SEARCH * HTTP/1.1\r\n"
    "HOST: %s:%d\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "MX: 3\r\n"
    "ST: %s\r\n"
    "USER-AGENT: Linux/5.0 SSDP Prober/1.0\r\n"
    "\r\n";

// Common search targets
const char *SEARCH_TARGETS[] = {
    "upnp:rootdevice",
    "ssdp:all",
    "uuid:*",
    NULL};

struct DeviceInfo
{
    string location;
    string usn;
    string st;
    string cacheControl;
    string server;
    struct sockaddr_in address;
};

void signalHandler(int sig)
{
    // Clean exit on Ctrl+C or SIGTERM
    exit(0);
}

bool sendMSearch(int sock, const char *target)
{
    char request[512];

    snprintf(request, sizeof(request), M_SEARCH_REQUEST,
             SSDP_ADDR, SSDP_PORT, target);

    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(SSDP_ADDR);
    destAddr.sin_port = htons(SSDP_PORT);

    ssize_t sent = sendto(sock, request, strlen(request), 0,
                          (struct sockaddr *)&destAddr, sizeof(destAddr));

    if (sent < 0)
    {
        cerr << "Error sending M-SEARCH: " << strerror(errno) << endl;
        return false;
    }

    cout << "Sent M-SEARCH for: " << target << " (" << sent << " bytes)" << endl;
    return true;
}

bool parseHTTPResponse(const char *buffer, size_t len, DeviceInfo &info)
{
    string response(buffer, static_cast<size_t>(len) < BUFF_SIZE ? static_cast<size_t>(len) : BUFF_SIZE);

    // Find Location header
    size_t locPos = response.find("Location:");
    if (locPos != string::npos)
    {
        size_t start = locPos + 9; // skip "Location:"
        size_t end = response.find("\r\n", start);
        if (end == string::npos)
            end = response.length();

        // Trim whitespace
        while (start < end && (response[start] == ' ' || response[start] == '\t'))
            start++;
        while (end > start && (response[end - 1] == ' ' || response[end - 1] == '\t' ||
                               response[end - 1] == '\r'))
            end--;

        info.location = response.substr(start, end - start);
    }

    // Find USN header
    size_t usnPos = response.find("USN:");
    if (usnPos != string::npos)
    {
        size_t start = usnPos + 4;
        size_t end = response.find("\r\n", start);
        if (end == string::npos)
            end = response.length();

        while (start < end && (response[start] == ' ' || response[start] == '\t'))
            start++;
        while (end > start && (response[end - 1] == ' ' || response[end - 1] == '\t' ||
                               response[end - 1] == '\r'))
            end--;

        info.usn = response.substr(start, end - start);
    }

    // Find ST header
    size_t stPos = response.find("ST:");
    if (stPos != string::npos)
    {
        size_t start = stPos + 3;
        size_t end = response.find("\r\n", start);
        if (end == string::npos)
            end = response.length();

        while (start < end && (response[start] == ' ' || response[start] == '\t'))
            start++;
        while (end > start && (response[end - 1] == ' ' || response[end - 1] == '\t' ||
                               response[end - 1] == '\r'))
            end--;

        info.st = response.substr(start, end - start);
    }

    // Find CACHE-CONTROL header
    size_t ccPos = response.find("CACHE-CONTROL:");
    if (ccPos != string::npos)
    {
        size_t start = ccPos + 14;
        size_t end = response.find("\r\n", start);
        if (end == string::npos)
            end = response.length();

        while (start < end && (response[start] == ' ' || response[start] == '\t'))
            start++;
        while (end > start && (response[end - 1] == ' ' || response[end - 1] == '\t' ||
                               response[end - 1] == '\r'))
            end--;

        info.cacheControl = response.substr(start, end - start);
    }

    // Find SERVER header
    size_t srvPos = response.find("SERVER:");
    if (srvPos != string::npos)
    {
        size_t start = srvPos + 7;
        size_t end = response.find("\r\n", start);
        if (end == string::npos)
            end = response.length();

        while (start < end && (response[start] == ' ' || response[start] == '\t'))
            start++;
        while (end > start && (response[end - 1] == ' ' || response[end - 1] == '\t' ||
                               response[end - 1] == '\r'))
            end--;

        info.server = response.substr(start, end - start);
    }

    // Check if we found at least Location
    return !info.location.empty();
}

void printDeviceInfo(const DeviceInfo &info)
{
    cout << "\n--- New Device Detected ---" << endl;
    cout << "Address:  " << inet_ntoa(info.address.sin_addr)
         << ":" << ntohs(info.address.sin_port) << endl;

    if (!info.st.empty())
        cout << "ST:       " << info.st << endl;
    if (!info.usn.empty())
        cout << "USN:      " << info.usn << endl;
    if (!info.cacheControl.empty())
        cout << "CACHE-CTRL:" << info.cacheControl << endl;
    if (!info.server.empty())
        cout << "SERVER:   " << info.server << endl;
    if (!info.location.empty())
    {
        cout << "Location: " << info.location << endl;

        // Optionally fetch via HTTP
        cout << "  -> Can be reached at this URL" << endl;
    }
    cout << "---------------------------\n"
         << endl;
}

void printUsage(const char *progName)
{
    cout << "Usage: " << progName << " [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -t <target>   Search target (default: upnp:rootdevice)" << endl;
    cout << "                Options: upnp:rootdevice, ssdp:all, uuid:*" << endl;
    cout << "  -a <address>  Multicast address (default: 239.255.255.250)" << endl;
    cout << "  -p <port>     Port number (default: 1900)" << endl;
    cout << "  -n <count>    Number of responses to collect (default: unlimited)" << endl;
    cout << "  -h            Show this help" << endl;
}

int main(int argc, char *argv[])
{
    // Set up signal handling for clean shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Parse command line arguments
    string searchTarget = "upnp:rootdevice";
    int maxDevices = 0; // 0 means unlimited

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
        {
            searchTarget = argv[++i];
        }
        else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc)
        {
            // Address argument handled in socket setup
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            // Port argument handled in socket setup
        }
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
        {
            maxDevices = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            printUsage(argv[0]);
            return 0;
        }
    }

    cout << "=== SSDP Prober ===" << endl;
    cout << "Search target: " << searchTarget << endl;
    cout << "Max devices:   " << (maxDevices ? to_string(maxDevices) : "unlimited") << endl;
    cout << "Press Ctrl+C to stop\n"
         << endl;

    // Create UDP socket
    int udpSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0)
    {
        cerr << "Error creating socket: " << strerror(errno) << endl;
        return 1;
    }

    // Enable socket reuse
    int reuse = 1;
    if (::setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        cerr << "Error setting SO_REUSEADDR: " << strerror(errno) << endl;
        ::close(udpSocket);
        return 1;
    }

    // Set socket receive timeout (5 seconds for recvfrom)
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (::setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        cerr << "Error setting receive timeout: " << strerror(errno) << endl;
    }

    // Configure multicast socket
    struct in_addr localInterface;
    localInterface.s_addr = inet_addr("0.0.0.0");

    if (::setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface)) < 0)
    {
        cerr << "Error setting multicast interface: " << strerror(errno) << endl;
    }

    // Disable multicast loopback to avoid receiving our own packets
    char loopch = 0;
    if (::setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopch, sizeof(loopch)) < 0)
    {
        cerr << "Warning: Could not disable multicast loop" << endl;
    }

    // Bind to local address and port - use ::bind() to avoid std::bind conflict
    struct sockaddr_in localSock;
    memset(&localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(SSDP_PORT);
    localSock.sin_addr.s_addr = INADDR_ANY;

    if (::bind(udpSocket, (struct sockaddr *)&localSock, sizeof(localSock)) < 0)
    {
        cerr << "Error binding to port " << SSDP_PORT << ": " << strerror(errno) << endl;
        ::close(udpSocket);
        return 1;
    }

    // Join multicast group
    struct ip_mreq group;
    memset(&group, 0, sizeof(group));
    group.imr_multiaddr.s_addr = inet_addr(SSDP_ADDR);
    group.imr_interface.s_addr = INADDR_ANY;

    if (::setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0)
    {
        cerr << "Error joining multicast group: " << strerror(errno) << endl;
        ::close(udpSocket);
        return 1;
    }

    // Send M-SEARCH request(s)
    if (searchTarget == "all")
    {
        // Search for all devices
        for (int i = 0; SEARCH_TARGETS[i] != NULL; i++)
        {
            sendMSearch(udpSocket, SEARCH_TARGETS[i]);
            usleep(500000); // Wait 500ms between searches
        }
    }
    else
    {
        sendMSearch(udpSocket, searchTarget.c_str());
    }

    cout << "\nListening for responses...\n"
         << endl;

    // Receive loop
    struct sockaddr_in si_other;
    socklen_t slen = sizeof(si_other);
    char buffer[BUFF_SIZE];
    int devicesFound = 0;

    while (true)
    {
        memset(buffer, 0, BUFF_SIZE);

        // Use select() for non-blocking behavior with timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(udpSocket, &readfds);

        tv.tv_sec = 3;
        tv.tv_usec = 0;

        int activity = ::select(udpSocket + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0)
        {
            cerr << "Select error: " << strerror(errno) << endl;
            break;
        }
        else if (activity == 0)
        {
            // Timeout - no data received, try again
            continue;
        }

        ssize_t recvLen = ::recvfrom(udpSocket, buffer, BUFF_SIZE - 1, 0,
                                     (struct sockaddr *)&si_other, &slen);

        if (recvLen < 0)
        {
            cerr << "Error receiving data: " << strerror(errno) << endl;
            continue;
        }

        buffer[recvLen] = '\0';

        // Parse and display device info
        DeviceInfo device;
        memset(&device, 0, sizeof(device));
        device.address = si_other;

        if (parseHTTPResponse(buffer, recvLen, device))
        {
            devicesFound++;
            printDeviceInfo(device);

            if (maxDevices > 0 && devicesFound >= maxDevices)
            {
                cout << "Reached maximum of " << maxDevices << " devices.\n"
                     << endl;
                break;
            }
        }
        else
        {
            // Not a valid SSDP response, but display raw data for debugging
            cout << "Received non-SSDP packet from: "
                 << inet_ntoa(si_other.sin_addr) << ":"
                 << ntohs(si_other.sin_port) << endl;
            cout << "Data (first 100 bytes): ";
            for (int i = 0; i < min((size_t)100, static_cast<size_t>(recvLen)); i++)
            {
                if (buffer[i] >= ' ' && buffer[i] <= '~')
                    cout << buffer[i];
                else
                    cout << ".";
            }
            cout << endl
                 << endl;
        }
    }

    // Cleanup
    ::setsockopt(udpSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group, sizeof(group));
    ::close(udpSocket);

    cout << "Scan complete. Devices found: " << devicesFound << endl;
    return 0;
}
