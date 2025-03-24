#pragma once
#include<socket.h>


namespace ipc
{
    class udp_unicast_client
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        udp_unicast_client(const std::string&  server_ip, 
                                 std::uint16_t server_port)
                                 : m_fd(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
                                 , m_dbg("[UDP unicast client]")
        {
            m_dbg.log();
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail to create");
            }


            // ************************************** //
            // *** Step 2 : config addr (no bind) *** //
            // ************************************** //
            m_server_addr.sin_family      = AF_INET;
            m_server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
            m_server_addr.sin_port        = htons(server_port);
        }
        
       ~udp_unicast_client()
        {
            if (m_fd > 0) ::close(m_fd);
        }

    public:
        void run()
        {
            socklen_t size = sizeof(sockaddr_in);
            while(true)
            {
                m_dbg.log("Send message: ");
                std::string message;
                std::getline(std::cin, message);


                // ********************* //
                // *** Step 5 : send *** //
                // ********************* //
                if (::sendto(m_fd, message.c_str(), message.size(), 0, (struct sockaddr*)(&m_server_addr), size) < 0)
                {
                    m_dbg.throw_exception("Fail to send");
                }


                // ********************* //
                // *** Step 5 : recv *** //
                // ********************* //
                char buf[4096];

                int recv_size = ::recvfrom(m_fd, buf, sizeof(buf), 0, (struct sockaddr*)(&m_server_addr), &size);
                if (recv_size > 0)
                {
                    m_dbg.log("Recv message : ", std::string{buf, (size_t)recv_size});
                }
                else if (recv_size == 0)
                {
                    m_dbg.log("Disconnect from server (UDP)");
                    return;
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

    private:
        sockaddr_in m_server_addr;
    };
}
