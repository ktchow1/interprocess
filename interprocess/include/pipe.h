#pragma once
#include<socket.h>
#include <sys/types.h>
#include <sys/stat.h>


// *********************************************************************** //
// About named pipe
// 1. test pipe in bash, using 2 terminals
//
// consumer$ cd ~
// consumer$ mkfifo my_pipe
// consumer$ cat my_pipe           <--- block here until producer produces
// producer$ cd ~
// producer$ echo ABCDEF ~/my_pipe
//
// alternatively ...
//
// producer$ cd ~
// consumer$ mkfifo my_pipe
// producer$ echo ABCDEF ~/my_pipe <--- block here until consumer consumes
// consumer$ cd ~
// consumer$ cat my_pipe 
//
// 2. named pipe cant be created in /mnt
//    named pipe can  be created in $HOME
// 3. named pipe can  be created in c++ code using ::mkfifo()
//
//
// About unnamed pipe
// 1. usually used with fork, so that :
//    parent process becomes producer
//    child  process becomes consumer
//    or vice versa ...
// *********************************************************************** //
namespace ipc
{
    void pipe_producer(int& fd, const std::string& name)
    {
        debugger dbg(name);
        while(true)
        {
            dbg.log("Send message: ");
            std::string message;
            std::getline(std::cin, message);
            

            if (::write(fd, message.c_str(), message.size()) < 0)
            {
                dbg.log("Fail to send");
                break;
            }
            if (message == "exit" || 
                message == "quit") break;
        }
        ::close(fd);
    }

    void pipe_consumer(int& fd, const std::string& name)
    {
        debugger dbg(name);
        while(true)
        {
            char buf[4096];

            int recv_size = ::read(fd, buf, sizeof(buf));
            if (recv_size > 0)
            {
                dbg.log("Recv message: ", std::string{buf, (size_t)recv_size});
            }
            else if (recv_size == 0)
            {
                dbg.log("Disconnect from producer");
                break;
            }
            else
            {
                dbg.log("Fail to recv");
                break;
            }

            std::string message{buf, (size_t)recv_size};
            if (message == "exit" || 
                message == "quit") break;
        }
        ::close(fd);
    }
}


namespace iopc
{


}

void test_ipc_named_pipe(bool is_producer)
{
  
    std::string name = std::string{std::getenv("HOME")} + "/my_pipe";
    if (is_producer)
    {
        // give full path please
    //  ::mkfifo(name.c_str(), S_IFIFO|0640); // 0777
        int fd = ::open(name.c_str(), O_CREAT|O_WRONLY, 0755);
        std::cout << "producer fd = " << fd << std::flush;
        ipc::pipe_producer(fd, "named pipe"); 
    }
    else
    {


        ::mkfifo(name.c_str(), S_IFIFO|0640); // 0777
        int fd = ::open(name.c_str(), O_RDONLY, 0755); 
        std::cout << "consumer fd = " << fd << std::flush;
        ipc::pipe_consumer(fd, "named pipe"); // this thing crash, don't know why? 
    }
}


void test_ipc_unnamed_pipe()
{
    // Fork will copy the pipes, copied pipes can be accessed in parent, child.
    int fds[2]; 
    ::pipe(fds); // create both pipes together

    if (fork() > 0)
    {
        ::close(fds[0]); // close unused fd
        ipc::pipe_producer(fds[1], "unnamed pipe"); // fds[1] is for writer
    }
    else
    {
        ::close(fds[1]); // close unused fd
        ipc::pipe_consumer(fds[0], "unnamed pipe"); // fds[0] is for reading
    }
}




