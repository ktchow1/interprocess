#pragma once
#include<socket.h>


void test_udp_multicast_server(const std::string& multicast_group, std::uint32_t port)
{
    int sockfd;
    struct sockaddr_in multicast_addr;
    std::string message = "Hello, multicast clients!";

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw std::runtime_error("[UDP multicast server] Cannot create socket");
    }
/*
    // Set multicast TTL (time-to-live) THIS IS NOT NECESSARY, IT CONTROLS HOW MANY ROUTER-HOPS for multicast
    int ttl = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) 
    {
        throw std::runtime_error("[UDP multicast server] Cannot set socket option");
        close(sockfd);
    }
*/
    // Configure multicast address
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(multicast_group.c_str());
    multicast_addr.sin_port = htons(port);

    while (true) 
    {
        // Send message to multicast group
        if (sendto(sockfd, message.c_str(), 
                           message.length(), 0,
                          (const sockaddr*) &multicast_addr, 
                                      sizeof(multicast_addr)) < 0) 
        {
            throw std::runtime_error("[UDP multicast server] Cannot send");
            close(sockfd);
        } 
        else 
        {
            std::cout << "Sent: " << message << std::endl;
        }
        sleep(2); // Send every 2 seconds
    }

    close(sockfd);
}
