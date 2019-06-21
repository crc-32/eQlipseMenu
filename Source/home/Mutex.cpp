#include <home/Mutex.hpp>

namespace home
{
    HomeMutex::HomeMutex()
    {
        mutexInit(&mtx);
    }

    void HomeMutex::lock()
    {
        mutexLock(&mtx);
    }

    void HomeMutex::unlock()
    {
        mutexUnlock(&mtx);
    }

    bool HomeMutex::try_lock()
    {
        return mutexTryLock(&mtx);
    }
}