#include <ipc/DialogService.hpp>
#include <ui/HomeApplication.hpp>
#include <sstream>

namespace ipc
{
    Result DialogService::AddDialogToQueue(InBuffer<char> title, InBuffer<char> contents, InBuffer<char> button)
    {
        ui::AddDialogToDialogQueue(std::string(title.buffer), std::string(contents.buffer), { std::string(button.buffer) }, false);
        return 0;
    }
}