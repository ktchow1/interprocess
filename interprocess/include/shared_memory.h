#pragma once
#include <socket.h>
#include <sys/mman.h> // for mmap
#include <atomic> 


// ****************************************************************************** //
// How does producer know that consumer finishes consuming, before next write?
//
// Answer : synchronization with atomic ready flag 
// ****************************************************************************** //
namespace ipc
{
    struct protocol
    {
        std::atomic<std::uint32_t> ready; // modified by both producer and consumer
        std::uint32_t              producer_pid;
        std::uint32_t              seqnum;
        std::uint32_t              message_size;
        char                       message[4096];
    };


    class shared_memory_producer
    {
    public:
        explicit shared_memory_producer(const std::string& sm_name)
                                       : m_sm_name("[shared memory producer]")
                                       , m_fd(::shm_open(sm_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))
                                       , m_dbg(m_sm_name)
        {
            // **************************** //
            // *** Create shared memory *** //
            // **************************** //
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail in shm_open");
            }
            
            if (::ftruncate(m_fd, sizeof(protocol)) < 0) // truncate to fixed length
            {
                ::close(m_fd);
                ::shm_unlink(m_sm_name.c_str());
                m_dbg.throw_exception("Fail in ftruncate");
            }

            m_protocol = reinterpret_cast<protocol*>(::mmap(NULL, sizeof(protocol), PROT_WRITE, MAP_SHARED, m_fd, 0));
            if (m_protocol == MAP_FAILED)
            {
                ::close(m_fd);
                ::shm_unlink(m_sm_name.c_str());
                m_dbg.throw_exception("Fail in mmap");
            }


            // ********************* //
            // *** Init protocol *** //
            // ********************* //
            std::memset(m_protocol, 0, sizeof(protocol));
            std::uint32_t seqnum = 1000;
        }

       ~shared_memory_producer()
        {
            ::close(m_fd);
            ::shm_unlink(m_sm_name.c_str());
        }

    private:
        const std::string m_sm_name;

    private:
        int m_fd;
        debugger m_dbg;

    private:
        protocol* m_protocol;
    };

    




    const std::string buffer_name = "ABC";
    const int buffer_size = 4096;

    void shared_memory_producer()
    {

        auto fd   = ::shm_open(buffer_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        auto res  = ::ftruncate(fd, buffer_size); // truncate buffer size to fixed length
        char* addr = reinterpret_cast<char*>(::mmap(NULL, buffer_size, PROT_WRITE, MAP_SHARED, fd, 0));
        std::memset(addr, 0, buffer_size);
        std::uint32_t seqnum = 1000;

        while(true)
        {
            std::cout << "Send message : " << std::flush; // no newline

            std::string message;
            std::getline(std::cin, message);
            std::uint32_t length = message.size();

            std::memcpy(reinterpret_cast<std::uint32_t*>(addr),     &seqnum, sizeof(seqnum));
            std::memcpy(reinterpret_cast<std::uint32_t*>(addr + 4), &length, sizeof(length));
            std::memcpy(reinterpret_cast<char*>         (addr + 8), message.c_str(), message.size());

            ++seqnum;
            if (message == "exit" || 
                message == "quit") break;

        }
        ::munmap(addr, buffer_size);
        ::shm_unlink(buffer_name.c_str());
    }

    void shared_memory_consumer()
    {
        auto  fd   = ::shm_open(buffer_name.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
        char* addr = reinterpret_cast<char*>(::mmap(NULL, buffer_size, PROT_READ, MAP_SHARED, fd, 0));

        while(true)
        {
            std::uint32_t seqnum = *reinterpret_cast<std::uint32_t*>(addr);
            std::uint32_t length = *reinterpret_cast<std::uint32_t*>(addr + 4);
            std::string   message  (reinterpret_cast<char*>(addr + 8), length);

            std::cout << "\nRecv msg_" << seqnum << " : " << message << std::flush;
            if (message == "exit" || 
                message == "quit") break;
        }
    }
}

