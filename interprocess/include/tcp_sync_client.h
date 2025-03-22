#pragma once
#include<socket.h>


namespace ipc
{
    class tcp_sync_client
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        tcp_sync_client(const std::string&  server_ip, 
                              std::uint16_t server_port, 
                              std::uint16_t client_port) // 0 for OS-pick port, >0 for custom-pick port
                              : m_fd(::socket(AF_INET, SOCK_STREAM, 0))
        {
            std::cout << "[TCP sync client]" << std::flush;
            if (m_fd < 0)
            {
                throw std::runtime_error("[TCP sync client] Cannot create socket");
            }


            // ******************************************** //
            // *** Step 2 : bind socket (to NIC & port) *** //
            // ******************************************** //
            // If we want to use specific NIC,         fill in "client_addr.sin_addr.s_addr".
            // If we want to use specific client port, fill in "client_addr.sin_port", otherwise OS wlll pick randomly.
            //
            if (client_port > 0)
            {
                sockaddr_in client_addr;
                client_addr.sin_family      = AF_INET;
                client_addr.sin_addr.s_addr = INADDR_ANY; 
                client_addr.sin_port        = htons(client_port);

                if (::bind(m_fd, (struct sockaddr*)(&client_addr), sizeof(client_addr)) < 0)
                {
                    throw std::runtime_error("[TCP sync client] Cannot bind socket");
                } 
            }


            // ******************************* //
            // *** Step 3 : connect server *** //
            // ******************************* //
            {
                sockaddr_in server_addr;
                server_addr.sin_family      = AF_INET;
                server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
                server_addr.sin_port        = htons(server_port);

                if (::connect(m_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0)
                {
                    throw std::runtime_error("[TCP sync client] Cannot connect server");
                }
            }
        }

       ~tcp_sync_client()
        {
            if (m_fd > 0) ::close(m_fd);
        }


    public:
        bool run()
        {
            while(true)
            {
                std::cout << "\n[TCP sync client] Enter message : " << std::flush;
                std::string message;
                std::cin >> message;
                if (message == "quit" ||
                    message == "exit")
                {
                    std::cout << "[TCP sync client] Disconnected by client" << std::flush;
                    return true;
                }



                // ********************* //
                // *** Step 5 : send *** //
                // ********************* //
                if (::send(m_fd, message.c_str(), message.size(), 0) < 0)
                {
                    throw std::runtime_error("[TCP sync client] Cannot send");
                }


                // ********************* //
                // *** Step 5 : recv *** //
                // ********************* //
                char buf[4096];

                int recv_size = ::recv(m_fd, buf, sizeof(buf), 0);
                if (recv_size > 0)
                {
                    std::cout << "[TCP sync client] Received : " << std::string{buf, (size_t)recv_size};
                }
                else if (recv_size == 0)
                {
                    std::cout << "[TCP sync client] Disconnect from server" << std::flush;
                    return true;
                }
                else
                {
                    std::cout << "[TCP sync client] Cannot recv" << std::flush;
                    return false;
                }
            }
        }

    private:
        int m_fd;
    };
}
