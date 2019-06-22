#include <home/Updates.hpp>
#include <libget/src/Get.hpp>
#include <home/Consts.hpp>

namespace home
{
    Updates::Updates(std::string LibgetDir)
    {
        if (HasAppstoreCapability())
        {
            get = new Get(LibgetDir.c_str(), "https://switch.apps.fortheusers.org");
        }
    }

    Package* Updates::GetIfTracked(std::string path)
    {
        if (!HasAppstoreCapability()) return nullptr;
        std::string packageID = ToPackageString(path);
        if (!packageID.empty())
        {
            Package *package = get->lookup(packageID);
            return package;
        }
        return nullptr;
    }

    std::string Updates::GetUpdate(std::string path)
    {
        Package *package = GetIfTracked(path);
        if (package && package->status == UPDATE)
            return package->version;
        return "";
    }

    std::string Updates::ToPackageString(std::string path)
    {
        auto endNode = path.rfind("/");
        if (endNode != std::string::npos)
        {
            std::string parent = path.substr(0, endNode-1); // Get path before nro
            auto parentNode = parent.rfind("/");

            if (parentNode != std::string::npos)
                parent = parent.substr(0, parentNode); // If not already simple enough, simplifies path even more to just parent folder
            return parent;
        }
        return "";
    }

    bool Updates::HasAppstoreCapability()
    {
        struct stat info;
        return stat( home::AppstoreDir.c_str(), &info ) == 0;
    }
}