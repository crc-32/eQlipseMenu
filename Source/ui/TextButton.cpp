#include <ui/TextButton.hpp>

namespace ui
{
    TextButton::TextButton(s32 X, s32 Y, std::string Text, s32 Size, pu::draw::Color Color, pu::draw::Color TextColor, u32 Radius) : IButton(X, Y, Color, Radius)
    {
        textclr = TextColor;
        fnt = pu::render::LoadDefaultFont(Size);
        ntex = pu::render::RenderText(fnt, Text, textclr);
        w = 70 + pu::render::GetTextWidth(fnt, Text);
        h = 30 + pu::render::GetTextHeight(fnt, Text);
    }

    TextButton::~TextButton()
    {
        if(this->fnt != NULL)
        {
            pu::render::DeleteFont(this->fnt);
            this->fnt = NULL;
        }
        if(this->ntex != NULL)
        {
            pu::render::DeleteTexture(this->ntex);
            this->ntex = NULL;
        }
    }

    void TextButton::OnTopRender(pu::render::Renderer *Drawer, s32 X, s32 Y)
    {
        Drawer->RenderTexture(ntex, X + (w - pu::render::GetTextureWidth(ntex)) / 2, Y + (h - pu::render::GetTextureHeight(ntex)) / 2);
    }

    void TextButton::SetText(std::string Text)
    {
        pu::render::DeleteTexture(this->ntex);
        ntex = pu::render::RenderText(fnt, Text, textclr);
    }
}