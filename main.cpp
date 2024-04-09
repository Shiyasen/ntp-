#include <iostream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <QString>
#include <sys/time.h>
#include <chrono>
#include <thread>
#define NTP_PACKET_SIZE 48
#define NTP_TIMESTAMP_OFFSET 2208988800UL
int sockfd;
struct sockaddr_in serverAddr;
unsigned char ntpPacket[NTP_PACKET_SIZE] = {0};
void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
void signalHandler(int signum)
{

    std::cout << "Ctrl+C pressed. Closing socket..." << std::endl;
    close(sockfd);
    exit(signum);
}
void clearpacket()
{
    // Set the first byte (Leap Indicator, Version Number, Mode)
    ntpPacket[0] = 0b11100011;

    // Set the second byte (Stratum, Poll Interval, Precision)
    ntpPacket[1] = 0;

    // Set the 8-byte padding of zeroes
    for (int i = 2; i < 12; ++i)
    {
        ntpPacket[i] = 0;
    }

    // Set the 4-byte NTP timestamp (transmit time) to 0
    for (int i = 12; i < NTP_PACKET_SIZE; ++i)
    {
        ntpPacket[i] = 0;
    }
}
void syncWithNTP()
{

   // std::cout << "111Syncing with NTP..." << std::endl;
    for(int attempt=1;attempt<=5;attempt++)
    {
         std::cout << "Attempt " << attempt << " to receive data." << std::endl;
         clearpacket();
         sendto(sockfd, ntpPacket, NTP_PACKET_SIZE, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
         struct timeval timeout;
         timeout.tv_sec = 5;
         timeout.tv_usec = 0;
         setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
         if (recvfrom(sockfd, ntpPacket, NTP_PACKET_SIZE, 0, nullptr, nullptr) > 0)
         {
//             printf("Received data: ");
//             for (ssize_t i = 0; i < 48; ++i)
//             {
//                  printf("%02X ", (unsigned char)ntpPacket[i]);
//             }
//             printf("\n");

             unsigned long long transmitTime =
                 (static_cast<unsigned long long>(ntpPacket[40]) << 24) |
                 (static_cast<unsigned long long>(ntpPacket[41]) << 16) |
                 (static_cast<unsigned long long>(ntpPacket[42]) << 8) |
                 (static_cast<unsigned long long>(ntpPacket[43]));

             // Convert NTP timestamp to Unix timestamp
             time_t unixTime = static_cast<time_t>(transmitTime - NTP_TIMESTAMP_OFFSET);
             //The time received from NTP is in UTC,
             //and to convert it to Beijing time, 8 hours should be added
             unixTime+=8*3600;
             struct tm *timeInfo = std::gmtime(&unixTime);
             char buffer[80];
             strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
             //settimeofday,This function configures UTC time,
             //After subtracting another eight hours, configure again
             //and the system internally automatically converts it to the local time
             struct timeval tv;
             tv.tv_sec = unixTime-8*3600;
             tv.tv_usec = 0;
             if (settimeofday(&tv, nullptr) == 0)
             {

                 std::cout << "System time set successfully." << std::endl;
             }
             else
             {
                 std::cerr << "Failed to set system time." << std::endl;
             }
             // Print the result

             std::cout << "Converted Time: " << buffer << std::endl;
             break;

         }
         else
         {

             std::cerr << "Attempt " << attempt << " failed. Error: " << strerror(errno) << std::endl;

         }

    }
    //return 0;
}

int main(int argc, char *argv[])
{
    int Interval_time=0;
    if (argc != 4)
    {

        std::cerr << "Usage: " << argv[0] << " <NTP Server IP>" <<" <PORT>"<<" <Interval time, unit: seconds>"<< std::endl;
        return EXIT_FAILURE;
    }

    int PORT = QString(argv[2]).toInt();
    const char *ntpServerIP = argv[1];
    Interval_time=QString(argv[3]).toInt();
    std::cout << "NTP Server: " << ntpServerIP <<":"<<PORT<<"  interval_time="<<Interval_time<<"s"<< std::endl;
    clearpacket();
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        error("Error creating socket!");
    }
    signal(SIGINT,signalHandler);
    std::cout << "udp socket create successfully!" << std::endl;
    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, ntpServerIP, &serverAddr.sin_addr) <= 0)
    {
        error("Error converting IP address");
    }


    // Send NTP packet
//    if (sendto(sockfd, ntpPacket, NTP_PACKET_SIZE, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
//    {
//        error("Error sending packet");
//    }
    while(1)
    {
        syncWithNTP();
        std::this_thread::sleep_for(std::chrono::seconds(Interval_time));
    }


    close(sockfd);
    return EXIT_SUCCESS;
}



