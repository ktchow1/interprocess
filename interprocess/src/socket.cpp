#include<tcp_server.h>
#include<tcp_client.h>
#include<udp_server.h>
#include<udp_client.h>


// Run executable with a dummy arg for server.
// Run executable without arg for client.
void test_tcp(bool is_server)
{
    if (is_server)
    {
        std::cout << "[TCP server]" << std::flush;
        ipc::tcp_server server(12345);
        while(true)
        {
            auto session = server.accept();
            session.run();
        }
    }
    else
    {
        std::cout << "[TCP client]" << std::flush;
        ipc::tcp_client client("127.0.0.1", 12345);
        client.run();
    }
}

void test_udp(bool is_server)
{
    if (is_server)
    {
        std::cout << "[UDP server]" << std::flush;
        ipc::udp_server server(12345);
        server.run();
    }
    else
    {
        std::cout << "[UDP client]" << std::flush;
        ipc::udp_client client("127.0.0.1", 12345);
        client.run();
    }
}
