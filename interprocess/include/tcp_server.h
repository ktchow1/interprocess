#include<iostream>
#include<cstdint>
#include<string>
#include<sys/socket.h> 
#include<arpa/inet.h> 
#include<unistd.h> // read & write

class tcp_session
{
public:
    tcp_session(int fd_) : fd(fd_) {}
   ~tcp_session() 
    {
        if (fd > 0) ::close(fd);
    }

public:
    bool run()
    {
        std::cout << "\n[TCP server] Connection done" << std::flush;
        while(true)
        {
            // ******************** //
            // *** READ & WRITE *** //
            // ******************** //
            int read_size = ::recv(fd ,buf, size, 0);
            if (read_size > 0)
            {
                ::write(fd, buf, read_size);
            }
            else if (read_size == 0)
            {
                std::cout << "\n[TCP server] Disconnected" << std::flush;
                return true;
            }
            else
            {
                std::cout << "\n[TCP server] Read-failure" << std::flush;
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

// **************************************************************** //
// Step 1 : Create socket
// Step 2 : Bind socket to port
// Step 3 : Put socket to listening mode (become a passive socket)
// Step 4 : Accept new connection to spawn an active socket
// **************************************************************** //
class tcp_server
{
public:
    // Step 1 : Create socket
    tcp_server(std::uint16_t port) : fd(::socket(AF_INET, SOCK_STREAM, 0))
    {
        if (fd == -1)
        {
            throw std::runtime_error("[TCP server] Cannot create socket");
        }

        sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(port);

        // Step 2 : Bind socket (without query)
        if (::bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        {
            throw std::runtime_error("[TCP server] Cannot bind socket");
        }

        // Step 3 : Listen mode
        ::listen(fd,3);
    }

   ~tcp_server() 
    {
        if (fd > 0) ::close(fd);
    }

public:
    tcp_session accept()
    {
        sockaddr_in client_addr;
        socklen_t socket_len = sizeof(sockaddr_in);

        // Step 4 : Accept connection and spawn active-socket
        int client_fd = ::accept(fd, (struct sockaddr*)(&client_addr), &socket_len);
        if (client_fd < 0)
        {
            throw std::runtime_error("[TCP server] Cannot accept connection");
        }
        return tcp_session(client_fd);
    }

private:
    int fd;
};

