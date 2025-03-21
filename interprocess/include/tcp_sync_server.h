#pragma once
#include<socket.h>


// ********************************************* //
// *** TCP sync server (serve 1 client only) *** //
// ********************************************* //
// socket used for accepting connection is called passive socket
// socket used for recv/send to client  is called  active socket
//
// TCP server usually has :
// 1 passive socket + 
// N  active sockets
//
namespace ipc
{
    class tcp_sync_session
    {
    public:
        tcp_sync_session(int fd_active, const sockaddr_in& client_addr) : m_fd_active(fd_active), m_client_addr(client_addr)
        {
            std::cout << "\n[TCP sync server] Connection from " << get_ip(m_client_addr) << std::flush;
        }

       ~tcp_sync_session() 
        {
            if (m_fd_active > 0) ::close(m_fd_active);
        }

    public:
        bool run()
        {
            while(true)
            {
                // **************************** //
                // *** Step 5 : recv & send *** //
                // **************************** //
                char buf[4096];

                int recv_size = ::recv(m_fd_active, buf, sizeof(buf), 0);
                if (recv_size > 0)
                {
                    :: send(m_fd_active, buf, recv_size, 0); // 0 means default option
                //  ::write(m_fd_active, buf, recv_size);
                }
                else if (recv_size == 0)
                {
                    std::cout << "\n[TCP sync server] Disconnect from " << get_ip(m_client_addr) << std::flush;
                    return true;
                }
                else
                {
                    std::cout << "\n[TCP sync server] Read failure" << std::flush;
                    return false;
                }
            }
        }

    private:
        int m_fd_active;
        sockaddr_in m_client_addr;
    };
}


namespace ipc
{
    class tcp_sync_server
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        tcp_sync_server(std::uint16_t server_port) : m_fd_passive(::socket(AF_INET, SOCK_STREAM, 0))
        {
            std::cout << "[TCP sync server]" << std::flush;
            if (m_fd_passive < 0)
            {
                throw std::runtime_error("[TCP sync server] Cannot create socket");
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
                throw std::runtime_error("[TCP sync server] Cannot bind socket");
            }


            // ****************************** //
            // *** Step 3 : listen socket *** //
            // ****************************** //
            if (::listen(m_fd_passive, SOMAXCONN) < 0) // 2nd arg = num of pending connection in queue 
            {
                throw std::runtime_error("[TCP sync server] Cannot listen socket");
            }
        }

       ~tcp_sync_server() 
        {
            if (m_fd_passive > 0) ::close(m_fd_passive);
        }

    public:
        void run()
        {
            while(true) // when client disconnected, server can accept next client
            {
                auto session = accept();
                session.run();
            }
        }
       
        tcp_sync_session accept()
        {
            // ****************************** //
            // *** Step 4 : accept client *** //
            // ****************************** //
            sockaddr_in client_addr;
            socklen_t size = sizeof(client_addr);

            int fd_active = ::accept(m_fd_passive, (struct sockaddr*)(&client_addr), &size); // unlike bind, pass address of size
            if (fd_active < 0)
            {
                throw std::runtime_error("[TCP sync server] Cannot accept connection");
            }
            return tcp_sync_session(fd_active, client_addr);
        }

    private:
        int m_fd_passive;
    };
}
