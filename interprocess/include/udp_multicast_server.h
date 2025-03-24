#pragma once
#include<socket.h>
#include<thread>


namespace ipc
{
    class udp_multicast_server
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        udp_multicast_server(const std::string&  multicast_group_ip, 
                                   std::uint16_t multicast_group_port)
                                   : m_fd(::socket(AF_INET, SOCK_DGRAM, 0)) 
                                   , m_dbg("[UDP mulitcast server]")
        {
            m_dbg.log();
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail to create");
            }


            // ************************************** //
            // *** Step 2 : config addr (no bind) *** //
            // ************************************** //
            m_group_addr.sin_family      = AF_INET;
            m_group_addr.sin_addr.s_addr = inet_addr(multicast_group_ip.c_str());
            m_group_addr.sin_port        = htons(multicast_group_port);


            // ******************************************************************** //
            // Optional : Set TTL (time-to-live), determines max num of ROUTER-HOPS 
            // ******************************************************************** //
        //  int ttl = 1;
        //  if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) 
        //  {
        //      m_dbg.throw_exception("Fail to set socket option");
        //      close(m_fd);
        //  } 
        }

       ~udp_multicast_server()
        {
            if (m_fd > 0) ::close(m_fd);
        }

    public:
        void run()
        {
            std::uint32_t n = 0;
            while(true) 
            {
                std::string message = std::string("This is multicast message ") + std::to_string(n); 
                ++n;

                // ********************* //
                // *** Step 5 : send *** //
                // ********************* //
                if (::sendto(m_fd, message.c_str(), message.size(), 0, (const sockaddr*)(&m_group_addr), sizeof(m_group_addr)) < 0) 
                {
                    m_dbg.throw_exception("Fail to send");
                    close(m_fd);
                    return;
                } 
                else 
                {
                    m_dbg.log("Sent message: ", message);
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }

    private:
        int m_fd;
        debugger m_dbg;

    private:
        sockaddr_in m_group_addr;
    };
}












