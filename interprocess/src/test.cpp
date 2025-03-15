#include<iostream>
#include<cstdint>

void test_ipc_unnamed_pipe();
void test_ipc_named_pipe(bool);
void test_ipc_shared_memory();
void test_epoll_socket(std::uint16_t port);
void test_tcp(bool is_server);
void test_udp(bool is_server);

int main(int argc, char* argv[])
{ 
//  test_ipc_unnamed_pipe();   
//  test_ipc_named_pipe(argc > 1); // with argument for producer
//  test_ipc_shared_memory(); 





    if (argc == 2)  
    {
        std::string arg = argv[1];
        if      (arg == "tcp_server_async") test_epoll_socket(12345);
        else if (arg == "tcp_server")       test_tcp(true);
        else if (arg == "tcp_client")       test_tcp(false);
        else if (arg == "udp_server")       test_udp(true);
        else if (arg == "udp_client")       test_udp(false);
    }
    else 
    {
        std::cout << "\nincorrect number of arg";
    }

    std::cout << "\n\n\n";
    return 0;
}
