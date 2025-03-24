#include<iostream>
#include<cstdint>

#include<tcp_sync_client.h>
#include<tcp_sync_server.h>
#include<tcp_async_client.h>
#include<tcp_async_server.h>
#include<udp_unicast_server.h>
#include<udp_unicast_client.h>
#include<udp_multicast_server.h>
#include<udp_multicast_client.h>



// void test_ipc_unnamed_pipe()
// void test_ipc_named_pipe(bool)
// void test_ipc_shared_memory()


std::uint16_t get_uint(int argc, char* argv[], std::uint32_t index, std::uint16_t default_value)
{
    std::uint16_t ans = default_value;
    if (argc >= index+1)
    {
        ans = std::stoi(argv[index]);
    }
    return ans;
}

std::string get_str(int argc, char* argv[], std::uint32_t index, std::string default_value)
{
    std::string ans = default_value;
    if (argc >= index+1)
    {
        ans = std::string(argv[index]);
    }
    return ans;
}



// ********************************************************* //
//      tcp_sync_server vs      tcp_sync_client = 1:1
//     tcp_async_server vs     tcp_async_client = 1:N or M:1
//   udp_unicast_server vs   udp_unicast_client = 1:N
// udp_multicast_server vs udp_multicast_client = 1:N or M:1
//
// ********************************************************* //
// Example 1, running all TCP server/client together :
// ./build/debug/Test ss  
// ./build/debug/Test as 12346
// ./build/debug/Test ac 12345 12346
// ./build/debug/Test sc 12346
//
// Example 2, unicast to multi clients
// ./build/debug/Test us
// ./build/debug/Test uc
// ./build/debug/Test uc
//
// ********************************************************* //
void print_manual()
{
    std::cout << "\nincorrect arg, please run as : ";
    std::cout << "\n";
    std::cout << "\n./build/debug/Test ss (or tcp_sync_server)      [server_port]               ";
    std::cout << "\n./build/debug/Test sc (or tcp_sync_client)      [server_port] [client_port] "; //            support  self_pick_client_port
    std::cout << "\n./build/debug/Test as (or tcp_async_server)     [server_port]               "; // use epoll
    std::cout << "\n./build/debug/Test ac (or tcp_async_client)     [server_port]...            "; // use epoll, support   connection to multi servers
    std::cout << "\n./build/debug/Test us (or udp_unicast_server)   [server_port]               "; //  no epoll, support  publication to multi clients
    std::cout << "\n./build/debug/Test uc (or udp_unicast_client)   [server_port]               "; //            support  self_pick_client_port
    std::cout << "\n./build/debug/Test ms (or udp_multicast_server) [server_port]               ";
    std::cout << "\n./build/debug/Test mc (or udp_multicast_client) [server_port]...            "; // use epoll, support subscription to multi groups
    std::cout << "\n";
    std::cout << "\n[]  means optional";
    std::cout << "\n... means variadic";
    std::cout << "\nyou can connect tcp_async_server from tcp_sync_client ot tcp_async_client";
    std::cout << "\nyou can connect  tcp_sync_server from tcp_sync_client ot tcp_async_client";

}


int main(int argc, char* argv[])
{ 
    if (argc >= 2)  
    {
        std::string arg = argv[1];

        // **************** //
        // *** TCP sync *** //
        // **************** //
        if (arg == "tcp_sync_server" || arg == "ss")  
        {
            ipc::tcp_sync_server server
            (
                get_uint(argc, argv, 2, 12345)
            );   
            server.run();
        }
        else if (arg == "tcp_sync_client" || arg == "sc") 
        {
            ipc::tcp_sync_client client
            (
                "127.0.0.1", 
                get_uint(argc, argv, 2, 12345),  // server port
                get_uint(argc, argv, 3, 0)       // client port
            );
            client.run();
        }


        // ***************** //
        // *** TCP async *** //
        // ***************** //
        else if (arg == "as" || arg == "tcp_async_server")    
        {
            ipc::tcp_async_server server
            {
                get_uint(argc, argv, 2, 12345)
            };
            server.run();
        }
        else if (arg == "ac" || arg == "tcp_async_client") 
        {
            std::vector<std::uint16_t> server_ports;
            for(int n=2; n!=argc; ++n)
            {
                server_ports.push_back(std::stoi(argv[n]));
            }

            ipc::tcp_async_client client
            (
                "127.0.0.1", server_ports 
            );
            client.run();
        }


        // ******************* //
        // *** UDP unicast *** //
        // ******************* //
        else if (arg == "us" || arg == "udp_unicast_server")   
        {
            ipc::udp_unicast_server server
            {
                get_uint(argc, argv, 2, 12345)
            };
            server.run();
        }
        else if (arg == "uc" || arg == "udp_unicast_client")   
        {
            ipc::udp_unicast_client client
            {
                "127.0.0.1",    
                get_uint(argc, argv, 2, 12345)
            };
            client.run();
        }


        // ********************* //
        // *** UDP multicast *** //
        // ********************* //
        else if (arg == "ms" || arg == "udp_multicast_server") 
        {
            test_udp_multicast_server("239.0.0.1", 12345);
        }
        else if (arg == "mc" || arg == "udp_multicast_client")
        {
            test_udp_multicast_client("239.0.0.1", 12345);
        }
//      else if (arg == "npp" || arg == "named_pipe_producer")      test_ipc_named_pipe(true);
//      else if (arg == "npc" || arg == "named_pipe_consumer")      test_ipc_named_pipe(false);
//      else if (arg == "up"  || arg == "unnamed_pipe")             test_ipc_unnamed_pipe();
//      else if (arg == "sm"  || arg == "shared_memory")            test_ipc_shared_memory();
        else print_manual();
    }
    else print_manual();

    std::cout << "\n\n\n";
    return 0;
}
