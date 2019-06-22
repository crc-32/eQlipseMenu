
#pragma once
#include <json.hpp>
#include <switch.h>
#include <home/Consts.hpp>
#include <home/System.hpp>

namespace home
{
    using JSON = nlohmann::json;
    
    enum class ItemType
    {
        Title,
        Homebrew
    };

    enum class MenuEntryType
    {
        RegularEntry,
        Folder
    };

    struct ThemeMeta
    {
        std::string Name;
        std::string Author;
    };

    struct ThemeGeneric
    {
        std::string Background;
        std::string BGM;
    };

    struct UIConfig
    {
        std::string HeaderImage;
        std::string FooterTitleImage;
        std::string FooterFolderImage;
        std::string FooterHbImage;
        std::string HbIcon;
        std::string MenuEditIcon;
        std::string SettingsIcon;
        std::string FolderIcon;
        std::string UpdateIndicator;
    };

    struct Theme
    {
        std::string Dir;
        ThemeMeta Meta;
        ThemeGeneric Generic;
        UIConfig UI;
    };

    struct MenuItem
    {
        std::string Name;
        std::string Author;
        std::string Version;
        ItemType Type;
        Title TitleData;
        std::string Path;
        std::string MetaIconName;
        std::string HbUpdateVersion;
    };

    struct MenuFolder
    {
        std::string Name;
        std::vector<MenuItem> Items;
    };

    struct MenuEntry
    {
        MenuEntryType Type;
        MenuItem EntryItem;
        MenuFolder FolderItem;
    };

    struct HomeConfig
    {
        std::string CurrentTheme;
        std::vector<MenuEntry> Entries;
        std::string AbsolutePath(std::string LytRelative);
    };

    std::vector<std::string> GetThemes();

    void CreateHomeConfig();
    bool ExistsHomeConfig();
    void EnsureHomeConfig();
    HomeConfig ProcessHomeConfig();
    void SaveConfig(HomeConfig Config);
    bool ItemEquals(MenuItem A, MenuItem B);
    bool IsItemIn(HomeConfig Config, MenuItem Item);
    bool IsFolderIn(HomeConfig Config, std::string Name);
    bool CheckContentAdded(HomeConfig &Config);
    bool CheckContentRemoved(HomeConfig &Config);

    Theme LoadTheme(std::string Name);
}