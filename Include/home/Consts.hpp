
#pragma once
#include <string>

namespace home
{
    static const std::string BaseDir = "sdmc:/eqlipse";

    static const std::string ItemsMetaDir = BaseDir + "/items_meta";
    static const std::string ConfigPath = BaseDir + "/config.json";
    static const std::string ThemesDir = BaseDir + "/themes";

    static constexpr u32 HblHBLAMagic = 0x414C4248;
}