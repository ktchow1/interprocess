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
/*
    // Please communicate with server using TCP client (both C++ or python are OK)
    {
        if (argc!=2)
        {
            std::cout << "\nError in input, please enter : exe 12345";
        }
        else
        {
            test_epoll_socket(std::stoi(std::string(argv[1])));
        }
    } */


    // ******************************************** //
    // *** TCP and UDP server and client (sync) *** //
    // ******************************************** //
    // invoke server by one dummy argument
    // invoke client by no extra argument
    // ******************************************** //
//  test_tcp(argc > 1);
    test_udp(argc > 1);
    std::cout << "\n\n\n";
    return 0;
}
