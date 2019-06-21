
#pragma once
#include <switch.h>
#include <string>
#include <vector>
#include <home/Consts.hpp>

namespace home
{
    struct Title
    {
        u64 Id;
        std::string Name;
        std::string Author;
        std::string Version;
        bool NeedsUser;
        FsStorageId Storage;
        u64 TotalSize;
    };

    struct HbTargetArgs
    {
        u32 Magic;
        char Entry[2048];
        char Argv[2048];
    };

    static constexpr u32 HbTargetMagic = 0x47544248;

    void DieExecution(std::string Msg);
    void Initialize();
    void Exit();

    bool RunningAtmosphere();
    bool RunningReiNX();
    bool RunningSXOS();
    std::string GetBaseCfwDir();
    void TargetHbmenu(std::string Path);
    
    std::vector<Title> GetTitlesForStorage(FsStorageId Id);
    std::vector<Title> GetAllTitles();
}