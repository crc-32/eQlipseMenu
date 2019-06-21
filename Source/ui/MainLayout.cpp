#include <ui/HomeApplication.hpp>
#include <home/System.hpp>
#include <home/Messages.hpp>
#include <dirent.h>

extern ApplicationHolder global_hold_application;
extern AppletHolder global_hold_applet;
extern home::HomeConfig global_home_menu;
extern home::Theme global_theme;
extern ui::HomeApplication *app_instance;
extern pu::music::Music bgm;

namespace ui
{
    static int folderidx = -1;
    static int opened_layeridx = -2;
    static int opened_itemidx = -1;
    static u32 root_lastidx = 0;

    MainLayout::MainLayout()
    {
        if(!global_theme.Generic.Background.empty()) SetBackgroundImage(global_home_menu.AbsolutePath(global_theme.Generic.Background));

        header_Image = new pu::element::Image(0, 0, global_home_menu.AbsolutePath(global_theme.UI.HeaderImage));
        Add(header_Image);

        footer_Image = new pu::element::Image(0, 585, global_home_menu.AbsolutePath(global_theme.UI.FooterTitleImage));
        Add(footer_Image);

        entries_Menu = new EntryListMenu();
        entries_Menu->SetOnEntryFocus(std::bind(&MainLayout::entries_Menu_OnFocus, this, std::placeholders::_1));
        entries_Menu->SetOnEntrySelect(std::bind(&MainLayout::entries_Menu_OnClick, this, std::placeholders::_1));
        for(u32 i = 0; i < global_home_menu.Entries.size(); i++)
        {
            if(global_home_menu.Entries[i].Type == home::MenuEntryType::Folder) entries_Menu->AddEntryIcon(global_home_menu.AbsolutePath(global_theme.UI.FolderIcon));
            else entries_Menu->AddEntryIcon(home::ItemsMetaDir + "/" + global_home_menu.Entries[i].EntryItem.MetaIconName + ".jpg");
        }
        Add(entries_Menu);

        entry_Name = new pu::element::TextBlock(40, 610, "", 30);
        entry_Author = new pu::element::TextBlock(45, 650, "", 20);
        entry_Version = new pu::element::TextBlock(45, 675, "", 20);
        entry_Name->SetColor({ 225, 225, 225, 255 });
        entry_Author->SetColor({ 225, 225, 225, 255 });
        entry_Version->SetColor({ 225, 225, 225, 255 });
        Add(entry_Name);
        Add(entry_Author);
        Add(entry_Version);

        button_Hb = new SideImageButton(Side::Left, 150, global_home_menu.AbsolutePath(global_theme.UI.HbIcon), 50, { 51, 153, 255, 255 }, 125, 100, 50, 25);
        button_Hb->SetOnClick(std::bind(&MainLayout::button_Hb_OnClick, this));
        button_Hb->SetPressKey(KEY_L);
        Add(button_Hb);

        button_MenuEdit = new SideImageButton(Side::Right, 215, global_home_menu.AbsolutePath(global_theme.UI.MenuEditIcon), 40, { 255, 215, 0, 255 }, 80, 70, 15, 25);
        button_MenuEdit->SetOnClick(std::bind(&MainLayout::button_MenuEdit_OnClick, this));
        button_MenuEdit->SetPressKey(KEY_R);
        Add(button_MenuEdit);

        button_Settings = new SideImageButton(Side::Right, 100, global_home_menu.AbsolutePath(global_theme.UI.SettingsIcon), 60, { 115, 115, 115, 255 }, 110, 80, 30, 25);
        button_Settings->SetOnClick(std::bind(&MainLayout::button_Settings_OnClick, this));
        button_Settings->SetPressKey(KEY_ZR);
        Add(button_Settings);

        button_Launch = new TextButton(1075, 630, "Launch", 25, { 90, 90, 240, 255 }, { 225, 225, 225, 255 });
        button_Launch->SetOnClick(std::bind(&MainLayout::button_Launch_OnClick, this));
        button_Launch->SetPressKey(KEY_A);
        Add(button_Launch);

        button_Options = new TextButton(885, 630, "Options", 25, { 0, 204, 204, 255 }, { 225, 225, 225, 255 });
        button_Options->SetOnClick(std::bind(&MainLayout::button_Options_OnClick, this));
        button_Options->SetPressKey(KEY_X | KEY_Y);
        Add(button_Options);

        entries_Menu_OnFocus(0);
        
        SetOnInput(std::bind(&MainLayout::OnInput, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }

    void MainLayout::OnInput(u64 Down, u64 Up, u64 Held, bool Touch)
    {
        if(Down & KEY_B)
        {
            if(folderidx != -1)
            {
                folderidx = -1;
                app_instance->FadeOut();
                ReloadItems();
                app_instance->FadeIn();
            }
        }
    }

    void MainLayout::NotifyTitleTerminated()
    {
        opened_layeridx = -2;
        entries_Menu->SetOpenedItemIndex(-1);
    }

    void MainLayout::ReloadItems()
    {
        if(folderidx == -1)
        {
            entries_Menu->ClearEntries();
            for(u32 i = 0; i < global_home_menu.Entries.size(); i++)
            {
                if(global_home_menu.Entries[i].Type == home::MenuEntryType::Folder) entries_Menu->AddEntryIcon(global_home_menu.AbsolutePath(global_theme.UI.FolderIcon));
                else entries_Menu->AddEntryIcon(home::ItemsMetaDir + "/" + global_home_menu.Entries[i].EntryItem.MetaIconName + ".jpg");
            }
            entries_Menu->SetFocusedItemIndex(root_lastidx);
            if(opened_layeridx == folderidx) entries_Menu->SetOpenedItemIndex(opened_itemidx);
            else entries_Menu->SetOpenedItemIndex(folderidx);
            entries_Menu_OnFocus(root_lastidx);
        }
        else
        {
            entries_Menu->ClearEntries();
            for(u32 i = 0; i < global_home_menu.Entries[folderidx].FolderItem.Items.size(); i++)
            {
                entries_Menu->AddEntryIcon(home::ItemsMetaDir + "/" + global_home_menu.Entries[folderidx].FolderItem.Items[i].MetaIconName + ".jpg");
            }
            if(opened_layeridx == folderidx) entries_Menu->SetOpenedItemIndex(opened_itemidx);
            else entries_Menu->SetOpenedItemIndex(-1);
            entries_Menu_OnFocus(0);
        }
    }

    void MainLayout::entries_Menu_OnFocus(u32 Index)
    {
        button_Launch->SetVisible(true);
        if((global_home_menu.Entries[Index].Type == home::MenuEntryType::Folder) && (folderidx == -1))
        {
            entry_Version->SetVisible(false);
            entry_Name->SetText(global_home_menu.Entries[Index].FolderItem.Name);
            entry_Author->SetText(std::to_string(global_home_menu.Entries[Index].FolderItem.Items.size()) + " entries");
            button_Launch->SetText("Open");
            if(global_home_menu.Entries[Index].FolderItem.Items.empty()) button_Launch->SetVisible(false);
            footer_Image->SetImage(global_home_menu.AbsolutePath(global_theme.UI.FooterFolderImage));
        }
        else
        {
            home::MenuItem itm = global_home_menu.Entries[Index].EntryItem;
            if(folderidx != -1)
            {
                itm = global_home_menu.Entries[folderidx].FolderItem.Items[Index]; 
                footer_Image->SetImage(global_home_menu.AbsolutePath(global_theme.UI.FooterFolderImage));
            }
            else
            {
                if(itm.Type == home::ItemType::Homebrew) footer_Image->SetImage(global_home_menu.AbsolutePath(global_theme.UI.FooterHbImage));
                else footer_Image->SetImage(global_home_menu.AbsolutePath(global_theme.UI.FooterTitleImage));
            }
            entry_Version->SetVisible(true);
            button_Launch->SetText("Launch");
            entry_Name->SetText(itm.Name);
            entry_Author->SetText(itm.Author);
            entry_Version->SetText(itm.Version);
        }
    }

    void MainLayout::HandleOpenTitle(u64 Id, bool User, u32 Index)
    {
        Result rc = 0;
        if(Id == 0x01008BB00013C000)
        {
            home::HandleLayeredFs(home::BinDir + "/SystemApplicationHbTarget", Id);
            rc = appletApplicationHolderCreateForSystem(&global_hold_application, Id);
        }
        else rc = appletApplicationHolderCreate(&global_hold_application, Id);
        if(rc == 0)
        {
            home::HbTargetArgs args = {};
            args.Magic = home::HbTargetMagic;
            strcpy(args.Entry, "sdmc:/switch/Goldleaf.nro");
            AppletStorage st;
            appletCreateStorage(&st, sizeof(home::HbTargetArgs));
            appletStorageWrite(&st, 0, &args, sizeof(home::HbTargetArgs));
            appletApplicationHolderPushLaunchParameter(&global_hold_application, AppletLaunchParameterKind_Application, &st);
            appletStorageClose(&st);
            rc = appletApplicationHolderPrepareForLaunch(&global_hold_application, User);//itm.TitleData.NeedsUser);
            if(rc == 0)
            {
                app_instance->FadeOut();
                app_instance->LoadTitleLaunchScreen();
                app_instance->FadeIn();
                rc = appletApplicationHolderLaunch(&global_hold_application);
                if(rc == 0)
                {
                    pu::music::FadeOut(500);
                    opened_layeridx = folderidx;
                    opened_itemidx = Index;
                    entries_Menu->SetOpenedItemIndex(Index);
                }
            }
        }
        if((rc != 0) && (rc != 0x4680))
        {
            app_instance->ResetLoaded();
            app_instance->CreateShowDialog("Title launching", "An error ocurred attempting to launch the title.", { "Ok" }, true);
        }
    }

    void MainLayout::HandleCloseTitle()
    {
        home::UnhandleLayeredFs(0x01008BB00013C000);
        appletApplicationHolderTerminate(&global_hold_application);
        appletApplicationHolderClose(&global_hold_application);
        NotifyTitleTerminated();
    }

    void MainLayout::entries_Menu_OnClick(u32 Index)
    {
        if((global_home_menu.Entries[Index].Type == home::MenuEntryType::Folder) && (folderidx == -1))
        {
            folderidx = Index;
            root_lastidx = entries_Menu->GetFocusedItemIndex();
            app_instance->FadeOut();
            ReloadItems();
            app_instance->FadeIn();
        }
        else
        {
            auto itm = global_home_menu.Entries[Index].EntryItem;
            if(folderidx != -1) itm = global_home_menu.Entries[folderidx].FolderItem.Items[Index]; 
            if(itm.Type == home::ItemType::Homebrew)
            {
                pu::music::FadeOut(500);
                app_instance->FadeOut();
                home::TargetHbmenu(itm.Path.c_str());
                pu::music::PlayWithFadeIn(bgm, -1, 1500);
                app_instance->FadeIn();
            }
            else
            {
                auto title = itm.TitleData;
                if(appletApplicationHolderIsInProcess(&global_hold_application))
                {
                    if((opened_layeridx == folderidx) && ((int)Index == opened_itemidx))
                    {
                        app_instance->LoadTitleLaunchScreen();
                        svcSleepThread(250000000L);
                        appletApplicationHolderRequestForeground(&global_hold_application);
                        return;
                    }
                    int sopt = app_instance->CreateShowDialog("Title already opened", "Would you like to close the currently open title?", {"Yes", "No"}, true);
                    if(sopt == 0)
                    {
                        HandleCloseTitle();
                    }
                    else return;
                }
                HandleOpenTitle(itm.TitleData.Id, itm.TitleData.NeedsUser, Index);
            }
        }
    }

    void MainLayout::button_Hb_OnClick()
    {
        app_instance->FadeOut();
        app_instance->LoadAllHbMenu();
        app_instance->FadeIn();
    }

    void MainLayout::button_MenuEdit_OnClick()
    {
        if(folderidx != -1)
        {
            app_instance->CreateShowDialog("Folder creation", "Folders can only be created on the main menu.\n(subfolders aren't supported)", {"Ok"}, true);
            return;
        }
        char tmpout[FS_MAX_PATH] = { };
        SwkbdConfig kbd;
        Result rc = swkbdCreate(&kbd, 0);
        if(rc == 0)
        {
            swkbdConfigMakePresetDefault(&kbd);
            swkbdConfigSetGuideText(&kbd, "Folder name to create");
            rc = swkbdShow(&kbd, tmpout, sizeof(tmpout));
            if(rc == 0)
            {
                home::MenuEntry ent;
                ent.Type = home::MenuEntryType::Folder;
                ent.FolderItem.Name = std::string(tmpout);
                if(home::IsFolderIn(global_home_menu, ent.FolderItem.Name))
                {
                    app_instance->CreateShowDialog("Create folder", "Another folder with the same name already exists.", {"Ok"}, true);
                    return;
                }
                global_home_menu.Entries.push_back(ent);
                home::SaveConfig(global_home_menu);
                app_instance->FadeOut();
                ReloadItems();
                app_instance->FadeIn();
            }
            swkbdClose(&kbd);
        }
    }

    void MainLayout::button_Settings_OnClick()
    {
        
    }

    void MainLayout::button_Launch_OnClick()
    {
        entries_Menu_OnClick(entries_Menu->GetFocusedItemIndex());
    }

    void MainLayout::button_Options_OnClick()
    {
        u32 idx = entries_Menu->GetFocusedItemIndex();
        if((global_home_menu.Entries[idx].Type == home::MenuEntryType::Folder) && (folderidx == -1))
        {
            auto folder = global_home_menu.Entries[idx].FolderItem;
            int sopt = app_instance->CreateShowDialog(folder.Name, "What would you like to do with this folder?", { "Open", "Delete", "Cancel" }, true);
            if(sopt == 0)
            {
                folderidx = idx;
                root_lastidx = entries_Menu->GetFocusedItemIndex();
                app_instance->FadeOut();
                ReloadItems();
                app_instance->FadeIn();
            }
            else if(sopt == 1)
            {
                int sopt2 = app_instance->CreateShowDialog("Delete folder", "All folder's sub-entries will be moved to the main menu.\nDo you want to continue?", { "Yes", "Cancel" }, true);
                if(sopt2 == 0)
                {
                    for(u32 i = 0; i < folder.Items.size(); i++)
                    {
                        home::MenuEntry ent;
                        ent.Type = home::MenuEntryType::RegularEntry;
                        ent.EntryItem = folder.Items[i];
                        global_home_menu.Entries.push_back(ent);
                    }
                    global_home_menu.Entries.erase(global_home_menu.Entries.begin() + idx);
                    home::SaveConfig(global_home_menu);
                    app_instance->FadeOut();
                    ReloadItems();
                    app_instance->FadeIn();
                }
            }
        }
        else
        {
            auto itm = global_home_menu.Entries[idx].EntryItem;
            if(folderidx != -1) itm = global_home_menu.Entries[folderidx].FolderItem.Items[idx]; 

            if(itm.Type == home::ItemType::Homebrew)
            {
                int sopt = app_instance->CreateShowDialog(itm.Name, "What would you like to do with this homebrew app?", { "Launch", "Move into folder", "Remove", "Cancel" }, true);
                if(sopt == 0)
                {
                    folderidx = idx;
                    root_lastidx = entries_Menu->GetFocusedItemIndex();
                    app_instance->FadeOut();
                    ReloadItems();
                    app_instance->FadeIn();
                }
                else if(sopt == 1)
                {
                    std::vector<u32> folderidxs;
                    std::vector<std::string> folders;
                    for(u32 i = 0; i < global_home_menu.Entries.size(); i++)
                    {
                        if(global_home_menu.Entries[i].Type == home::MenuEntryType::Folder)
                        {
                            folderidxs.push_back(i);
                            folders.push_back(global_home_menu.Entries[i].FolderItem.Name);
                        }
                    }
                    if(folderidxs.empty())
                    {
                        app_instance->CreateShowDialog("Moving homebrew app", "There are no folders in main menu.\nCreate one before moving homebrew apps.", { "Ok" }, true);
                        return;
                    }
                    folders.push_back("Cancel");
                    int sopt = app_instance->CreateShowDialog("Moving homebrew app", "Which folder would you like to move this homebrew app to?", folders, true);
                    if(sopt >= 0)
                    {
                        u32 fidx = folderidxs[sopt];
                        opened_itemidx = global_home_menu.Entries[fidx].FolderItem.Items.size();
                        opened_layeridx = fidx;
                        global_home_menu.Entries[fidx].FolderItem.Items.push_back(itm);
                        global_home_menu.Entries.erase(global_home_menu.Entries.begin() + idx);
                        home::SaveConfig(global_home_menu);
                        app_instance->FadeOut();
                        ReloadItems();
                        app_instance->FadeIn();
                    }
                }
                else if(sopt == 2)
                {
                    int sopt2 = app_instance->CreateShowDialog("Remove homebrew", "This homebrew will be removed. (just from this menu)\nDo you want to continue?", { "Yes", "Cancel" }, true);
                    if(sopt2 == 0)
                    {
                        global_home_menu.Entries.erase(global_home_menu.Entries.begin() + idx);
                        home::SaveConfig(global_home_menu);
                        app_instance->FadeOut();
                        ReloadItems();
                        app_instance->FadeIn();
                    }
                }
            }
            else
            {
                if((opened_layeridx == folderidx) && ((int)idx == opened_itemidx))
                {
                    int sopt = app_instance->CreateShowDialog(itm.Name, "What would you like to do with this title?", { "Return to it", "Close", "Move into folder", "Cancel" }, true);
                    if(sopt == 0)
                    {
                        app_instance->LoadTitleLaunchScreen();
                        svcSleepThread(250000000L);
                        appletApplicationHolderRequestForeground(&global_hold_application);
                    }
                    else if(sopt == 1)
                    {
                        HandleCloseTitle();
                    }
                    else if(sopt == 2)
                    {
                        std::vector<u32> folderidxs;
                        std::vector<std::string> folders;
                        for(u32 i = 0; i < global_home_menu.Entries.size(); i++)
                        {
                            if(global_home_menu.Entries[i].Type == home::MenuEntryType::Folder)
                            {
                                folderidxs.push_back(i);
                                folders.push_back(global_home_menu.Entries[i].FolderItem.Name);
                            }
                        }
                        if(folderidxs.empty())
                        {
                            app_instance->CreateShowDialog("Moving title", "There are no folders in main menu.\nCreate one before moving titles.", { "Ok" }, true);
                            return;
                        }
                        folders.push_back("Cancel");
                        int sopt = app_instance->CreateShowDialog("Moving title", "Which folder would you like to move this title to?", folders, true);
                        if(sopt >= 0)
                        {
                            u32 fidx = folderidxs[sopt];
                            opened_itemidx = global_home_menu.Entries[fidx].FolderItem.Items.size();
                            opened_layeridx = fidx;
                            global_home_menu.Entries[fidx].FolderItem.Items.push_back(itm);
                            global_home_menu.Entries.erase(global_home_menu.Entries.begin() + idx);
                            home::SaveConfig(global_home_menu);
                            app_instance->FadeOut();
                            ReloadItems();
                            app_instance->FadeIn();
                        }
                    }
                }
                else
                {
                    int sopt = app_instance->CreateShowDialog(itm.Name, "What would you like to do with this title?", { "Launch", "Move into folder", "Cancel" }, true);
                    if(sopt == 0)
                    {
                        HandleOpenTitle(0x01008BB00013C000, true, idx);
                    }
                    else if(sopt == 1)
                    {
                        std::vector<u32> folderidxs;
                        std::vector<std::string> folders;
                        for(u32 i = 0; i < global_home_menu.Entries.size(); i++)
                        {
                            if(global_home_menu.Entries[i].Type == home::MenuEntryType::Folder)
                            {
                                folderidxs.push_back(i);
                                folders.push_back(global_home_menu.Entries[i].FolderItem.Name);
                            }
                        }
                        if(folderidxs.empty())
                        {
                            app_instance->CreateShowDialog("Moving title", "There are no folders in main menu.\nCreate one before moving titles.", { "Ok" }, true);
                            return;
                        }
                        folders.push_back("Cancel");
                        int sopt = app_instance->CreateShowDialog("Moving title", "Which folder would you like to move this title to?", folders, true);
                        if(sopt >= 0)
                        {
                            u32 fidx = folderidxs[sopt];
                            opened_itemidx = global_home_menu.Entries[fidx].FolderItem.Items.size();
                            opened_layeridx = fidx;
                            global_home_menu.Entries[fidx].FolderItem.Items.push_back(itm);
                            global_home_menu.Entries.erase(global_home_menu.Entries.begin() + idx);
                            home::SaveConfig(global_home_menu);
                            app_instance->FadeOut();
                            ReloadItems();
                            app_instance->FadeIn();
                        }
                    }
                }
            }
        }
    }
}