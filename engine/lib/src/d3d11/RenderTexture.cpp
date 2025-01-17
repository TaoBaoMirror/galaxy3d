#include "RenderTexture.h"
#include "Debug.h"

namespace Galaxy3D
{
    std::list<std::shared_ptr<RenderTexture>> RenderTexture::m_textures_idle;
    std::list<std::shared_ptr<RenderTexture>> RenderTexture::m_textures_using;

    std::shared_ptr<RenderTexture> RenderTexture::GetTemporary(
        int width,
        int height,
        RenderTextureFormat::Enum format,
        DepthBuffer::Enum depth)
    {
        std::shared_ptr<RenderTexture> texture;

        for(auto i=m_textures_idle.begin(); i!=m_textures_idle.end(); i++)
        {
            auto t = *i;
            if(	t->GetWidth() == width &&
                t->GetHeight() == height &&
                t->GetFormat() == format &&
                t->GetDepth() == depth)
            {
                texture = t;

                m_textures_idle.erase(i);
                m_textures_using.push_front(texture);
                break;
            }
        }

        if(!texture)
        {
            texture = Create(width, height, format, depth, FilterMode::Bilinear);

            m_textures_using.push_front(texture);
        }

        return texture;
    }

    void RenderTexture::ReleaseTemporary(const std::shared_ptr<RenderTexture> &temp)
    {
        for(auto i=m_textures_using.begin(); i!=m_textures_using.end(); i++)
        {
            if((*i) == temp)
            {
                m_textures_using.erase(i);
                m_textures_idle.push_front(temp);
                break;
            }
        }
    }

    void RenderTexture::SwapColorBuffer(std::shared_ptr<RenderTexture> &t1, std::shared_ptr<RenderTexture> &t2)
    {
        auto temp_rt = t1->m_render_target_view;
        t1->m_render_target_view = t2->m_render_target_view;
        t2->m_render_target_view = temp_rt;

        auto temp_srv = t1->m_shader_resource_view_color;
        t1->m_shader_resource_view_color = t2->m_shader_resource_view_color;
        t2->m_shader_resource_view_color = temp_srv;
    }

    std::shared_ptr<RenderTexture> RenderTexture::Create(
        int width,
        int height,
        RenderTextureFormat::Enum format,
        DepthBuffer::Enum depth,
        FilterMode::Enum filter_mode,
        TextureWrapMode::Enum wrap_mode)
    {
        auto texture = std::shared_ptr<RenderTexture>(new RenderTexture(width, height, format, depth, filter_mode, wrap_mode));
        texture->Create();

        return texture;
    }

    RenderTexture::~RenderTexture()
    {
        SAFE_RELEASE(m_render_target_view);
        SAFE_RELEASE(m_depth_stencil_view);
        SAFE_RELEASE(m_shader_resource_view_color);
        SAFE_RELEASE(m_shader_resource_view_depth);
        SAFE_RELEASE(m_sampler_state);
    }

    void RenderTexture::Create()
    {
        auto device = GraphicsDevice::GetInstance()->GetDevice();

		if(m_format == RenderTextureFormat::Depth)
		{
			D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT d3d9_shadow_support;
			ZeroMemory(&d3d9_shadow_support, sizeof(D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT));

			device->CheckFeatureSupport(
					D3D11_FEATURE_D3D9_SHADOW_SUPPORT, 
					&d3d9_shadow_support,
					sizeof(D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT)
					);
			if(d3d9_shadow_support.SupportsDepthAsTextureWithLessEqualComparisonFilter == FALSE)
			{
				Debug::Log("RenderTexture::Create:Device do not support d3d9 shadow!");
			}
		}

        {
            DXGI_FORMAT fmt;

            if(m_format == RenderTextureFormat::RGBA32)
            {
                fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            else if(m_format == RenderTextureFormat::RGBAHalf)
            {
                fmt = DXGI_FORMAT_R16G16B16A16_FLOAT;
            }
            else if(m_format == RenderTextureFormat::RGHalf)
            {
                fmt = DXGI_FORMAT_R16G16_FLOAT;
            }
            else if(m_format == RenderTextureFormat::RFloat || m_format == RenderTextureFormat::Depth)
            {
                fmt = DXGI_FORMAT_R32_FLOAT;
            }

            D3D11_TEXTURE2D_DESC td =
            {
                (UINT) m_width,//UINT Width;
                (UINT) m_height,//UINT Height;
                1,//UINT MipLevels;
                1,//UINT ArraySize;
                fmt,//DXGI_FORMAT Format;
                1, 0,//DXGI_SAMPLE_DESC SampleDesc;
                D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
                D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
                0,//UINT CPUAccessFlags;
                0//UINT MiscFlags;
            };

            ID3D11Texture2D *texture = 0;
            device->CreateTexture2D(&td, NULL, &texture);
            device->CreateRenderTargetView(texture, NULL, &m_render_target_view);
            device->CreateShaderResourceView(texture, NULL, &m_shader_resource_view_color);
            texture->Release();
        }

        if(m_depth != DepthBuffer::Depth_0 || m_format == RenderTextureFormat::Depth)
        {
            DXGI_FORMAT fmt;
            DXGI_FORMAT fmt_dsv;
            DXGI_FORMAT fmt_srv;
            if(m_depth == DepthBuffer::Depth_16)
            {
                fmt = DXGI_FORMAT_R16_TYPELESS;
                fmt_dsv = DXGI_FORMAT_D16_UNORM;
                fmt_srv = DXGI_FORMAT_R16_UNORM;
            }
            else if(m_depth == DepthBuffer::Depth_24 || m_format == RenderTextureFormat::Depth)
            {
                fmt = DXGI_FORMAT_R24G8_TYPELESS;
                fmt_dsv = DXGI_FORMAT_D24_UNORM_S8_UINT;
                fmt_srv = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            }

            D3D11_TEXTURE2D_DESC td =
            {
                (UINT) m_width,//UINT Width;
                (UINT) m_height,//UINT Height;
                1,//UINT MipLevels;
                1,//UINT ArraySize;
                fmt,//DXGI_FORMAT Format;
                1, 0,//DXGI_SAMPLE_DESC SampleDesc;
                D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
                D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
                0,//UINT CPUAccessFlags;
                0//UINT MiscFlags;    
            };

            ID3D11Texture2D *depth_texture = 0;
            device->CreateTexture2D(&td, NULL, &depth_texture);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
            ZeroMemory(&dsvd, sizeof(dsvd));
            dsvd.Format = fmt_dsv;
            dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            device->CreateDepthStencilView(depth_texture, &dsvd, &m_depth_stencil_view);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
            ZeroMemory(&srvd, sizeof(srvd));
            srvd.Format = fmt_srv;
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvd.Texture2D.MipLevels = 1;
            device->CreateShaderResourceView(depth_texture, &srvd, &m_shader_resource_view_depth);

            depth_texture->Release();
        }

		// create sampler states
        D3D11_SAMPLER_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Filter = Texture::FILTER_MODES[m_filter_mode];
		sd.AddressU = Texture::ADDRESS_MODES[m_wrap_mode];
		sd.AddressV = Texture::ADDRESS_MODES[m_wrap_mode];
		sd.AddressW = Texture::ADDRESS_MODES[m_wrap_mode];
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

        device->CreateSamplerState(&sd, &m_sampler_state);
    }
}