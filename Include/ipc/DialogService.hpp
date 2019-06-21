
#pragma once
#include <stratosphere.hpp>

namespace ipc
{
    class DialogService : public IServiceObject
    {
        protected:
            Result AddDialogToQueue(InBuffer<char> title, InBuffer<char> contents, InBuffer<char> button);

        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MakeServiceCommandMeta<0, &DialogService::AddDialogToQueue>()
            };
    };
}