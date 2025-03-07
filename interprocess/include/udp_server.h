#include<iostream>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

class udp_server
{
public:
    // Step 1. Create socket
    udp_server(std::uint16_t port) : fd(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
    {
        if (fd == -1)
        {
            throw std::runtime_error("[UDP server] Cannot create socket");
        }

        sockaddr_in addr_this;
        addr_this.sin_family = AF_INET;
        addr_this.sin_port = htons(port);
        addr_this.sin_addr.s_addr = htonl(INADDR_ANY);

        // Step 2. Bind socket
        if (::bind(fd, (struct sockaddr*)&addr_this, sizeof(addr_this)) == -1)
        {
            throw std::runtime_error("[UDP server] Cannot bind socket");
        }
    }

   ~udp_server()
    {
        if (fd > 0) ::close(fd);
    }

public:
    // No session for UDP
    void run()
    {
        sockaddr_in addr_client;
        socklen_t socket_len = sizeof(sockaddr_in);
        while(true)
        {
            int read_size = ::recvfrom(fd, buf, size, 0, (struct sockaddr*)&addr_client, &socket_len);
            if (read_size > 0)
            {
                std::cout << "\n[UDP server] Received : " << std::string{buf, (size_t)read_size};
                std::cout << " [from " << inet_ntoa(addr_client.sin_addr) << ":"
                                           << ntohs(addr_client.sin_port) << "]";

                ::sendto(fd, buf, read_size, 0, (struct sockaddr*)&addr_client, socket_len);
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
    int fd;
    char buf[size];
};
