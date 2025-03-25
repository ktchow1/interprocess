#pragma once
#include<socket.h>
#include <sys/types.h>
#include <sys/stat.h>


namespace ipc
{
    void pipe_producer(int& fd, debugger& dbg)
    {
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
    }

    void pipe_consumer(int& fd, debugger& dbg)
    {
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
    }
}


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
// 4. do not use "~/my_pipe" as ~ is not identified by ::mkfifo()
//           use std::getenv("HOME") instead
// *********************************************************************** //
namespace ipc
{
    // *************************** //
    // *** Named pipe producer *** //
    // *************************** //
    class named_pipe_producer
    {
    public:
        explicit named_pipe_producer(const std::string& pipe_name) : m_dbg("[named pipe producer]")
        {
            m_dbg.log();

            std::string full_name = std::string{std::getenv("HOME")} + "/" + pipe_name;
            if (::mkfifo(full_name.c_str(), S_IFIFO|0640) < 0)
            {
                m_dbg.log("Requested pipe already exists, lets continue ...");
            }

            m_fd = ::open(full_name.c_str(), O_CREAT|O_WRONLY, 0755); // <--- blocking until consumer also called open
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail open pipe");
            }
        }

       ~named_pipe_producer()
        {
            if (m_fd > 0) ::close(m_fd);
        }

    public:
        void run()
        {
            pipe_producer(m_fd, m_dbg); 
        }

    private:
        int m_fd;
        debugger m_dbg;
    };


    // *************************** //
    // *** Named pipe consumer *** //
    // *************************** //
    class named_pipe_consumer
    {
    public:
        explicit named_pipe_consumer(const std::string& pipe_name) : m_dbg("[named pipe consumer]")
        {
            m_dbg.log();

            std::string full_name = std::string{std::getenv("HOME")} + "/" + pipe_name;
            if (::mkfifo(full_name.c_str(), S_IFIFO|0640) < 0)
            {
                m_dbg.log("Requested pipe already exists, lets continue ...");
            }

            m_fd = ::open(full_name.c_str(), O_RDONLY, 0755); // <--- blocking until producer also called open
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail open pipe");
            }
        }

       ~named_pipe_consumer()
        {
            if (m_fd > 0) ::close(m_fd);
        }

    public:
        void run()
        {
            pipe_consumer(m_fd, m_dbg); 
        }

    private:
        int m_fd;
        debugger m_dbg;
    };
}

  
// ************************************* //
// About unnamed pipe
// 1. usually used with fork, so that :
//    parent process becomes producer
//    child  process becomes consumer
//    or vice versa ...
// ************************************* //
namespace ipc
{
    inline void unnamed_pipe_fork_producer_and_consumer()
    {
        int fds[2];  // both pipes are copied by fork, copied pipes can be accessed in parent, child
        ::pipe(fds); // both pipes are created together

        if (fork() > 0)
        {
            debugger dbg("[unnamed pipe producer]");
            dbg.log("pid = " + std::to_string(getpid()) + ". ");

            ::close(fds[0]); // producer uses fds[1], close unused fds[0]
            ipc::pipe_producer(fds[1], dbg); 
        }
        else
        {
            debugger dbg("[unnamed pipe consumer]");
            dbg.log("pid = " + std::to_string(getpid()) + ". ");

            ::close(fds[1]); // consumer uses fds[0], close unused fds[1]
            ipc::pipe_consumer(fds[0], dbg);
        }
    }
}

