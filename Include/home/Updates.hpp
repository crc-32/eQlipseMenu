#pragma once
#include <switch.h>
#include <libget/src/Get.hpp>
#include <string>

namespace home
{
    class Updates
    {
        Get* get;
        Package* GetIfTracked(std::string path);

        public:
        std::string GetUpdate(std::string path);
        std::string ToPackageString(std::string path);
        bool HasAppstoreCapability();
        Updates(std::string LibgetDir);
    };
}