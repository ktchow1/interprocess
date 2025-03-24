#pragma once
#include<iostream>
#include<cstdint>
#include<string>
#include<string.h>

// socket 
#include<sys/socket.h> 
#include<arpa/inet.h> 
#include<unistd.h> 

// epoll
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>


namespace ipc
{
    class debugger
    {
    public:
        explicit debugger(const std::string& name) : m_name(name)
        {
        }

        void throw_exception(const std::string& ex)
        {
            throw std::runtime_error(m_name + " " + ex);
        }

        template<typename...ARGS>
        void log(ARGS&&...args)
        {
            (std::cout << "\n" << m_name << " " << ... << std::forward<ARGS>(args)) << std::flush;
        }

    private:
        std::string m_name;
    };
}


// ********************************************************************************************************************* //
// About struct sockaddr
// * struct sockaddr_in            is for IP_v4
// * struct sockaddr               is for generic protocol
//
// In this repo :
// * struct sockaddr_in is used most of the time, but ...
// * struct sockaddr_in is usually casted into sockaddr before calling bind() / listen() / recv() / send() 
//
// ********************************************************************************************************************* //
// About function setsockopt()
//
// setsockopt(socket_fd, protocol_level, option, option_struct, option_struct_size)
// * protocol_level = SOL_SOCKET,  for socket level option
// * protocol_level = IPPROTO_IP,  for     IP level option
// * protocol_level = IPPROTO_TCP, for    TCP level option
//
// SOL_SOCKET level option 
// * option = SO_REUSEADDR         allow reuse of address (like rebinding)
// * option = SO_REUSEPORT         allow resue of port    (like load balancing)
// * option = SO_TYPE              get socket type        (TCP = SOCK_STREAM, UDP = SOCK_DGRAM)
// * option = SO_ERROR             get socket error status
// * option = SO_RCVBUF            set recv buffer size
// * option = SO_SNDBUF            set send buffer size
// * option = SO_RCVTIMEO          set recv timeout (struct timeval)
// * option = SO_SNDTIMEO          set send timeout
// * option = SO_BROADCAST         allow sending to broadcast addresses (UDP)
//
// IPPROTO_IP level option 
// * option = IP_TTL               set TTL for   unicast packets (TTL is lifetime of packet, unit = router hops)
// * option = IP_MULTICAST_TTL     set TTL for multicast packets (TTL is lifetime of packet, unit = router hops) 
// * option = IP_MULTICAST_LOOP    enable/disable loopback of multicast (onload issue for qfi_ilp_ice & qfi_mid_ice)
// * option = IP_MULTICAST_IF      set outgoing interface for multicast
// * option = IP_ADD_MEMBERSHIP    join a multicast group (clients only)
// * option = IP_DROP_MEMBERSHIP   drop a multicast group
// 
// IPPROTO_TCP level option 
// * option = TCP_NODELAY          disable Nagle’s algorithm 
// * option = TCP_KEEPIDLE         idle time before keepalive probes
// * option = TCP_KEEPINTVL        interval between keepalive probes
// * option = TCP_KEEPCNT          number of failed probes before drop
//
// If you have a few options to set, you need to call setsockopt() couple of times.
// ********************************************************************************************************************* //
// About function getsockopt()
// 
// * it is used once in async client to check if connection to server is successful or not
// ********************************************************************************************************************* //
  
namespace ipc
{
    inline std::string get_ip(const sockaddr_in& addr)
    {
        std::string ip(inet_ntoa(addr.sin_addr)); 
        std::uint32_t port(ntohs(addr.sin_port));
        return ip + ":" + std::to_string(port);
    }


    inline bool set_socket_non_blocking(int fd)
    {
        // ***************** //
        // *** get flags *** //
        // ***************** //
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) 
        {
            return false;
        }

        // ***************** //
        // *** set flags *** //
        // ***************** //
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0) 
        {
            return false;
        }
        return true;
    }

    inline bool get_connection_status(int fd)
    {
        int error;
        socklen_t size = sizeof(error);

        if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &size) < 0 || error != 0)
        {
            return false;
        }
        return true;
    }


}


// ******************************************************************************************** //
// When a socket is blocking :
// * recv(fd) will block, until data is available.
//
// When a socket is non-blocking :
// * recv(fd) will not block and returns data         when data is     available, 
// * recv(fd) will not block and returns EAGAIN error when data is not available.
// * hence we need a while loop logic for non-blocking socke
//
// ******************************************************************************************** //
// About epoll event : 
// * when to use EPOLLIN?    waiting to recv / read / accept
// * when to use EPOLLOUT?   waiting to send / write / "get connection result by getsockopt()"
//
// ******************************************************************************************** //
// Why do we need to wait for EPOLLOUT before sending in TCP async client,
//    but no need to wait for EPOLLOUT before sending in TCP async server (for echo)?
//
// This is because :
// * async client needs to wait for successful connection to server
// * async server has already connected to client via accept
//
// ******************************************************************************************** //
// Why do we always have EPOLLIN  with EPOLLET (edge trigger)
//            while have EPOlLOUT without it  (level trigger)?
//
// This is because :
// * for EPOLLIN, if level trigger is used, there will be a lot of events when data comes in 
//   is better to use edge trigger, read socket repeatedly until no more data can be read
// * for EPOLLOUT, if edge trigger is used, there will be rate events, or even missing event
//   when socket write-queue is ready, also using level trigger to avoid while-loop handling
//
// ******************************************************************************************** //
// For tcp_async_client, it has to do both send and recv async, hence need to :
//
// * modify socket option to  IN before next recv
// * modify socket option to OUT before next send
// ******************************************************************************************** //

namespace ipc
{
    inline bool epoll_add_in_out_event(int epoll_fd, int socket_fd)
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events  = EPOLLIN | EPOLLOUT; 
        return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == 0; 
    }


    inline bool epoll_add_in_event(int epoll_fd, int socket_fd)
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events  = EPOLLIN | EPOLLET; // wait until ready-to-read, with edge-trigger
        return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == 0; 
    }


    inline bool epoll_add_out_event(int epoll_fd, int socket_fd)
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events  = EPOLLOUT; // wait until ready-to-write, with level-trigger 
        return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == 0; 
    }
    

    inline bool epoll_mod_in_event(int epoll_fd, int socket_fd)
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events  = EPOLLIN | EPOLLET; 
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &event) == 0; 
    }


    inline bool epoll_mod_out_event(int epoll_fd, int socket_fd)
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events  = EPOLLOUT; 
        return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &event) == 0; 
    }


    inline bool epoll_delete(int epoll_fd, int socket_fd)
    {
        return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, nullptr) == 0;
    }
}


