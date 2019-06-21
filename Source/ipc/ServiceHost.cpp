#include <ipc/ServiceHost.hpp>

namespace ipc
{
    struct HomeServicesManagerOptions
    {
        static const size_t PointerBufferSize = 0x500;
        static const size_t MaxDomains = 4;
        static const size_t MaxDomainObjects = 0x100;
    };

    using ServicesManager = WaitableManager<HomeServicesManagerOptions>;

    static void ServicesThread(void *arg)
    {
        auto manager = new ServicesManager(2);
        manager->AddWaitable(new ServiceServer<DialogService>("eq-dlg", 0x40));
        manager->Process();
        delete manager;
    }

    Result StartServiceHostingThread()
    {
        Thread t_Services;
        auto rc = threadCreate(&t_Services, &ServicesThread, NULL, 0x4000, 0x2b, -2);
        if(rc != 0) return rc;
        return threadStart(&t_Services);
    }
}