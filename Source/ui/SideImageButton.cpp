#include <ui/SideImageButton.hpp>

namespace ui
{
    SideImageButton::SideImageButton(Side Side, s32 Y, std::string Image, s32 ImageSize, pu::draw::Color Color, s32 ButtonWidth, s32 ButtonHeight, s32 ImageX, s32 Radius) : IButton(0, Y, Color, Radius)
    {
        switch(Side)
        {
            case Side::Left:
                x = -(3 * Radius);
                w = ButtonWidth - x;
                h = ButtonHeight;
                y = Y;
                imagex = ImageX;
                break;
            case Side::Right:
                x = 1280 - ButtonWidth;
                w = ButtonWidth  + (3 * Radius);
                h = ButtonHeight;
                y = Y;
                imagex = x + (ButtonWidth - ImageSize - ImageX);
                break;
        }
        ntex = pu::render::LoadImage(Image);
        imagesz = ImageSize;
    }

    SideImageButton::~SideImageButton()
    {
        if(this->ntex != NULL)
        {
            pu::render::DeleteTexture(this->ntex);
            this->ntex = NULL;
        }
    }

    void SideImageButton::OnTopRender(pu::render::Renderer *Drawer, s32 X, s32 Y)
    {
        Drawer->RenderTextureScaled(ntex, imagex, Y + (h - imagesz) / 2, imagesz, imagesz);
    }
}