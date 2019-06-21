#include <home/HomeConfig.hpp>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <iomanip>
#include <sstream>
#include <home/System.hpp>

namespace home
{
    static bool ExistsFile(std::string Path)
    {
        std::ifstream ifs(Path);
        bool ex = ifs.good();
        ifs.close();
        return ex;
    }

    std::string HomeConfig::AbsolutePath(std::string Path)
    {
        if(Path.substr(0, 7) == "romfs:/") return Path;
        if(ExistsFile(Path)) return Path;
        return ThemesDir + "/" + CurrentTheme + "/" + Path;
    }

    std::vector<std::string> GetThemes()
    {
        std::vector<std::string> lyts;
        DIR *dp = opendir(ThemesDir.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                std::ifstream ifs(ThemesDir + "/" + std::string(dt->d_name) + "/theme.json");
                bool ok = ifs.good();
                ifs.close();
                if(ok) lyts.push_back(std::string(dt->d_name));
            }
        }
        closedir(dp);
        return lyts;
    }

    void CreateHomeConfig()
    {
        auto titles = GetAllTitles();
        JSON json = JSON::object();
        auto lyts = GetThemes();
        if(lyts.empty()) DieExecution("Unable to find any Themes to load.");
        json["Theme"] = lyts[0];
        for(u32 i = 0; i < titles.size(); i++)
        {
            std::stringstream strm;
            strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << titles[i].Id;
            json["entries"][i] =
            {
                { "type", "title" }, 
                { "id", strm.str() },
            };
        }
        std::ofstream ofs(ConfigPath);
        ofs << std::setw(4) << json << std::endl;
        ofs.close();
    }

    bool ExistsHomeConfig()
    {
        return ExistsFile(ConfigPath);
    }

    void EnsureHomeConfig()
    {
        if(!ExistsHomeConfig()) CreateHomeConfig();
    }

    static bool ProcessSingleEntry(std::vector<Title> &titles, JSON &input, MenuItem &out)
    {
        auto item = input;
        auto type = item["type"].get<std::string>();
        if(type == "title")
        {
            auto id = item["id"].get<std::string>();
            u64 tid = strtoull(id.c_str(), NULL, 16);
            for(u32 j = 0; j < titles.size(); j++)
            {
                if(titles[j].Id == tid)
                {
                    out.TitleData = titles[j];
                    out.Type = ItemType::Title;
                    out.Name = titles[j].Name;
                    out.Author = titles[j].Author;
                    out.Version = titles[j].Version;
                    out.MetaIconName = id;
                    return true;
                }
            }
        }
        else if(type == "homebrew")
        {
            auto path = item["path"].get<std::string>();
            FILE *nro = fopen(path.c_str(), "rb");
            if(nro)
            {
                out.Type = home::ItemType::Homebrew;
                out.Path = path;
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
                                    out.Version = std::string(nacp.version);
                                    NacpLanguageEntry *ent;
                                    nacpGetLanguageEntry(&nacp, &ent);
                                    if(ent != NULL)
                                    {
                                        if((ent->name[0] != '\0') && (ent->author[0] != '\0'))
                                        {
                                            out.Name = std::string(ent->name);
                                            out.Author = std::string(ent->author);
                                            fseek(nro, h.size + ash.icon.offset, SEEK_SET);
                                            u8 icon[ash.icon.size] = {0};
                                            if(fread(icon, ash.icon.size, 1, nro) == 1)
                                            {
                                                mkdir(home::ItemsMetaDir.c_str(), 777);

                                                std::stringstream strm;
                                                for(u32 i = 0; i < 0x10; i++) strm << std::hex << std::setfill('0') << std::uppercase << std::setw(2) << (int)h.build_id[i];

                                                out.MetaIconName = strm.str();

                                                FILE *iconf = fopen((home::ItemsMetaDir + "/" + out.MetaIconName + ".jpg").c_str(), "wb+");
                                                fwrite(icon, 1, ash.icon.size, iconf);
                                                fclose(iconf);
                                            }
                                            return true;
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
        return false;
    }

    HomeConfig ProcessHomeConfig()
    {
        HomeConfig m = {};
        std::ifstream ifs(ConfigPath);
        if(ifs.good())
        {
            auto titles = GetAllTitles();
            auto json = JSON::parse(ifs);
            m.CurrentTheme = json["Theme"].get<std::string>();
            for(u32 i = 0; i < json["entries"].size(); i++)
            {
                auto type = json["entries"][i]["type"].get<std::string>();
                if(type == "folder")
                {
                    auto folder = json["entries"][i];
                    MenuFolder f = {};
                    f.Name = folder.value("name", "");
                    if(f.Name.empty()) continue;
                    for(u32 i = 0; i < folder["entries"].size(); i++)
                    {
                        auto type = folder["entries"][i]["type"].get<std::string>();
                        if((type == "title") || (type == "homebrew"))
                        {
                            MenuItem itm = {};
                            if(ProcessSingleEntry(titles, folder["entries"][i], itm)) f.Items.push_back(itm);
                        }
                    }
                    m.Entries.push_back({ MenuEntryType::Folder, {}, f });
                }
                else if((type == "title") || (type == "homebrew"))
                {
                    MenuItem itm = {};
                    if(ProcessSingleEntry(titles, json["entries"][i], itm)) m.Entries.push_back({ MenuEntryType::RegularEntry, itm, {} });
                }
            }
        }
        ifs.close();
        return m;
    }

    void SaveConfig(HomeConfig Config)
    {
        auto json = JSON::object();
        json["Theme"] = Config.CurrentTheme;
        for(u32 i = 0; i < Config.Entries.size(); i++)
        {
            if(Config.Entries[i].Type == MenuEntryType::Folder)
            {
                json["entries"][i]["type"] = "folder";
                json["entries"][i]["name"] = Config.Entries[i].FolderItem.Name;
                for(u32 j = 0; j < Config.Entries[i].FolderItem.Items.size(); j++)
                {
                    std::string type = "title";
                    if(Config.Entries[i].FolderItem.Items[j].Type == ItemType::Homebrew) type = "homebrew";
                    json["entries"][i]["entries"][j] =
                    {
                        { "type", type },
                    };
                    if(Config.Entries[i].FolderItem.Items[j].Type == ItemType::Homebrew)
                    {
                        json["entries"][i]["entries"][j]["path"] = Config.Entries[i].FolderItem.Items[j].Path;
                    }
                    else
                    {
                        std::stringstream strm;
                        strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << Config.Entries[i].FolderItem.Items[j].TitleData.Id;
                        json["entries"][i]["entries"][j]["id"] = strm.str();
                    }
                }
            }
            else
            {
                std::string type = "title";
                if(Config.Entries[i].EntryItem.Type == ItemType::Homebrew) type = "homebrew";
                json["entries"][i] =
                {
                    { "type", type },
                };
                if(Config.Entries[i].EntryItem.Type == ItemType::Homebrew)
                {
                    json["entries"][i]["path"] = Config.Entries[i].EntryItem.Path;
                }
                else
                {
                    std::stringstream strm;
                    strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << Config.Entries[i].EntryItem.TitleData.Id;
                    json["entries"][i]["id"] = strm.str();
                }
            }
        }
        remove(ConfigPath.c_str());
        std::ofstream ofs(ConfigPath);
        ofs << std::setw(4) << json << std::endl;
        ofs.close();
    }

    bool ItemEquals(MenuItem A, MenuItem B)
    {
        if(A.Type == B.Type)
        {
            switch(A.Type)
            {
                case ItemType::Title:
                    if(A.TitleData.Id == B.TitleData.Id) return true;
                    break;
                case ItemType::Homebrew:
                    if(A.Path == B.Path) return true;
                    break;
            }
        }
        return false;
    }

    bool IsItemIn(HomeConfig Config, MenuItem Item)
    {
        for(u32 i = 0; i < Config.Entries.size(); i++)
        {
            if(Config.Entries[i].Type == MenuEntryType::Folder)
            {
                for(u32 j = 0; j < Config.Entries[i].FolderItem.Items.size(); j++)
                {
                    if(ItemEquals(Item, Config.Entries[i].FolderItem.Items[j])) return true;
                }
            }
            else
            {
                if(ItemEquals(Config.Entries[i].EntryItem, Item)) return true;
            }
        }
        return false;
    }

    bool IsFolderIn(HomeConfig Config, std::string Name)
    {
        for(u32 i = 0; i < Config.Entries.size(); i++)
        {
            if(Config.Entries[i].Type == MenuEntryType::Folder)
            {
                if(Config.Entries[i].FolderItem.Name == Name)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool CheckContentAdded(HomeConfig &Config)
    {
        bool added = false;
        auto ts = GetAllTitles();
        for(u32 i = 0; i < ts.size(); i++)
        {
            bool found = false;
            for(u32 j = 0; j < Config.Entries.size(); j++)
            {
                if(found) break;
                if(Config.Entries[j].Type == MenuEntryType::Folder)
                {
                    for(u32 k = 0; k < Config.Entries[j].FolderItem.Items.size(); k++)
                    {
                        if(Config.Entries[j].FolderItem.Items[k].Type == home::ItemType::Title)
                        {
                            if(Config.Entries[j].FolderItem.Items[k].TitleData.Id == ts[i].Id)
                            {
                                found = true;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    if(Config.Entries[j].EntryItem.Type == home::ItemType::Title)
                    {
                        if(Config.Entries[j].EntryItem.TitleData.Id == ts[i].Id)
                        {
                            found = true;
                            break;
                        }
                    }
                }
            }
            if(!found)
            {
                added = true;
                MenuItem itm;
                itm.Type = ItemType::Title;
                itm.Name = ts[i].Name;
                itm.Author = ts[i].Author;
                itm.Version = ts[i].Version;
                itm.TitleData = ts[i];
                std::stringstream strm;
                strm << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << ts[i].Id;
                itm.MetaIconName = strm.str();
                MenuEntry ent;
                ent.Type = MenuEntryType::RegularEntry;
                ent.EntryItem = itm;
                Config.Entries.push_back(ent);
            }
        }
        return added;
    }

    bool CheckContentRemoved(HomeConfig &Config)
    {
        bool rmvd = false;
        auto ts = GetAllTitles();
        for(u32 i = 0; i < Config.Entries.size(); i++)
        {
            if(Config.Entries[i].Type == MenuEntryType::Folder)
            {
                for(u32 j = 0; j < Config.Entries[i].FolderItem.Items.size(); j++)
                {
                    if(Config.Entries[i].FolderItem.Items[j].Type == ItemType::Title)
                    {
                        bool found = false;
                        for(u32 k = 0; k < ts.size(); k++)
                        {
                            if(ts[k].Id == Config.Entries[i].FolderItem.Items[j].TitleData.Id)
                            {
                                found = true;
                                break;
                            }
                        }
                        if(!found)
                        {
                            rmvd = true;
                            Config.Entries[i].FolderItem.Items.erase(Config.Entries[i].FolderItem.Items.begin() + i);
                        }
                    }
                }
            }
            else
            {
                if(Config.Entries[i].EntryItem.Type == ItemType::Title)
                {
                    bool found = false;
                    for(u32 j = 0; j < ts.size(); j++)
                    {
                        if(ts[j].Id == Config.Entries[i].EntryItem.TitleData.Id)
                        {
                            found = true;
                            break;
                        }
                    }
                    if(!found)
                    {
                        rmvd = true;
                        Config.Entries.erase(Config.Entries.begin() + i);
                    }
                }
            }
        }
        return rmvd;
    }

    Theme LoadTheme(std::string Name)
    {
        Theme lyt = {};
        lyt.Dir = Name;
        std::ifstream ifs(ThemesDir + "/" + Name + "/theme.json");
        if(ifs.good())
        {
            auto json = JSON::parse(ifs);
            lyt.Meta.Name = json["meta"]["name"].get<std::string>();
            lyt.Meta.Author = json["meta"]["author"].get<std::string>();
            lyt.Generic.Background = json["generic"].value("background", "");
            lyt.Generic.BGM = json["generic"].value("bgm", "");
            lyt.UI.HeaderImage = "romfs:/default/Header.png";
            lyt.UI.FooterTitleImage = "romfs:/default/Footer.title.png";
            lyt.UI.FooterFolderImage = "romfs:/default/Footer.folder.png";
            lyt.UI.FooterHbImage = "romfs:/default/Footer.hb.png";
            lyt.UI.HbIcon = "romfs:/default/Hb.png";
            lyt.UI.MenuEditIcon = "romfs:/default/MenuEdit.png";
            lyt.UI.SettingsIcon = "romfs:/default/Settings.png";
            lyt.UI.FolderIcon = "romfs:/default/Folder.png";
            if(json.count("ui"))
            {
                lyt.UI.HeaderImage = json["ui"].value("headerImage", "romfs:/default/Header.png");
                lyt.UI.FooterTitleImage = json["ui"].value("footerTitleImage", "romfs:/default/Footer.title.png");
                lyt.UI.FooterFolderImage = json["ui"].value("footerFolderImage", "romfs:/default/Footer.folder.png");
                lyt.UI.FooterHbImage = json["ui"].value("footerHbImage", "romfs:/default/Footer.hb.png");
                lyt.UI.HbIcon = json["ui"].value("hbIcon", "romfs:/default/Hb.png");
                lyt.UI.MenuEditIcon = json["ui"].value("menuEditIcon", "romfs:/default/MenuEdit.png");
                lyt.UI.SettingsIcon = json["ui"].value("settingsIcon", "romfs:/default/Settings.png");
                lyt.UI.FolderIcon = json["ui"].value("folderIcon", "romfs:/default/Folder.png");
            }
        }
        ifs.close();
        return lyt;
    }
}