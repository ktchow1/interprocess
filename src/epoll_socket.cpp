#include <iostream>
#include <cstdint>

#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#define MAXEVENTS 64

// ************** //
// *** Step 1 *** //
// ************** //
int create_and_bind(std::uint16_t port)
{
    addrinfo hints;
    addrinfo *result;

    // *** Step A : Initial address hints *** //
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM; // request TCP socket
    hints.ai_family   = AF_UNSPEC;   // request both IPv4 and IPv6
    hints.ai_flags    = AI_PASSIVE;

    // *** Step B : Get address results *** //
    std::string port_str = std::to_string(port);
    if (auto status = getaddrinfo(NULL, port_str.c_str(), &hints, &result) != 0) return -1;

    // *** Step C : Iterate address results *** //
    for(const auto* result_ptr = result; result_ptr!=NULL; result_ptr = result_ptr->ai_next)
    {
        // *** Step 1 : Create socket *** //
        int fd = socket(result_ptr->ai_family,
                        result_ptr->ai_socktype,
                        result_ptr->ai_protocol);
        if (fd == -1) continue;

        // *** Step 2 : Bind socket *** //
        if (bind(fd, result_ptr->ai_addr,
                     result_ptr->ai_addrlen) == 0)
        {
            freeaddrinfo(result);
            return fd;
        }
        close(fd);
    }

    freeaddrinfo(result);
    return -1;
}

// *** Step D : Non blocking *** //
int set_socket_non_blocking(int fd)
{
    // Get flags
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;

    // Set flags
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) return -1;
    return 0;
}

int create_passive_socket(std::uint16_t port)
{
    int FD_passive = create_and_bind(port);
    if (FD_passive == -1)                          return -1;
    if (set_socket_non_blocking(FD_passive) == -1) return -1;
    if (listen(FD_passive, SOMAXCONN) == -1)       return -1; // *** Step 3 *** //

    return FD_passive;
}

// *************** //
// *** Helpers *** //
// *************** //
std::string get_client_ip(const sockaddr& addr, const socklen_t& len)
{
    char buf0[NI_MAXHOST];
    char buf1[NI_MAXSERV];
    std::string str;

    if (getnameinfo(&addr, len, buf0, sizeof(buf0),
                                buf1, sizeof(buf1),
                                NI_NUMERICHOST | NI_NUMERICSERV) == 0)
    {
        using namespace std::string_literals;
        str = std::string(buf0) + ":"s + std::string(buf1);
    }
    return str;
}

int register_fd_to_epoll(int FD_epoll, int FD)
{
    struct epoll_event event;
    event.data.fd = FD;
    event.events = EPOLLIN | EPOLLET;
    return epoll_ctl(FD_epoll, EPOLL_CTL_ADD, FD, &event);
}

// *************** //
// *** Testing *** //
// *************** //
void test_epoll_socket(std::uint16_t port)
{
    int FD_passive = create_passive_socket(port);
    if (FD_passive == -1)
    {
        std::cout << "\nFail to create passive socket";
        return;
    }

    int FD_epoll = epoll_create1(0);
    if (FD_epoll == -1)
    {
        std::cout << "\nFail to create epoll";
        return;
    }

    if (register_fd_to_epoll(FD_epoll, FD_passive) == -1)
    {
        std::cout << "\nFail to register fd to epoll";
        return;
    }

    // ********************** //
    // *** The event loop *** //
    // ********************** //
    epoll_event events[MAXEVENTS];
    while(true)
    {
        // Blocked until ready ...
        int num_events = epoll_wait(FD_epoll, events, MAXEVENTS, -1);
        for(int n=0; n!=num_events; ++n)
        {
            // ******************************* //
            // *** Event 1 : Disconnection *** //
            // ******************************* //
            if ((events[n].events & EPOLLERR) ||
                (events[n].events & EPOLLHUP) ||
               !(events[n].events & EPOLLIN ))
            {
                std::cout << "\nDisconnection detected";
                close(events[n].data.fd);
            }
            // ******************************** //
            // *** Event 2 : New connection *** //
            // ******************************** //
            else if (FD_passive == events[n].data.fd)
            {
                sockaddr sock_addr;
                socklen_t sock_len = sizeof(sock_addr);

                int FD_active = accept(FD_passive, &sock_addr, &sock_len); // *** Step 4 *** //
                if (FD_active == -1)
                {
                    std::cout << "\nError in acceptng new connection, next event ... ";
                }
                else
                {
                    std::cout << "\nAccepted connection from " << get_client_ip(sock_addr, sock_len);

                    if (set_socket_non_blocking(FD_active) == -1)
                    {
                        std::cout << "\nFail to set socket non blocking";
                        return;
                    }

                    if (register_fd_to_epoll(FD_epoll, FD_active) == -1)
                    {
                        std::cout << "\nFail to register fd to epoll";
                        return;
                    }
                }
            }
            // ************************************************** //
            // *** Event 3 : Message from existing connection *** //
            // ************************************************** //
            else
            {
                char buf[512];
                size_t count = read(events[n].data.fd, buf, sizeof(buf));
                if (count == -1)
                {
                    std::cout << "\nConnection read failure";
                    close(events[n].data.fd);
                    break;
                }
                else if (count == 0)
                {
                    std::cout << "\nDisconnection detected [EOF]";
                    close(events[n].data.fd);
                    break;
                }

                std::cout << "\n" << std::string(buf, count);
                write(events[n].data.fd, buf, count); // echo back
            }
        }
    }
    close(FD_passive);
}
