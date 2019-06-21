
#pragma once
#include <ui/IButton.hpp>

namespace ui
{
    class TextButton : public IButton
    {
        public:
            TextButton(s32 X, s32 Y, std::string Text, s32 Size, pu::draw::Color Color, pu::draw::Color TextColor, u32 Radius = 20);
            ~TextButton();
            void OnTopRender(pu::render::Renderer *Drawer, s32 X, s32 Y);
            void SetText(std::string Text);
        private:
            pu::draw::Color textclr;
            pu::render::NativeFont fnt;
            pu::render::NativeTexture ntex;
    };
}