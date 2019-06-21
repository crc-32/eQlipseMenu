
#pragma once
#include <ui/MainLayout.hpp>
#include <ui/AllHomebrewLayout.hpp>
#include <ui/BootLayout.hpp>
#include <ui/TitleLaunchLayout.hpp>

namespace ui
{
    struct DialogShowData
    {
        std::string Title;
        std::string Message;
        std::vector<std::string> Options;
        bool UseLastOptionAsCancel;
        std::string Icon;
    };

    class HomeApplication : public pu::Application
    {
        public:
            HomeApplication();
            void DialogQueueThread();
            void LoadTitleLaunchScreen();
            void LoadAllHbMenu();
            void ResetLoaded();
            void ShowNotification(std::string Text);
        private:
            MainLayout *main;
            AllHomebrewLayout *allHb;
            BootLayout *boot;
            TitleLaunchLayout *launch;
    };

    void AddDialogToDialogQueue(std::string Title, std::string Content, std::vector<std::string> Options, bool UseLastOptionAsCancel, std::string Icon = "");
    void SendFlagToUI(int Flag);
}