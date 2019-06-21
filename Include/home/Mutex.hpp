
#pragma once
#include <switch.h>
#include <thread>
#include <mutex>

namespace home
{
    class HomeMutex
    {
        public:
            HomeMutex();

            // For std::scoped_lock compatibility
            void lock();
            void unlock();
            bool try_lock();
        private:
            Mutex mtx;
    };

    typedef std::scoped_lock<HomeMutex> HomeMutexLock;
}