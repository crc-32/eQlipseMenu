#include <ui/IButton.hpp>

namespace ui
{
    IButton::IButton(s32 X, s32 Y, pu::draw::Color ButtonColor, u32 Radius)
    {
        x = X;
        y = Y;
        rad = Radius;
        baseclr = ButtonColor;
        click = [](){};
        clickcooldown = false;
        presskey = 0;
    }

    s32 IButton::GetX()
    {
        return x;
    }

    void IButton::SetX(s32 X)
    {
        x = X;
    }

    s32 IButton::GetY()
    {
        return y;
    }

    void IButton::SetY(s32 Y)
    {
        y = Y;
    }

    s32 IButton::GetWidth()
    {
        return w;
    }

    void IButton::SetWidth(s32 Width)
    {
        w = Width;
    }

    s32 IButton::GetHeight()
    {
        return h;
    }

    void IButton::SetHeight(s32 Height)
    {
        h = Height;
    }

    void IButton::SetPressKey(u64 Key)
    {
        presskey = Key;
    }

    void IButton::OnInput(u64 Down, u64 Up, u64 Held, bool Touch, bool Focus)
    {
        if(clickcooldown)
        {
            auto ctime = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(ctime - basetime).count();
            if(diff >= 250)
            {
                clickcooldown = false;
                click();
            }
        }
        if(Touch)
        {
            touchPosition tch;
            hidTouchRead(&tch, 0);
            if(((s32)tch.px >= x) && ((s32)tch.px < (x + w)) && ((s32)tch.py >= y) && ((s32)tch.py < (y + h)))
            {
                if(!clickcooldown)
                {
                    clickcooldown = true;
                    basetime = std::chrono::steady_clock::now();
                }
            }
        }
        if(Down & presskey)
        {
            if(!clickcooldown)
            {
                clickcooldown = true;
                basetime = std::chrono::steady_clock::now();
            }
        }
    }

    void IButton::SetOnClick(std::function<void()> Click)
    {
        click = Click;
    }

    static u8 CompMinus(u8 Input, u8 Diff)
    {
        s32 val = (s32)Input - (s32)Diff;
        if(val < 0) val = 0;
        return (u8)val;
    }

    void IButton::OnRender(pu::render::Renderer *Drawer)
    {
        pu::draw::Color upclr = baseclr;
        pu::draw::Color downclr = { CompMinus(baseclr.R, 30), CompMinus(baseclr.G, 30), CompMinus(baseclr.B, 30), baseclr.A };
        s32 cropy = y;
        if(!clickcooldown) cropy -= 8;
        Drawer->RenderRoundedRectangleFill(downclr, x, y, w, h, rad);
        Drawer->RenderRoundedRectangleFill(upclr, x, cropy, w, h, rad);
        OnTopRender(Drawer, x, cropy);
    }
}