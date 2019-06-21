
#pragma once
#include <switch.h>
#include <thread>
#include <mutex>

namespace home
{
    enum class GeneralChannelMessage
    {
        Invalid,
        HomeButton = 2,
        Sleep = 3,
        Shutdown = 5,
        Reboot = 6,
        OverlayBrightValueChanged = 13,
        OverlayAutoBrightnessChanged = 14,
        OverlayAirplaneModeChanged = 15,
        HomeButtonHold = 16,
        OverlayHidden = 17,
    };

    enum class AppletMessage
    {
        Invalid,
        Exit = 4,
        FocusStateChange = 0xf,
        HomeButton = 0x14,
        PowerButton = 22,
        BackFromSleep = 26,
        ChangeOperationMode = 0x1e,
        ChangePerformanceMode = 0x1f,
        SdCardOut = 33,
    };

    u32 GetLatestGeneralChannelMessage();
    u32 GetLatestAppletMessage();
}