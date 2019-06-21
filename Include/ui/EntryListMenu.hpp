
#pragma once
#include <pu/Plutonium>

namespace ui
{
    class EntryListMenu : public pu::element::Element
    {
        public:
            EntryListMenu();
            s32 GetX();
            s32 GetY();
            s32 GetWidth();
            s32 GetHeight();
            void OnInput(u64 Down, u64 Up, u64 Held, bool Touch, bool Focus);
            void AddEntryIcon(std::string Path);
            void RemoveEntrt(u32 Index);
            void ClearEntries();
            void SetOnEntryFocus(std::function<void(u32)> OnFocus);
            void SetOnEntrySelect(std::function<void(u32)> OnSelect);
            void OnRender(pu::render::Renderer *Drawer);
            void SetOpenedItemIndex(int Index);
            int GetOpenedItemIndex();
            void SetFocusedItemIndex(u32 Index);
            u32 GetFocusedItemIndex();
        private:
            int openedidx;
            std::vector<std::string> entries;
            std::vector<pu::render::NativeTexture> entryicons;
            u32 mainidx;
            std::function<void(u32)> focuscb;
            std::function<void(u32)> selcb;
    };
}