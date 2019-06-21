#include <ui/EntryListMenu.hpp>

namespace ui
{
    EntryListMenu::EntryListMenu()
    {
        focuscb = [](u32){};
        selcb = [](u32){};
        mainidx = 0;
        openedidx = -1;
    }

    s32 EntryListMenu::GetX()
    {
        return 0;
    }

    s32 EntryListMenu::GetY()
    {
        return 304;
    }

    s32 EntryListMenu::GetWidth()
    {
        return 1280;
    }

    s32 EntryListMenu::GetHeight()
    {
        return 416;
    }

    void EntryListMenu::OnInput(u64 Down, u64 Up, u64 Held, bool Touch, bool Focus)
    {
        if(Down & KEY_LEFT)
        {
            if(mainidx > 0)
            {
                mainidx--;
                if(entryicons.size() == 7)
                {
                    pu::render::DeleteTexture(entryicons.back());
                    entryicons.pop_back();
                }
                focuscb(mainidx);
            }
        }
        else if(Down & KEY_RIGHT)
        {
            if(entryicons.size() > (mainidx + 1))
            {
                mainidx++;
                if(mainidx > 1)
                {
                    pu::render::DeleteTexture(entryicons.front());
                    entryicons.erase(entryicons.begin());
                }
                if(entries.size() > (mainidx + 5)) entryicons.push_back(pu::render::LoadImage(entries[mainidx + 5]));
                focuscb(mainidx);
            }
        }
    }

    void EntryListMenu::AddEntryIcon(std::string Path)
    {
        entries.push_back(Path);
    }

    void EntryListMenu::ClearEntries()
    {
        for(u32 i = 0; i < entryicons.size(); i++) pu::render::DeleteTexture(entryicons[i]);
        entries.clear();
        entryicons.clear();
        mainidx = 0;
    }

    void EntryListMenu::SetOnEntryFocus(std::function<void(u32)> OnFocus)
    {
        focuscb = OnFocus;
    }
    
    void EntryListMenu::SetOnEntrySelect(std::function<void(u32)> OnSelect)
    {
        selcb = OnSelect;
    }

    void EntryListMenu::OnRender(pu::render::Renderer *Drawer)
    {
        if(entries.empty()) return;
        if(entryicons.empty())
        {
            auto sz = std::min(entries.size(), (size_t)7);
            for(u32 i = 0; i < sz; i++)
            {
                entryicons.push_back(pu::render::LoadImage(entries[i]));
            }
        }

        u32 basex = 33 + 35;
        u32 basey = 368;
        u32 normalmargin = 30;
        u32 mainmargin = 35;
        u32 normalsize = 192;
        u32 mainsize = 256;

        u32 premaincount = mainidx;
        if(premaincount > 0)
        {
            basex -= (mainmargin + normalsize * premaincount + normalmargin * (premaincount - 1));
        }
        for(u32 i = 0; i < entryicons.size(); i++)
        {
            if(i == mainidx)
            {
                Drawer->RenderTextureScaled(entryicons[i], basex, basey - 64, mainsize, mainsize);
                if((int)i == openedidx)
                {
                    Drawer->RenderRectangleFill({ 50, 210, 80, 175 }, basex, basey - 64 + mainsize + 10, mainsize, 10);
                }
                basex += mainsize + mainmargin;
            }
            else
            {
                Drawer->RenderTextureScaled(entryicons[i], basex, basey, normalsize, normalsize);
                if((int)i == openedidx)
                {
                    Drawer->RenderRectangleFill({ 50, 210, 80, 175 }, basex, basey + normalsize + 10, normalsize, 10);
                }
                basex += normalsize + normalmargin;
                if((i + 1) == mainidx)
                {
                    basex -= normalmargin;
                    basex += mainmargin;
                }
                
            }
        }
    }

    void EntryListMenu::SetOpenedItemIndex(int Index)
    {
        openedidx = Index;
    }

    int EntryListMenu::GetOpenedItemIndex()
    {
        return openedidx;
    }

    void EntryListMenu::SetFocusedItemIndex(u32 Index)
    {
        mainidx = Index;
    }

    u32 EntryListMenu::GetFocusedItemIndex()
    {
        return mainidx;
    }
}