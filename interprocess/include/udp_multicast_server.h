#pragma once
#include<socket.h>


namespace ipc
{
    class udp_multicast_server
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        udp_multicast_server(const std::string&  multicast_group_ip, 
                                   std::uint16_t multicast_group_port)
                                   : m_fd(::socket(AF_INET, SOCK_DGRAM, 0)) 
                                   , m_dbg("[UDP mulitcast server]")
        {
            m_dbg.log();
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail to create");
            }


            // ************************************** //
            // *** Step 2 : config addr (no bind) *** //
            // ************************************** //
            m_group_addr.sin_family      = AF_INET;
            m_group_addr.sin_addr.s_addr = inet_addr(multicast_group_ip.c_str());
            m_group_addr.sin_port        = htons(multicast_group_port);
        }

    private:
        int m_fd;
        debugger m_dbg;

    private:
        sockaddr_in m_group_addr;
    };
}














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
