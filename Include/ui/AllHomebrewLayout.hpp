
#pragma once
#include <home/HomeConfig.hpp>
#include <ui/EntryListMenu.hpp>
#include <ui/TextButton.hpp>
#include <ui/SideImageButton.hpp>

namespace ui
{
    class AllHomebrewLayout : public pu::Layout
    {
        public:
            AllHomebrewLayout();
            void OnInput(u64 Down, u64 Up, u64 Held, bool Touch);

            void entries_Menu_OnFocus(u32 Index);
            void entries_Menu_OnClick(u32 Index);
            void button_Hb_OnClick();
            void button_Select_OnClick();
        private:
            EntryListMenu *entries_Menu;
            
            pu::element::Image *footer_Image;

            SideImageButton *button_Hb; // Same button, icon and location than main menu, but this one opens hbmenu directly

            TextButton *button_Select; // Same button too, this time different

            pu::element::TextBlock *entry_Name;
            pu::element::TextBlock *entry_Author;
            pu::element::TextBlock *entry_Version;
    };
}