#pragma once
#include<socket.h>


namespace ipc
{
    class udp_unicast_server
    {
    public:
        // Step 1. Create socket
        udp_unicast_server(std::uint16_t port) : m_fd(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
        {
            if (m_fd == -1)
            {
                throw std::runtime_error("[UDP server] Cannot create socket");
            }

            sockaddr_in addr_this;
            addr_this.sin_family = AF_INET;
            addr_this.sin_port = htons(port);
            addr_this.sin_addr.s_addr = htonl(INADDR_ANY);

            // Step 2. Bind socket
            if (::bind(m_fd, (struct sockaddr*)&addr_this, sizeof(addr_this)) == -1)
            {
                throw std::runtime_error("[UDP server] Cannot bind socket");
            }
        }

       ~udp_unicast_server()
        {
            if (m_fd > 0) ::close(m_fd);
        }

    public:
        // No session for UDP
        void run()
        {
            sockaddr_in addr_client;
            socklen_t socket_len = sizeof(sockaddr_in);
            while(true)
            {
                int read_size = ::recvfrom(m_fd, m_buf, size, 0, (struct sockaddr*)&addr_client, &socket_len);
                if (read_size > 0)
                {
                    std::cout << "\n[UDP server] Received : " << std::string{m_buf, (size_t)read_size};
                    std::cout << " [from " << inet_ntoa(addr_client.sin_addr) << ":"
                                               << ntohs(addr_client.sin_port) << "]";

                    ::sendto(m_fd, m_buf, read_size, 0, (struct sockaddr*)&addr_client, socket_len);
                }
                else
                {
                    std::cout << " \n[UDP server] Read-failure" << std::flush;
                    return;
                }
            }
        }

    private:
        static const std::uint32_t size = 4096;

    private:
        int m_fd;
        char m_buf[size];
    };
}
