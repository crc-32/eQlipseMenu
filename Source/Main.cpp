#include <home/System.hpp>
#include <home/Messages.hpp>
#include <home/Mutex.hpp>
#include <home/Consts.hpp>
#include <home/Updates.hpp>
#include <ipc/ServiceHost.hpp>
#include <ui/HomeApplication.hpp>
#include <libget/src/Get.hpp>

extern "C"
{
    u32 __nx_applet_type = AppletType_SystemApplet;
    size_t __nx_heap_size = 0xb400000;
}

home::HomeConfig global_home_menu;
home::Theme global_theme;

ApplicationHolder global_hold_application;
AppletHolder global_hold_applet;

home::Updates *global_update_manager;

Thread t_GeneralChannel;
Thread t_Messages;
extern home::HomeMutex ui_threading_lock;

void HandleHomeButton()
{
    ui::SendFlagToUI(6);
    if(appletHolderRequestExit(&global_hold_applet) == 0) return;
    if(appletApplicationHolderIsInProcess(&global_hold_application))
    {
        if(appletApplicationHolderIsForeground(&global_hold_application))
        {
            ui::SendFlagToUI(2);
            svcSleepThread(250000000L);
            appletApplicationHolderRequestBackground(&global_hold_application);
        }
        else
        {
            ui::SendFlagToUI(1);
            svcSleepThread(250000000L);
            appletApplicationHolderRequestForeground(&global_hold_application);
        }
    }
    else if(appletApplicationHolderIsValid(&global_hold_application))
    {
        appletApplicationHolderClose(&global_hold_application);
        ui::SendFlagToUI(3);
        ui::SendFlagToUI(2);
        svcSleepThread(250000000L);
    }
}

void HandleSleep()
{
    ui::SendFlagToUI(6);
    appletHomeRequestSleep();
}

void GeneralChannelMessageHandle(void *arg)
{
    while(true)
    {
        svcSleepThread(50000000L);
        home::GeneralChannelMessage msg = (home::GeneralChannelMessage)home::GetLatestGeneralChannelMessage();
        if(msg != home::GeneralChannelMessage::Invalid)
        {
            switch(msg)
            {
                case home::GeneralChannelMessage::HomeButton:
                    HandleHomeButton();
                    break;
                case home::GeneralChannelMessage::Sleep:
                    HandleSleep();
                    break;
                case home::GeneralChannelMessage::Shutdown:
                    appletHomeRequestShutdown();
                    break;
                case home::GeneralChannelMessage::Reboot:
                    appletHomeRequestReboot();
                    break;
                default:
                    // ui::AddDialogToDialogQueue("General message", std::to_string((u32)msg), {"Ok"}, true);
                    break;
            }
        }
    }
}

void AppletMessageHandle(void *arg)
{
    while(true)
    {
        svcSleepThread(50000000L);
        home::AppletMessage msg = (home::AppletMessage)home::GetLatestAppletMessage();
        if(msg != home::AppletMessage::Invalid)
        {
            switch(msg)
            {
                case home::AppletMessage::HomeButton:
                    HandleHomeButton();
                    break;
                case home::AppletMessage::PowerButton:
                    HandleSleep();
                    break;
                case home::AppletMessage::SdCardOut:
                    appletHomeRequestShutdown();
                    break;
                default:
                    // ui::AddDialogToDialogQueue("Applet message", std::to_string((u32)msg), {"Ok"}, true);
                    break;
            }
        }
    }
}

Result InitializeThreads()
{
    auto rc = threadCreate(&t_GeneralChannel, &GeneralChannelMessageHandle, NULL, 0x2000, 0x2b, -2);
    if(rc != 0) return rc;
    rc = threadStart(&t_GeneralChannel);
    if(rc != 0) return rc;
    rc = threadCreate(&t_Messages, &AppletMessageHandle, NULL, 0x2000, 0x2b, -2);
    if(rc != 0) return rc;
    rc = threadStart(&t_Messages);
    if(rc != 0) return rc;
    return ipc::StartServiceHostingThread();
}

int main()
{
    svcSleepThread(500000000L);
    home::Initialize();

    auto rc = InitializeThreads();
    if(rc != 0) home::DieExecution("Failed to initialize inner threads.");

    global_update_manager = new home::Updates(home::AppstoreDir  + ".get/");

    home::EnsureHomeConfig();
    global_home_menu = home::ProcessHomeConfig();

    global_theme = home::LoadTheme(global_home_menu.CurrentTheme);

    auto app = new ui::HomeApplication();
    app->Show();
    delete app;
    delete global_update_manager;

    home::Exit();

    return 0;
}