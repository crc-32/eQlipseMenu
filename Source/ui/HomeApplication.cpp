#include <ui/HomeApplication.hpp>
#include <home/Mutex.hpp>

std::vector<ui::DialogShowData> dialog_queue;
int ui_communicate_flag = -1;

home::HomeMutex ui_threading_lock;

ui::HomeApplication *app_instance;
extern home::HomeConfig global_home_menu;
extern home::Theme global_theme;
pu::overlay::Toast *notif;
pu::music::Music bgm = NULL;

namespace ui
{
    HomeApplication::HomeApplication()
    {
        pu::render::SetDefaultFont("romfs:/Gilroy-Bold.ttf");
        AddThread(std::bind(&HomeApplication::DialogQueueThread, this));
        app_instance = this;
        notif = new pu::overlay::Toast("", 25, { 225, 225, 225, 255 }, { 15, 15, 15, 255 });
        boot = new BootLayout();
        LoadLayout(boot);
        FadeIn();
        CallForRender(); // The time these 3 layouts take to be created the bootlogo will remain there
        main = new MainLayout();
        allHb = new AllHomebrewLayout();
        launch = new TitleLaunchLayout();
        FadeOut();
        LoadLayout(main);
        if(!global_theme.Generic.BGM.empty())
        {
            bgm = pu::music::Open(global_home_menu.AbsolutePath(global_theme.Generic.BGM));
            pu::music::PlayWithFadeIn(bgm, -1, 1000);
        }
        FadeIn();
    }

    void HomeApplication::DialogQueueThread()
    {
        home::HomeMutexLock lck(ui_threading_lock);
        if(ui_communicate_flag > 0)
        {
            switch(ui_communicate_flag)
            {
                case 1:
                    pu::music::FadeOut(500);
                    LoadTitleLaunchScreen();
                    break;
                case 2:
                    pu::music::PlayWithFadeIn(bgm, -1, 1500);
                    ResetLoaded();
                    break;
                case 3:
                    main->NotifyTitleTerminated();
                    break;
                case 4:
                    FadeIn();
                    break;
                case 5:
                    FadeOut();
                    break;
                case 6:
                    bool changed = false;
                    if(home::CheckContentAdded(global_home_menu))
                    {
                        changed = true;
                        ShowNotification("New title(s) were installed. Main menu has been updated.");
                    }
                    if(home::CheckContentRemoved(global_home_menu))
                    {
                        changed = true;
                        ShowNotification("Title(s) were removed from the system. Main menu has been updated.");
                    }
                    if(changed)
                    {
                        home::SaveConfig(global_home_menu);
                        main->ReloadItems();
                    }
                    break;
            }
            ui_communicate_flag = -1;
        }
        if(!dialog_queue.empty())
        {
            auto dlgtop = dialog_queue[0];
            dialog_queue.erase(dialog_queue.begin());
            CreateShowDialog(dlgtop.Title, dlgtop.Message, dlgtop.Options, dlgtop.UseLastOptionAsCancel, dlgtop.Icon);
        }
    }

    void AddDialogToDialogQueue(std::string Title, std::string Content, std::vector<std::string> Options, bool UseLastOptionAsCancel, std::string Icon)
    {
        home::HomeMutexLock lck(ui_threading_lock);
        dialog_queue.push_back({ Title, Content, Options, UseLastOptionAsCancel, Icon });
    }

    void SendFlagToUI(int Flag)
    {
        home::HomeMutexLock lck(ui_threading_lock);
        ui_communicate_flag = Flag;
    }

    void HomeApplication::LoadTitleLaunchScreen()
    {
        LoadLayout(launch);
    }

    void HomeApplication::LoadAllHbMenu()
    {
        LoadLayout(allHb);
    }

    void HomeApplication::ResetLoaded()
    {
        LoadLayout(main);
        main->ReloadItems();
    }

    void HomeApplication::ShowNotification(std::string Text)
    {
        notif->SetText(Text);
        StartOverlayWithTimeout(notif, 1500);
    }
}