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


    inline bool add_fd_to_epoll(int epoll_fd, int socket_fd)
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events = EPOLLIN | EPOLLET;

        // No need to keep event alive after setting control
        return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == 0;
    }

    inline bool delete_fd_from_epoll(int epoll_fd, int socket_fd)
    {
        return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, nullptr) == 0;
    }
}


// ********************************************************************************************************************* //
// Address struct
// * struct sockaddr_in           is for IP_v4
// * struct sockaddr              is for generic protocol
//
// In this repo,
// * struct sockaddr_in is used, but it is usually cast as sockaddr for bind(), listen(), recv() and send() functions
//
// ********************************************************************************************************************* //
// How to use setsockopt? It looks like that :
//
// setsockopt(socket_fd, protocol_level, option, option_struct, option_struct_size)
//
//
//
// protocol_level = SOL_SOCKET,   for socket level option
// protocol_level = IPPROTO_IP,   for     IP level option
// protocol_level = IPPROTO_TCP,  for    TCP level option
//
// SOL_SOCKET level option 
// option = SO_REUSEADDR          allow reuse of address (like rebinding)
// option = SO_REUSEPORT          allow resue of port    (like load balancing)
// option = SO_TYPE               get socket type        (TCP = SOCK_STREAM, UDP = SOCK_DGRAM)
// option = SO_ERROR              get socket error status
// option = SO_RCVBUF             set recv buffer size
// option = SO_SNDBUF             set send buffer size
// option = SO_RCVTIMEO           set recv timeout (struct timeval)
// option = SO_SNDTIMEO           set send timeout
// option = SO_BROADCAST          allow sending to broadcast addresses (UDP)
// 
//
// IPPROTO_IP level option 
// option = IP_TTL                set TTL for   unicast packets (TTL is lifetime of packet, unit = router hops)
// option = IP_MULTICAST_TTL      set TTL for multicast packets (TTL is lifetime of packet, unit = router hops) 
// option = IP_MULTICAST_LOOP     enable/disable loopback of multicast (onload issue for qfi_ilp_ice & qfi_mid_ice)
// option = IP_MULTICAST_IF	      set outgoing interface for multicast
// option = IP_ADD_MEMBERSHIP     join a multicast group (clients only)
// option = IP_DROP_MEMBERSHIP    drop a multicast group
// 
// 
// IPPROTO_TCP level option 
// option = TCP_NODELAY           disable Nagle’s algorithm 
// option = TCP_KEEPIDLE          idle time before keepalive probes
// option = TCP_KEEPINTVL         interval between keepalive probes
// option = TCP_KEEPCNT	          number of failed probes before drop
//
//
//
// If you have a few options to set, you need to call setsockopt() couple of times.
// ********************************************************************************************************************* //
