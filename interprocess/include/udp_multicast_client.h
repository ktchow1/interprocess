#pragma once
#include<socket.h>



namespace ipc
{
    class udp_multicast_client
    {
    public:
        // ****************************** //
        // *** Step 1 : create socket *** //
        // ****************************** //
        udp_multicast_client(const std::string&  multicast_group_ip, 
                                   std::uint16_t multicast_group_port)
                                   : m_fd(::socket(AF_INET, SOCK_DGRAM, 0)) 
                                   , m_dbg("[UDP mulitcast client]")
        {
            m_dbg.log();
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail to create");
            }



            // *************************************** //
            // *** Step 2 : bind socket (to group) *** //
            // *************************************** //
            // This is the API design :
            // * setting group port by bind() 
            // * setting group ip by setsockopt() 
            //
            
            struct sockaddr_in multicast_group_addr;
            multicast_group_addr.sin_family      = AF_INET;
            multicast_group_addr.sin_addr.s_addr = INADDR_ANY; // NIC 
            multicast_group_addr.sin_port        = htons(multicast_group_port);

            if (::bind(m_fd, (struct sockaddr*)(&multicast_group_addr), sizeof(multicast_group_addr)) < 0)
            {
                m_dbg.throw_exception("Fail to bind");
            }

            // Join multicast group
            m_multicast_request.imr_multiaddr.s_addr = inet_addr(multicast_group_ip.c_str());
            m_multicast_request.imr_interface.s_addr = INADDR_ANY;
            if (::setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_multicast_request, sizeof(m_multicast_request)) < 0)
            {
                m_dbg.throw_exception("Fail to join group");
            }


            // *************************** //
            // *** Step 3 : set option *** //
            // *************************** //
            // Set "reusable" option using setsockopt() function. 
            // Allows multi clients in same host to listen same port, 
            // otherwise it results in "address already in use" ERROR.
            //
            // Function setsockopt() is hence used twice :
            // * setting group ip
            // * setting option, make port reusable
              
            int reuse = 1;
            if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
            {
                m_dbg.throw_exception("Fail to set reuse option");
            }
        }

       ~udp_multicast_client() 
        {
            // Quit multicast group
            ::setsockopt(m_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &m_multicast_request, sizeof(m_multicast_request));

            if (m_fd > 0) ::close(m_fd);
        }

    public:
        void run()
        {
            while (true) 
            {
                // ********************* //
                // *** Step 5 : recv *** //
                // ********************* //
                char buf[4096];

                int recv_size = recvfrom(m_fd, buf, sizeof(buf), 0, nullptr, nullptr);
                if (recv_size > 0)
                {
                    m_dbg.log("Recv message: ", std::string{buf, (size_t)recv_size});
                }
                else if (recv_size == 0)
                {
                    m_dbg.log("Disconnect from server");
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
        ip_mreq m_multicast_request;
    };
}


