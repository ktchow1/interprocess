#pragma once
#include<socket.h>



namespace ipc
{
    class udp_multicast_client
    {
    public:
        udp_multicast_client(const std::string&  multicast_group_ip, 
                                   std::uint16_t multicast_group_port)
                                   : m_fd(::socket(AF_INET, SOCK_DGRAM, 0)) 
                                   , m_dbg("[UDP mulitcast client]")
        {
            m_dbg.log();
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail to create");
            }
        }

    private:
        int m_fd;
        debugger m_dbg;
    };
}


void test_udp_multicast_client(const std::string& multicast_group, std::uint32_t port)
{
    constexpr std::uint32_t buffer_size = 1024;

    int sockfd;
    char buffer[buffer_size];
    struct sockaddr_in local_addr;
    struct ip_mreq multicast_request;
    
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        throw std::runtime_error("[UDP multicast client] Cannot create socket");
    }

    // THERE ARE 2 set option CALL :
    // 1. allow multi clients in same host to bind to same port, otherwise, result in "address already in use" ERROR
    // 2. join multicast group


    // Configure local address
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port);

    // Bind socket to multicast port
    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        close(sockfd);
        throw std::runtime_error("[UDP multicast client] Cannot bind socket");
    }

    // Join multicast group
    multicast_request.imr_multiaddr.s_addr = inet_addr(multicast_group.c_str());
    multicast_request.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) < 0)
    {
        close(sockfd);
        throw std::runtime_error("[UDP multicast client] Cannot set socket option - join group");
    }

    // Allow multiple sockets to use the same port
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
    {
        close(sockfd);
        throw std::runtime_error("[UDP multicast client] Cannot set socket option");
    }

    std::cout << "Listening for multicast messages on " << multicast_group << ":" << port << std::endl;
    while (true) 
    {
        // Receive multicast messages
        ssize_t recv_len = recvfrom(sockfd, buffer, buffer_size, 0, nullptr, nullptr);
        if (recv_len < 0) 
        {
            std::cout << "connection failed" << std::endl;
            break;
        }
        buffer[recv_len] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }

    // Leave multicast group
    setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast_request, sizeof(multicast_request));
    close(sockfd);
}



