#ifndef __ImageEffectToneMapping_h__
#define __ImageEffectToneMapping_h__

#include "ImageEffect.h"

namespace Galaxy3D
{
    struct AdaptiveTextureSize
    {
        enum Enum
        {
            Square16 = 16,
            Square32 = 32,
            Square64 = 64,
            Square128 = 128,
            Square256 = 256,
            Square512 = 512,
            Square1024 = 1024,
        };
    };

    class ImageEffectToneMapping : public ImageEffect
    {
    public:
        ImageEffectToneMapping():
            m_middle_gray(0.4f),
            m_white(2),
            m_adaption_speed(1.5f),
            m_adaptive_texture_size(AdaptiveTextureSize::Square256)
        {}
        virtual void OnRenderImage(const std::shared_ptr<RenderTexture> &source, const std::shared_ptr<RenderTexture> &destination);
        void SetMiddleGray(float gray) {m_middle_gray = gray;}
        void SetWhite(float white) {m_white = white;}
        void SetAdaptionSpeed(float speed) {m_adaption_speed = speed;}
        void SetAdaptiveTextureSize(AdaptiveTextureSize::Enum size) {m_adaptive_texture_size = size;}

    protected:
        virtual void Start();

    private:
        float m_middle_gray;
        float m_white;
        float m_adaption_speed;
        int m_adaptive_texture_size;
        std::shared_ptr<RenderTexture> m_adaptive_texture_1;

        bool NewAdaptiveTexture();
    };
}

#endif