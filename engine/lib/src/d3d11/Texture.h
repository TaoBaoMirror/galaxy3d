#ifndef __Texture_h__
#define __Texture_h__

#include "Object.h"
#include "FilterMode.h"
#include "TextureWrapMode.h"
#include "GraphicsDevice.h"

namespace Galaxy3D
{
	class Texture : public Object
	{
	public:
        static const DXGI_FORMAT *TEXTURE_FORMATS;
        static const D3D11_FILTER *FILTER_MODES;
        static const D3D11_TEXTURE_ADDRESS_MODE *ADDRESS_MODES;
        static const int PIXEL_BITS_SIZE[20];

		virtual ~Texture() {}
		int GetWidth() const {return m_width;}
		int GetHeight() const {return m_height;}

	protected:
		FilterMode::Enum m_filter_mode;
		TextureWrapMode::Enum m_wrap_mode;
		int m_width;
		int m_height;

		Texture(int w, int h, FilterMode::Enum filter_mode, TextureWrapMode::Enum wrap_mode):
			m_filter_mode(filter_mode),
			m_wrap_mode(wrap_mode),
			m_width(w),
			m_height(h)
		{}
	};
}

#endif