#include <home/Messages.hpp>

namespace home
{
    u32 GetLatestGeneralChannelMessage()
    {
        AppletStorage stg;
        auto rc = appletHomeGeneralChannelPop(&stg);
        if(R_SUCCEEDED(rc))
        {
            u8 data[0x10] = {0};
            rc = appletStorageRead(&stg, 0, data, 0x10);
            if(R_SUCCEEDED(rc))
            {
                appletStorageClose(&stg);
                return data[8];
            }
        }
        return 0;
    }

    u32 GetLatestAppletMessage()
    {
        u32 msg = 0;
        appletGetMessage(&msg);
        return msg;
    }
}