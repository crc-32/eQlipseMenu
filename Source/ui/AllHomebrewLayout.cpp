#include <ui/AllHomebrewLayout.hpp>
#include <ui/HomeApplication.hpp>
#include <dirent.h>
#include <sstream>
#include <iomanip>

extern home::HomeConfig global_home_menu;
extern home::Theme global_theme;
extern ui::HomeApplication *app_instance;
extern pu::music::Music bgm;

namespace ui
{
    static std::vector<home::MenuItem> hbs;

    static bool ExistsFile(std::string Path)
    {
        std::ifstream ifs(Path);
        bool ex = ifs.good();
        ifs.close();
        return ex;
    }

    static std::vector<home::MenuItem> QueryAllHomebrew(std::string Path = "sdmc:/switch")
    {
        std::vector<home::MenuItem> itms;
        DIR *dp = opendir(Path.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                struct stat st;
                auto fname = std::string(dt->d_name);
                auto full = Path + "/" + fname;
                stat(full.c_str(), &st);
                if(st.st_mode & S_IFREG)
                {
                    if(fname.substr(fname.find_last_of(".") + 1) == "nro")
                    {
                        home::MenuItem itm = {};
                        FILE *nro = fopen(full.c_str(), "rb");
                        if(nro)
                        {
                            itm.Type = home::ItemType::Homebrew;
                            itm.Path = full;
                            NroHeader h = {};
                            fseek(nro, sizeof(NroStart), SEEK_SET);
                            if(fread(&h, sizeof(NroHeader), 1, nro) == 1)
                            {
                                if(h.magic == NROHEADER_MAGIC)
                                {
                                    fseek(nro, h.size, SEEK_SET);
                                    NroAssetHeader ash = {};
                                    if(fread(&ash, sizeof(NroAssetHeader), 1, nro) == 1)
                                    {
                                        fseek(nro, h.size + ash.nacp.offset, SEEK_SET);
                                        NacpStruct nacp = {};
                                        if(fread(&nacp, ash.nacp.size, 1, nro) == 1)
                                        {
                                            if(nacp.version[0] != '\0')
                                            {
                                                itm.Version = std::string(nacp.version);
                                                NacpLanguageEntry *ent;
                                                nacpGetLanguageEntry(&nacp, &ent);
                                                if(ent != NULL)
                                                {
                                                    if((ent->name[0] != '\0') && (ent->author[0] != '\0'))
                                                    {
                                                        itm.Name = std::string(ent->name);
                                                        itm.Author = std::string(ent->author);
                                                        fseek(nro, h.size + ash.icon.offset, SEEK_SET);
                                                        u8 icon[ash.icon.size] = {0};
                                                        if(fread(icon, ash.icon.size, 1, nro) == 1)
                                                        {
                                                            mkdir(home::ItemsMetaDir.c_str(), 777);

                                                            std::stringstream strm;
                                                            for(u32 i = 0; i < 0x10; i++) strm << std::hex << std::setfill('0') << std::uppercase << std::setw(2) << (int)h.build_id[i];

                                                            itm.MetaIconName = strm.str();

                                                            auto fullicon = home::ItemsMetaDir + "/" + itm.MetaIconName + ".jpg";
                                                            if(!ExistsFile(fullicon))
                                                            {
                                                                FILE *iconf = fopen(fullicon.c_str(), "wb+");
                                                                fwrite(icon, 1, ash.icon.size, iconf);
                                                                fclose(iconf);
                                                            }
                                                        }
                                                        itms.push_back(itm);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            fclose(nro);
                        }
                    }
                }
                else if(st.st_mode & S_IFDIR)
                {
                    auto data = QueryAllHomebrew(full);
                    if(!data.empty()) itms.insert(itms.begin(), data.begin(), data.end());
                }
            }
            closedir(dp);
        }
        return itms;
    }

    AllHomebrewLayout::AllHomebrewLayout()
    {
        if(!global_theme.Generic.Background.empty()) SetBackgroundImage(global_home_menu.AbsolutePath(global_theme.Generic.Background));
        hbs = QueryAllHomebrew();

        entries_Menu = new EntryListMenu();
        entries_Menu->SetOnEntryFocus(std::bind(&AllHomebrewLayout::entries_Menu_OnFocus, this, std::placeholders::_1));
        entries_Menu->SetOnEntrySelect(std::bind(&AllHomebrewLayout::entries_Menu_OnClick, this, std::placeholders::_1));
        for(u32 i = 0; i < hbs.size(); i++) entries_Menu->AddEntryIcon(home::ItemsMetaDir + "/" + hbs[i].MetaIconName + ".jpg");
        Add(entries_Menu);

        footer_Image = new pu::element::Image(0, 585, global_home_menu.AbsolutePath(global_theme.UI.FooterHbImage));
        Add(footer_Image);

        entry_Name = new pu::element::TextBlock(40, 610, "", 30);
        entry_Author = new pu::element::TextBlock(45, 650, "", 20);
        entry_Version = new pu::element::TextBlock(45, 675, "", 20);
        entry_Name->SetColor({ 225, 225, 225, 255 });
        entry_Author->SetColor({ 225, 225, 225, 255 });
        entry_Version->SetColor({ 225, 225, 225, 255 });
        entry_Name->SetHorizontalAlign(pu::element::HorizontalAlign::Center);
        entry_Author->SetHorizontalAlign(pu::element::HorizontalAlign::Center);
        entry_Version->SetHorizontalAlign(pu::element::HorizontalAlign::Center);
        Add(entry_Name);
        Add(entry_Author);
        Add(entry_Version);

        button_Hb = new SideImageButton(Side::Left, 150, global_home_menu.AbsolutePath(global_theme.UI.HbIcon), 50, { 51, 153, 255, 255 }, 125, 100, 50, 25);
        button_Hb->SetOnClick(std::bind(&AllHomebrewLayout::button_Hb_OnClick, this));
        button_Hb->SetPressKey(KEY_L);
        Add(button_Hb);

        button_Select = new TextButton(1075, 630, "Select", 25, { 90, 90, 240, 255 }, { 225, 225, 225, 255 });
        button_Select->SetOnClick(std::bind(&AllHomebrewLayout::button_Select_OnClick, this));
        button_Select->SetPressKey(KEY_A);
        Add(button_Select);

        entries_Menu_OnFocus(0);
        SetOnInput(std::bind(&AllHomebrewLayout::OnInput, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }

    void AllHomebrewLayout::entries_Menu_OnFocus(u32 Index)
    {
        entry_Name->SetText(hbs[Index].Name);
        entry_Author->SetText(hbs[Index].Author);
        entry_Version->SetText(hbs[Index].Version);
    }

    void AllHomebrewLayout::entries_Menu_OnClick(u32 Index)
    {
        int sopt = app_instance->CreateShowDialog("Homebrew menu", "What would you like to do with the selected homebrew application?", { "Launch", "Add to main menu", "Cancel" }, true);
        if(sopt == 0)
        {
            pu::music::FadeOut(500);
            app_instance->FadeOut();
            home::TargetHbmenu(hbs[Index].Path);
            pu::music::PlayWithFadeIn(bgm, -1, 1500);
            app_instance->FadeIn();
        }
        else if(sopt == 1)
        {
            home::MenuEntry entry;
            entry.Type = home::MenuEntryType::RegularEntry;
            entry.EntryItem = hbs[Index];
            if(home::IsItemIn(global_home_menu, hbs[Index]))
            {
                app_instance->CreateShowDialog("Homebrew", "This homebrew is already in the main menu.", {"Ok"}, true);
                return;
            }
            global_home_menu.Entries.push_back(entry);
            home::SaveConfig(global_home_menu);
        }
    }

    void AllHomebrewLayout::button_Hb_OnClick()
    {
        pu::music::FadeOut(500);
        app_instance->FadeOut();
        home::TargetHbmenu("sdmc:/hbmenu.nro");
        pu::music::PlayWithFadeIn(bgm, -1, 1500);
        app_instance->FadeIn();
    }
    
    void AllHomebrewLayout::button_Select_OnClick()
    {
        entries_Menu_OnClick(entries_Menu->GetFocusedItemIndex());
    }

    void AllHomebrewLayout::OnInput(u64 Down, u64 Up, u64 Held, bool Touch)
    {
        if(Down & KEY_B)
        {
            app_instance->FadeOut();
            app_instance->ResetLoaded();
            app_instance->FadeIn();
        }
    }
}