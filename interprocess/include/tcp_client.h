#pragma once
#include<interprocess.h>


namespace ipc
{
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

/*
            // Step 2 : Bind to one NIC (if there are multi NICs)
            sockaddr_in client_addr;
            client_addr.sin_family      = AF_INET;
        //  client_addr.sin_addr.s_addr = inet_addr("192.168.1.34"); 
            client_addr.sin_addr.s_addr = inet_addr("192.168.1.156"); // hard code desired private IP
            client_addr.sin_port        = 0;                          // OS picks available port

            if (::bind(fd, (struct sockaddr*)(&client_addr), sizeof(client_addr)) <= 0)
            {
                throw std::runtime_error("[TCP client] Cannot bind NIC");
            } */


            // Step 3 : Connection
            sockaddr_in server_addr;
            server_addr.sin_family      = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
            server_addr.sin_port        = htons(port);

            if (::connect(fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0)
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
                if (message == "quit")
                {
                    std::cout << "[TCP client] Disconnected by client" << std::flush;
                    return true;
                }

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
                int read_size = ::recv(fd, buf, size, 0);
                if (read_size > 0)
                {
                    std::cout << "[TCP client] Received : " << std::string{buf, (size_t)read_size};
                }
                else if (read_size)
                {
                    std::cout << "[TCP client] Disconnected by server" << std::flush;
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
}
