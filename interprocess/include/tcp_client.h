#include<iostream>
#include<string>
#include<sys/socket.h> 
#include<arpa/inet.h>
#include<unistd.h> // read & write

class tcp_client
{
public:
    // Step 1 : Create socket
    tcp_client(const std::string& ip, std::uint16_t port) : fd(::socket(AF_INET, SOCK_STREAM, 0))
    {
        if (fd == -1)
        {
            throw std::runtime_error("[TCP client] Cannot create socket");
        }

        sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port        = htons(port);

        // Step 2 : Connection
        if (::connect(fd, (struct sockaddr*)(&addr), sizeof(addr)) == -1)
        {
            throw std::runtime_error("[TCP client] Cannot connect server");
        }
        std::cout << "\n[TCP client] Connection done" << std::flush;
    }

   ~tcp_client()
    {
        if (fd > 0) ::close(fd);
    }

public:
    bool run()
    {
        while(true)
        {
            std::cout << "\n[TCP client] Enter message : " << std::flush;
            std::string message;
            std::cin >> message;

            // ************* //
            // *** WRITE *** //
            // ************* //
            if (::send(fd, message.c_str(), message.size(), 0) < 0)
            {
                throw std::runtime_error("[TCP client] Cannot send");
            }

            // ************ //
            // *** READ *** //
            // ************ //
            int read_size = ::recv(fd , buf, size, 0);
            if (read_size > 0)
            {
                std::cout << "[TCP client] Received : " << std::string{buf, (size_t)read_size};
            }
            else if (read_size)
            {
                std::cout << "[TCP client] Disconnected" << std::flush;
                return true;
            }
            else
            {
                std::cout << "[TCP client] Read-failure" << std::flush;
                return false;
            }
        }
    }

private:
    static const std::uint32_t size = 4096;

private:
    int fd;
    char buf[size];
};
