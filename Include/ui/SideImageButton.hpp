
#pragma once
#include <ui/IButton.hpp>

namespace ui
{
    enum class Side
    {
        Left,
        Right
    };

    class SideImageButton : public IButton
    {
        public:
            SideImageButton(Side Side, s32 Y, std::string Image, s32 ImageSize, pu::draw::Color Color, s32 ButtonWidth, s32 ButtonHeight, s32 ImageX, s32 Radius);
            ~SideImageButton();
            void OnTopRender(pu::render::Renderer *Drawer, s32 X, s32 Y);
        private:
            s32 imagex;
            s32 imagesz;
            pu::render::NativeTexture ntex;
    };
}