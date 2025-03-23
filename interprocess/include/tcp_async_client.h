#pragma once
#include<socket.h>
#include<vector>
#include<unordered_map>


// *********************************************************** //
// *** TCP async client (able to connect to multi servers) *** //
// *********************************************************** //
// BUG1 : create epoll once only
// BUG2 : non blocking connect returns < 0, must check errno 
// BUG3 : non blocking send requires option EPOLLOUT, hence set EPOLLOUT in callback_recv
//        non blocking recv requires option EPOLLIN,  hence set EPOLLIN  in callback_send
//
namespace ipc
{
    class tcp_async_client
    {
    public:
        enum class status
        {
            requested,
            connected,
            disconnected
        };

    public:
        tcp_async_client(const std::string& server_ip,
                         const std::vector<std::uint16_t>& server_ports)
        {
            std::cout << "\n[TCP async client]" << std::flush;


            // ************* //
            // *** Epoll *** //
            // ************* //
            m_fd_epoll = ::epoll_create1(0); // BUG1 : Dont put epoll creation into the for loop, otherwise only last fd is monitored
            if (m_fd_epoll < 0)
            {
                throw std::runtime_error("[TCP async client] Fail to create epoll");
            }

            for(const auto& server_port:server_ports)
            {
                // ****************************** //
                // *** Step 1 : create socket *** //
                // ****************************** //
                int fd = ::socket(AF_INET, SOCK_STREAM, 0);
                if (fd < 0)
                {
                    throw std::runtime_error("[TCP async client] Fail to create socket");
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
                    throw std::runtime_error("[TCP async client] Fail to set non-blocking");
                }


                // ******************************* //
                // *** Step 4 : connect server *** //
                // ******************************* //
                sockaddr_in server_addr;
                server_addr.sin_family      = AF_INET;
                server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
                server_addr.sin_port        = htons(server_port);

                if (::connect(fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0 && 
                    errno != EINPROGRESS) // <--- BUG2 : This is critical for async client, dont miss it.
                {
                    throw std::runtime_error("[TCP async client] Fail to connect");
                }

                m_fd_map[fd] = std::make_pair(get_ip(server_addr), status::requested); 
                std::cout << "\n[TCP async client] Connection to server " << get_ip(server_addr) << ", status::requested" << std::flush;


                // ************* //
                // *** Epoll *** //
                // ************* //
                if (!epoll_add_in_out_event(m_fd_epoll, fd))
                {
                    throw std::runtime_error("[TCP async client] Fail to add fd to epoll");
                }
            }
        }

       ~tcp_async_client() 
        {
            close_all_fds();

            if (m_fd_epoll > 0) ::close(m_fd_epoll);
        }

    public:
        void run()
        {
            static const std::uint32_t MAX_NUM_EVENTS = 64;
            epoll_event events[MAX_NUM_EVENTS];

            while(is_any_server_requested_or_connected()) 
            {
                int num_events = epoll_wait(m_fd_epoll, events, MAX_NUM_EVENTS, -1); 
                for(int n=0; n!=num_events; ++n)
                {
                    int fd = events[n].data.fd;

                    // **************************************** //
                    // *** Event : update connection status *** //
                    // **************************************** //
                    if (is_server_requested(fd))
                    {
                        callback_update_connection_status(fd);
                    }
                    // ******************** //
                    // *** Event : send *** //
                    // ******************** //
                    else if (events[n].events & EPOLLOUT)
                    {
                        callback_send(fd);
                    }
                    // ******************** //
                    // *** Event : recv *** //
                    // ******************** //
                    else
                    {
                        callback_recv(fd);
                    }
                }
            }
            std::cout << "\n[TCP async client] Disconnect from all servers, client terminated" << std::flush;
        }

    private:
        void callback_update_connection_status(int fd)
        {
            bool connected = get_connection_status(fd);
            if (connected)
            {
                m_fd_map[fd].second = status::connected;
                std::cout << "\n[TCP async client] Connection to server " << ip(fd) << ", status::connected" << std::flush;
            }
            else
            {
                m_fd_map[fd].second = status::disconnected;
                std::cout << "\n[TCP async client] Connection to server " << ip(fd) << ", status::disconnected" << std::flush;
                close_fd(fd);
            }
        }

        void callback_send(int fd)
        {
            std::cout << "\n[TCP async client] Send message, server " << ip(fd) << ": " << std::flush;
            std::string message;
            std::getline(std::cin, message); 


            // ********************* //
            // *** Step 5 : send *** //
            // ********************* //
            int sent_total = 0;
            while(sent_total < message.size())
            {
                int sent_size = ::send(fd, message.c_str() + sent_total, message.size() - sent_total, 0);
                if (sent_size > 0)
                {
                    sent_total += sent_size;
                }
                else if (sent_size == 0)
                {
                    std::cout << "\n[TCP async client] Disconnect from server " << ip(fd) << std::flush;
                    close_fd(fd);
                    return;
                }
                else
                {
                    std::cout << "\n[TCP async client] Fail to send to server " << ip(fd) << std::flush;
                    close_fd(fd);
                    return;
                }
            }
            epoll_mod_in_event(m_fd_epoll, fd); // BUG3 : Dont miss this, alternate IN/OUT option
        }

        void callback_recv(int fd)
        {
            // ********************* //
            // *** Step 5 : recv *** //
            // ********************* //
            bool done = false;
            while(!done)
            {
                char buf[4096];

                int recv_size = ::recv(fd, buf, sizeof(buf), 0);
                if (recv_size > 0)
                {
                    std::cout << "\n[TCP async client] Recv message, server " << ip(fd) << ": " << std::string{buf, (size_t)recv_size} << std::flush;
                }
                else if (recv_size == 0)
                {
                    std::cout << "\n[TCP async client] Disconnect from server " << ip(fd) << std::flush;
                    close_fd(fd);
                    return;
                }
                else if (errno != EAGAIN)
                {
                    std::cout << "\n[TCP async client] Fail to recv from server " << ip(fd) << std::flush;
                    close_fd(fd);
                    return;
                }
                else 
                {
                    done = true; // no more to read
                }
            }
            epoll_mod_out_event(m_fd_epoll, fd); // BUG3 : Dont miss this, alternate IN/OUT option
        }
  
    private:    
        std::string ip(int fd) const
        {
            auto iter = m_fd_map.find(fd);
            if (iter != m_fd_map.end())
            {
                return iter->second.first;
            }
            return "unknown";
        }

        void close_fd(int fd)
        {
            epoll_delete(m_fd_epoll, fd);
            ::close(fd);
            
            auto iter = m_fd_map.find(fd);
            if (iter != m_fd_map.end())
            {
                m_fd_map.erase(iter); // avoid duplicated close() on destruction
            }
        }

        void close_all_fds()
        {
            for(auto& x:m_fd_map)
            {
                close_fd(x.first);
            }
        }

    private:    
        bool is_server_requested(int fd) const
        {
            auto iter = m_fd_map.find(fd);
            if (iter != m_fd_map.end() && iter->second.second == status::requested) return true;
            return false;
        }

        bool is_any_server_requested_or_connected() const
        {
            for(const auto& x:m_fd_map)
            {
                if (x.second.second == status::requested ||
                    x.second.second == status::connected) return true;
            }
            return false;
        }

    private:
        int m_fd_epoll;

        // **************************** //
        // key          = server fd 
        // value.first  = server IP
        // value.second = server status
        // **************************** //
        std::unordered_map<int, std::pair<std::string, status>> m_fd_map; 
    };
}

