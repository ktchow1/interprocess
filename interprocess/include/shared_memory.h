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
        std::atomic<std::uint32_t> ready; // 0 for ready-to-write, 1 for ready-to-read
        std::uint32_t              producer_pid;
        std::uint32_t              seqnum;
        std::uint32_t              message_size;
        char                       message[4096];
    };
}


// **************** //
// *** Producer *** //
// **************** //
namespace ipc
{
    class shared_memory_producer
    {
    public:
        explicit shared_memory_producer(const std::string& sm_name)
                                       : m_sm_name(sm_name)
                                       , m_fd(::shm_open(sm_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))
                                       , m_dbg("[shared memory producer]")
        {
            m_dbg.log();

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

            m_protocol = reinterpret_cast<protocol*>(::mmap(NULL, sizeof(protocol), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));
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
            m_protocol->ready.store(0, std::memory_order_release);
            m_protocol->producer_pid = getpid();
            m_protocol->seqnum       = 999;
            m_protocol->message_size = 0;
        }

       ~shared_memory_producer()
        {
            // Follow these steps in order (reverse of construction) : 
            ::munmap(m_protocol, sizeof(protocol));
            ::close(m_fd);
            ::shm_unlink(m_sm_name.c_str());
        }

    public:
        void run()
        {
            while(true)
            {
                std::cout << "Send message : " << std::flush; // no newline
                std::string message;
                std::getline(std::cin, message);
 

                // **************************************** //
                // *** Block until it is ready-to-write *** //
                // **************************************** //
                while(m_protocol->ready.load(std::memory_order_acquire) == 1)
                {
                    std::this_thread::yield();
                }

                std::memcpy(m_protocol->message, message.c_str(), message.size());
                m_protocol->message_size = message.size();
                m_protocol->producer_pid = getpid();
                m_protocol->seqnum      += 1;
                m_protocol->ready.store(1, std::memory_order_release); 

                if (message == "exit" || 
                    message == "quit") break;
            }
        }

    private:
        const std::string m_sm_name;
        protocol* m_protocol;

    private:
        int m_fd;
        debugger m_dbg;
    };
}
    

// **************** //
// *** Consumer *** //
// **************** //
namespace ipc
{
    class shared_memory_consumer
    {
    public:
        explicit shared_memory_consumer(const std::string& sm_name)
                                       : m_sm_name(sm_name)
                                       , m_fd(::shm_open(sm_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) // using RDONLY is wrong, as it writes to ready_flag
                                       , m_dbg("[shared memory consumer]")
        {
            m_dbg.log();

            // **************************** //
            // *** Create shared memory *** //
            // **************************** //
            if (m_fd < 0)
            {
                m_dbg.throw_exception("Fail in shm_open");
            }

            m_protocol = reinterpret_cast<protocol*>(::mmap(NULL, sizeof(protocol), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));
            if (m_protocol == MAP_FAILED)
            {
                ::close(m_fd);
                m_dbg.throw_exception("Fail in mmap");
            }
        }

       ~shared_memory_consumer()
        {
            // No shm_unlink() is needed, as it is done by producer
            ::munmap(m_protocol, sizeof(protocol));
            ::close(m_fd);
        }

    public:
        void run()
        {
            while(true)
            {
                // *************************************** //
                // *** Block until it is ready-to-read *** //
                // *************************************** //
                while(m_protocol->ready.load(std::memory_order_acquire) == 0)
                {
                    std::this_thread::yield();
                }


                std::string message{m_protocol->message, m_protocol->message_size};
                std::cout << "\nRecv message_" << m_protocol->seqnum 
                                   << " from " << m_protocol->producer_pid
                                       << ": " << message << std::flush;

                m_protocol->ready.store(0, std::memory_order_release); 
                if (message == "exit" || 
                    message == "quit") break;
            }
        }

    private:
        const std::string m_sm_name;
        protocol* m_protocol;

    private:
        int m_fd;
        debugger m_dbg;
    };
}

