#pragma once
#include<socket.h>
#include<unordered_map>


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
        tcp_async_server(std::uint16_t server_port) : m_fd_passive(::socket(AF_INET, SOCK_STREAM, 0)), m_dbg("[TCP async server]")
        {
            m_dbg.log();
            if (m_fd_passive < 0)
            {
                m_dbg.throw_exception("Fail to create socket");
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
                m_dbg.throw_exception("Fail to bind");
            }
        

            // **************************************** //
            // *** Step 3 : unblock & listen socket *** //
            // **************************************** //
            if (!set_socket_non_blocking(m_fd_passive)) 
            {
                m_dbg.throw_exception("Fail to set non-blocking");
            }

            if (::listen(m_fd_passive, SOMAXCONN) < 0)
            {
                m_dbg.throw_exception("Fail to listen");
            }


            // ************* //
            // *** Epoll *** //
            // ************* //
            m_fd_epoll = ::epoll_create1(0);
            if (m_fd_epoll < 0)
            {
                m_dbg.throw_exception("Fail to create epoll");
            }

            if (!epoll_add_in_event(m_fd_epoll, m_fd_passive))
            {
                m_dbg.throw_exception("Fail to add socket to epoll");
            }
        }

       ~tcp_async_server() 
        {
            close_all_active_fds();

            if (m_fd_passive > 0) ::close(m_fd_passive);
            if (m_fd_epoll   > 0) ::close(m_fd_epoll);
        }

    public:
        void run() 
        {
            static const std::uint32_t MAX_NUM_EVENTS = 64;
            epoll_event events[MAX_NUM_EVENTS];

            while(true)
            {
                int num_events = epoll_wait(m_fd_epoll, events, MAX_NUM_EVENTS, -1); // <--- thread is blocked here 
                for(int n=0; n!=num_events; ++n)
                {
                    int fd = events[n].data.fd;

                    // ************************** //
                    // *** Event : disconnect *** //
                    // ************************** //
                    if ((events[n].events & EPOLLERR) ||
                        (events[n].events & EPOLLHUP) ||
                       !(events[n].events & EPOLLIN ))
                    {
                        callback_disconnect(fd);
                    }
                    // *********************** //
                    // *** Event : connect *** //
                    // *********************** //
                    else if (fd == m_fd_passive)
                    {
                        callback_accept(fd);
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
        }

    private:
        void callback_disconnect(int fd_active)
        {
            m_dbg.log("Disconnect from client ", ip(fd_active), " (rare path)");
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
                m_dbg.log("Fail to accept, continue ...");
            }
            else
            {
                m_fd_active_map[fd_active] = get_ip(client_addr);
                m_dbg.log("Connection from client ", get_ip(client_addr));

                if (!set_socket_non_blocking(fd_active))
                {
                    m_dbg.log("Fail to set non-blocking, continue ...");
                }

                if (!epoll_add_in_event(m_fd_epoll, fd_active))
                {
                    m_dbg.log("Fail to add socket to epoll, continue ...");
                }
            }
        }

        void callback_recv(int fd_active)
        {
            // **************************** //
            // *** Step 5 : recv & send *** //
            // **************************** //
            // * need while a loop to ensure all data are consumed
            // * "errno" will be EAGAIN when all data are consumed
            // * "errno" is a thread-local global variable
            // * EAGAIN means epoll wait again
            //
            bool done = false;
            while(!done)
            {
                char buf[32]; // use small size intentionally 

                int recv_size = ::recv(fd_active, buf, sizeof(buf), 0);
                if (recv_size > 0)
                {
                    :: send(fd_active, buf, recv_size, 0);
                //  ::write(fd_active, buf, recv_size);
                }
                else if (recv_size == 0)
                {
                    m_dbg.log("Disconnect from client ", ip(fd_active));
                    close_active_fd(fd_active);
                    return;
                }
                else if (errno != EAGAIN)
                {
                    m_dbg.log("Fail to recv from client ", ip(fd_active));
                    close_active_fd(fd_active);
                    return;
                }
                else 
                {
                    done = true; // no more to read
                }
            }
        }

    private:
        std::string ip(int fd) const
        {
            auto iter = m_fd_active_map.find(fd);
            if (iter != m_fd_active_map.end())
            {
                return iter->second;
            }
            return "unknown";
        }

        void close_active_fd(int fd)
        {
            epoll_delete(m_fd_epoll, fd);
            ::close(fd);
            
            auto iter = m_fd_active_map.find(fd);
            if (iter != m_fd_active_map.end())
            {
                m_fd_active_map.erase(iter); // avoid duplicated close() on destruction
            }
        }
        
        void close_all_active_fds()
        {
            for(auto& x:m_fd_active_map)
            {
                close_active_fd(x.first);
            }
        }

    private:
        int m_fd_epoll;
        int m_fd_passive;

        //****************** //
        // key   = active fd
        // value = active IP
        //****************** //
        std::unordered_map<int, std::string> m_fd_active_map;
        debugger m_dbg;
    };
}

