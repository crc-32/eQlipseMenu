
#pragma once
#include <home/HomeConfig.hpp>
#include <ui/EntryListMenu.hpp>
#include <ui/TextButton.hpp>
#include <ui/SideImageButton.hpp>

namespace ui
{
    class MainLayout: public pu::Layout
    {
        public:
            MainLayout();
            void OnInput(u64 Down, u64 Up, u64 Held, bool Touch);
            
            void HandleOpenTitle(u64 Id, bool User, u32 Index);
            void HandleCloseTitle();
            void NotifyTitleTerminated();
            void ReloadItems();

            void entries_Menu_OnFocus(u32 Index);
            void entries_Menu_OnClick(u32 Index);

            void button_Hb_OnClick();
            void button_MenuEdit_OnClick();
            void button_Settings_OnClick();

            void button_Launch_OnClick();
            void button_Options_OnClick();
        private:
            EntryListMenu *entries_Menu;
            
            pu::element::Image *header_Image;
            pu::element::Image *footer_Image;


            pu::element::TextBlock *entry_Name;
            pu::element::TextBlock *entry_Author;
            pu::element::TextBlock *entry_Version;

            SideImageButton *button_Hb;

            SideImageButton *button_MenuEdit;
            SideImageButton *button_Settings;

            TextButton *button_Launch;
            TextButton *button_Options;
    };
}