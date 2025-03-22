#pragma once
#include<socket.h>
#include<vector>
#include<unordered_map>


// *********************************************************** //
// *** TCP async client (able to connect to multi servers) *** //
// *********************************************************** //
namespace ipc
{
    class tcp_async_client
    {
    public:
        tcp_async_client(const std::string& server_ip,
                         const std::vector<std::uint16_t>& server_ports)
        {
            std::cout << "[TCP async client]" << std::flush;
            for(const auto& server_port:server_ports)
            {
                // ****************************** //
                // *** Step 1 : create socket *** //
                // ****************************** //
                int fd = ::socket(AF_INET, SOCK_STREAM, 0);
                if (fd < 0)
                {
                    throw std::runtime_error("[TCP async client] Cannot create socket");
                }


                // ******************************************** //
                // *** Step 2 : bind socket (to NIC & port) *** //
                // ******************************************** //
                // skip this part for simplicity, refer to tcp_sync_client for binding BIC
            

                // ******************************* //
                // *** Step 3 : unblock socket *** //
                // ******************************* //
                if (!set_socket_non_blocking(fd)) 
                {
                    throw std::runtime_error("[TCP async client] Cannot set socket non_blocking");
                }


                // ******************************* //
                // *** Step 4 : connect server *** //
                // ******************************* //
                sockaddr_in server_addr;
                server_addr.sin_family      = AF_INET;
                server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
                server_addr.sin_port        = htons(server_port);

                if (::connect(fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0)
                {
                    throw std::runtime_error("[TCP async client] Cannot connect server");
                }

                m_fd2ip[fd] = get_ip(server_addr);
                std::cout << "\n[TCP async client] Connected to " << get_ip(server_addr) << std::flush;


                // ************* //
                // *** Epoll *** //
                // ************* //
            }
        }

       ~tcp_async_client() 
        {
            for(auto& x:m_fd2ip)
            {
                if (x.first > 0) ::close(x.first);
            }
            if (m_fd_epoll > 0)  ::close(m_fd_epoll);
        }

    public:
   /*     void run()
        {
            m_fd_epoll = ::epoll_create1(0);
            if (m_fd_epoll < 0)
            {
                throw std::runtime_error("[TCP async client] Cannot create epoll");
            }

            if (!add_fd_to_epoll(m_fd_epoll, m_fd_passive))
            {
                throw std::runtime_error("[TCP async client] Cannot add passive socket to epoll");
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
                    // ************************** //
                    // *** Event : disconnect *** //
                    // ************************** //
                    if ((events[n].events & EPOLLERR) ||
                        (events[n].events & EPOLLHUP) ||
                       !(events[n].events & EPOLLIN ))
                    {
                        callback_disconnect(events[n].data.fd);
                    }
                    // *********************** //
                    // *** Event : connect *** //
                    // *********************** //
                    else if (events[n].data.fd == m_fd_passive)
                    {
                        callback_accept(events[n].data.fd);
                    }
                    // ******************** //
                    // *** Event : recv *** //
                    // ******************** //
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
            std::cout << "\n[TCP async client] Disconnect (rare) from " << m_fd2ip[fd_active] << std::flush;
            close_active_fd(fd_active);
        }

        void callback_accept(int fd_passive)
        {
            // ****************************** //
            // *** Step 4 : accept client *** //
            // ****************************** //
            sockaddr_in client_addr;
            socklen_t size = sizeof(client_addr);

            int fd_active = accept(fd_passive, (struct sockaddr*)(&client_addr), &size); 
            if (fd_active < 0)
            {
                std::cout << "[TCP async client] Cannot accept connection, continue ...";
            }
            else
            {
                m_fd2ip[fd_active] = get_ip(client_addr);
                std::cout << "\n[TCP sync server] Connection from " << get_ip(client_addr) << std::flush;

                if (!set_socket_non_blocking(fd_active))
                {
                    std::cout << "\n[TCP async client] Cannot set socket non_blocking, continue ...";
                }

                if (!add_fd_to_epoll(m_fd_epoll, fd_active))
                {
                    std::cout << "\n[TCP async client] Cannot add active socket to epoll, continue ...";
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
                std::cout << "\n[TCP async client] Disconnect from " << m_fd2ip[fd_active] << std::flush;
                close_active_fd(fd_active);
            }
            else
            {
                std::cout << "\n[TCP async client] Read failure" << std::flush;
                close_active_fd(fd_active);
            }
        }

        void close_active_fd(int fd)
        {
            delete_fd_from_epoll(m_fd_epoll, fd);
            ::close(fd);
        }
*/
    private:
        std::unordered_map<int, std::string> m_fd2ip; 
        int m_fd_epoll;
    };
}

