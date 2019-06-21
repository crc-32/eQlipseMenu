
#pragma once
#include <pu/Plutonium>

namespace ui
{
    class IButton : public pu::element::Element
    {
        public:
            IButton(s32 X, s32 Y, pu::draw::Color ButtonColor, u32 Radius = 20);
            s32 GetX();
            void SetX(s32 X);
            s32 GetY();
            void SetY(s32 Y);
            s32 GetWidth();
            void SetWidth(s32 Width);
            s32 GetHeight();
            void SetHeight(s32 Height);
            void SetPressKey(u64 Key);
            void OnInput(u64 Down, u64 Up, u64 Held, bool Touch, bool Focus);
            void SetOnClick(std::function<void()> Click);
            void OnRender(pu::render::Renderer *Drawer);
            virtual void OnTopRender(pu::render::Renderer *Drawer, s32 X, s32 Y) = 0;
        protected:
            u32 rad;
            u64 presskey;
            pu::draw::Color baseclr;
            std::function<void()> click;
            bool clickcooldown;
            std::chrono::time_point<std::chrono::steady_clock> basetime;
            s32 x;
            s32 y;
            s32 w;
            s32 h;
    };
}