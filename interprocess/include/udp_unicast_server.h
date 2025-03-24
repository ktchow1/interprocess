#pragma once
#include<socket.h>


namespace ipc
{
    class udp_unicast_server
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        udp_unicast_server(std::uint16_t server_port)
                          : m_fd(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
                          , m_dbg("[UDP unicast server]")
        {
            m_dbg.log();
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail to create");
            }


            // ******************************************** //
            // *** Step 2 : bind socket (to NIC & port) *** //
            // ******************************************** //
            sockaddr_in server_addr;
            server_addr.sin_family      = AF_INET;
            server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            server_addr.sin_port        = htons(server_port);

            if (::bind(m_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0)
            {
                m_dbg.throw_exception("Fail to bind");
            }

            // ************************ //
            // *** Step 3 : skipped *** //
            // *** Step 4 : skipped *** //
            // ************************ //
        }

       ~udp_unicast_server()
        {
            if (m_fd > 0) ::close(m_fd);
        }

    public:
        void run()
        {
            sockaddr_in client_addr;
            socklen_t size = sizeof(sockaddr_in);

            while(true)
            {
                // **************************** //
                // *** Step 5 : recv & send *** //
                // **************************** //
                char buf[4096];

                int recv_size = ::recvfrom(m_fd, buf, sizeof(buf), 0, (struct sockaddr*)(&client_addr), &size);
                if (recv_size > 0)
                {
                    ::sendto(m_fd, buf, recv_size, 0, (struct sockaddr*)(&client_addr), size);
                    m_dbg.log("Received and sent to ", get_ip(client_addr));
                }
                else
                {
                    m_dbg.log("Fail to recv");
                    return;
                }
            }
        }

    private:
        int m_fd;
        debugger m_dbg;
    };
}
