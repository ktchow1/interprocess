#pragma once
#include<unordered_map>
#include<socket.h>


// ******************************************************* //
// *** TCP async server (serve N clients, using epoll) *** //
// ******************************************************* //
namespace ipc
{
    class tcp_async_server
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        tcp_async_server(std::uint16_t server_port) : m_fd_passive(::socket(AF_INET, SOCK_STREAM, 0))
        {
            std::cout << "[TCP async server]" << std::flush;
            if (m_fd_passive < 0)
            {
                throw std::runtime_error("[TCP async server] Cannot create socket");
            }


            // ******************************************** //
            // *** Step 2 : bind socket (to NIC & port) *** //
            // ******************************************** //
            sockaddr_in server_addr;
            server_addr.sin_family      = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY; 
            server_addr.sin_port        = htons(server_port);

            if (::bind(m_fd_passive, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0)
            {
                throw std::runtime_error("[TCP async server] Cannot bind socket");
            }
        

            // **************************************** //
            // *** Step 3 : unblock & listen socket *** //
            // **************************************** //
            if (!set_socket_non_blocking(m_fd_passive)) 
            {
                throw std::runtime_error("[TCP async server] Cannot set socket non_blocking");
            }

            if (::listen(m_fd_passive, SOMAXCONN) < 0)
            {
                throw std::runtime_error("[TCP async server] Cannot listen socket");
            }
        }

       ~tcp_async_server() 
        {
            if (m_fd_passive > 0)  ::close(m_fd_passive);
            if (m_fd_epoll   > 0)  ::close(m_fd_epoll);
        }

    public:
        void run()
        {
            m_fd_epoll = ::epoll_create1(0);
            if (m_fd_epoll < 0)
            {
                throw std::runtime_error("[TCP async server] Cannot create epoll");
            }

            if (!add_fd_to_epoll(m_fd_epoll, m_fd_passive))
            {
                throw std::runtime_error("[TCP async server] Cannot add passive socket to epoll");
            }
            
            // ****************** //
            // *** Event loop *** //
            // ****************** //
            static const std::uint32_t MAX_NUM_EVENTS = 64;
            epoll_event events[MAX_NUM_EVENTS];

            while(true)
            {
                int num_events = epoll_wait(m_fd_epoll, events, MAX_NUM_EVENTS, -1); // <--- thread is blocked here 
                for(int n=0; n!=num_events; ++n)
                {
                    // ******************************* //
                    // *** Event 1 : Disconnection *** //
                    // ******************************* //
                    if ((events[n].events & EPOLLERR) ||
                        (events[n].events & EPOLLHUP) ||
                       !(events[n].events & EPOLLIN ))
                    {
                        callback_disconnect(events[n].data.fd);
                    }
                    // ******************************** //
                    // *** Event 2 : New connection *** //
                    // ******************************** //
                    else if (events[n].data.fd == m_fd_passive)
                    {
                        callback_accept(events[n].data.fd);
                    }
                    // ****************************** //
                    // *** Event 3 : Message recv *** //
                    // ****************************** //
                    else
                    {
                        callback_recv(events[n].data.fd);
                    }
                }
            }
        }

    private:
        void callback_disconnect(int fd_active)
        {
            std::cout << "\n[TCP async server] Disconnect (rare) from " << m_fd2ip[fd_active] << std::flush;
            close_active_fd(fd_active);
        }

        void callback_accept(int fd_passive)
        {
            // **************************************************** //
            // *** Step 4 : accept connection and create socket *** //
            // **************************************************** //
            sockaddr_in client_addr;
            socklen_t size = sizeof(client_addr);

            int fd_active = accept(fd_passive, (struct sockaddr*)(&client_addr), &size); 
            if (fd_active < 0)
            {
                std::cout << "[TCP async server] Cannot accept connection, continue ...";
            }
            else
            {
                m_fd2ip[fd_active] = get_ip(client_addr);
                std::cout << "\n[TCP sync server] Connection from " << get_ip(client_addr) << std::flush;

                if (!set_socket_non_blocking(fd_active))
                {
                    std::cout << "\n[TCP async server] Cannot set socket non_blocking, continue ...";
                }

                if (!add_fd_to_epoll(m_fd_epoll, fd_active))
                {
                    std::cout << "\n[TCP async server] Cannot add active socket to epoll, continue ...";
                }
            }
        }

        void callback_recv(int fd_active)
        {
            // **************************** //
            // *** Step 5 : recv & send *** //
            // **************************** //
            char buf[4096];

            int recv_size = ::recv(fd_active, buf, sizeof(buf), 0);
            if (recv_size > 0)
            {
                :: send(fd_active, buf, recv_size, 0);
            //  ::write(fd_active, buf, recv_size);
            }
            else if (recv_size == 0)
            {
                std::cout << "\n[TCP async server] Disconnect from " << m_fd2ip[fd_active] << std::flush;
                close_active_fd(fd_active);
            }
            else
            {
                std::cout << "\n[TCP async server] Read failure" << std::flush;
                close_active_fd(fd_active);
            }
        }

        void close_active_fd(int fd)
        {
            delete_fd_from_epoll(m_fd_epoll, fd);
            ::close(fd);
        }

    private:
        int m_fd_passive;
        int m_fd_epoll;

    private:
        std::unordered_map<int, std::string> m_fd2ip;
    };
}

