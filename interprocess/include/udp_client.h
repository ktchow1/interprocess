#include<iostream>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h> // read & write

class udp_client
{
public:
    // Step 1 : Create socket
    udp_client(const std::string& ip, std::uint16_t port) : fd(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
    {
        if (fd == -1)
        {
            throw std::runtime_error("[UDP client] Cannot create socket");
        }

        addr_server.sin_family = AF_INET;
        addr_server.sin_port   = htons(port);
        inet_aton(ip.c_str(), &addr_server.sin_addr);
    }
    
   ~udp_client()
    {
        if (fd > 0) ::close(fd);
    }

public:
    bool run()
    {
        socklen_t socket_len = sizeof(sockaddr_in);
        while(true)
        {
            std::cout << "\n[UDP client] Enter message : " << std::flush;
            std::string message;
            std::cin >> message;

            // ************* //
            // *** WRITE *** //
            // ************* //
            if (sendto(fd, message.c_str(), message.size(), 0, (struct sockaddr*)&addr_server, socket_len) < 0)
            {
                throw std::runtime_error("[UDP client] Cannot send");
            }

            // ************ //
            // *** READ *** //
            // ************ //
            int read_size = ::recvfrom(fd, buf, size, 0, (struct sockaddr*)&addr_server, &socket_len);
            if (read_size > 0)
            {
                std::cout << "[UDP client] Received : " << std::string{buf, (size_t)read_size};
            }
            else if (read_size)
            {
                std::cout << "[UDP client] Disconnected" << std::flush;
                return true;
            }
            else
            {
                std::cout << "[UDP client] Read-failure" << std::flush;
                return false;
            }
        }
        close(fd);
        return true;
    }

private:
    static const std::uint32_t size = 4096;
    static const std::uint16_t port = 12345;

private:
    int fd;
    char buf[size];

    // Unlike TCP, there is no connection in UDP, we need to keep the address ...
    sockaddr_in addr_server;
};
