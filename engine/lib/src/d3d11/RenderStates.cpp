#include "RenderStates.h"
#include "GTString.h"

namespace Galaxy3D
{
	const std::string RenderStates::Keys[KeyCount] =
	{
		"Cull",
		"ZWrite",
		"ZTest",
		"Offset",
		"Blend",
		"BlendOp",
		"ColorMask",
		"Fog",
		"Stencil",
	};

	const std::string RenderStates::CullNames[CullNameCount] =
	{
		"Back",
		"Front",
		"Off",
	};

    static const D3D11_CULL_MODE CullValues[RenderStates::CullNameCount] =
    {
        D3D11_CULL_BACK,
        D3D11_CULL_FRONT,
        D3D11_CULL_NONE,
    };

	const std::string RenderStates::ZWriteNames[ZWriteNameCount] =
	{
		"On",
		"Off",
	};

    static const D3D11_DEPTH_WRITE_MASK ZWriteValues[RenderStates::ZWriteNameCount] = 
    {
        D3D11_DEPTH_WRITE_MASK_ALL,
        D3D11_DEPTH_WRITE_MASK_ZERO,
    };

	const std::string RenderStates::ZTestNames[ZTestNameCount] =
	{
		"Less",
		"Greater",
		"LEqual",
		"GEqual",
		"Equal",
		"NotEqual",
		"Always",
	};

    static const D3D11_COMPARISON_FUNC ZTestValues[RenderStates::ZTestNameCount] = 
    {
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_ALWAYS,
    };

	const std::string RenderStates::BlendNames[BlendNameCount] =
	{
		"Off",
		"One",
		"Zero",
		"SrcColor",
		"SrcAlpha",
		"DstColor",
		"DstAlpha",
		"OneMinusSrcColor",
		"OneMinusSrcAlpha",
		"OneMinusDstColor",
		"OneMinusDstAlpha",
	};

    static const int BlendValues[RenderStates::BlendNameCount] = 
    {
        0,
        D3D11_BLEND_ONE,
        D3D11_BLEND_ZERO,
        D3D11_BLEND_SRC_COLOR,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_DEST_COLOR,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_INV_SRC_COLOR,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_INV_DEST_COLOR,
        D3D11_BLEND_INV_DEST_ALPHA,
    };

	const std::string RenderStates::BlendOpNames[BlendOpNameCount] =
	{
		"Add",
		"Sub",
		"RevSub",
		"Min",
		"Max",
	};

    static const D3D11_BLEND_OP BlendOpValues[RenderStates::BlendOpNameCount] = 
    {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_MIN,
        D3D11_BLEND_OP_MAX,
    };

    struct StencilKey
    {
        enum Enum
        {
            Ref,
            ReadMask,
            WriteMask,
            Comp,
            Pass,
            Fail,
            ZFail,
        };
    };
    static const int StencilKeyNamesCount = 7;
    static const std::string StencilKeyNames[StencilKeyNamesCount] =
    {
        "Ref",
        "ReadMask",
        "WriteMask",
        "Comp",
        "Pass",
        "Fail",
        "ZFail",
    };

    const std::string RenderStates::StencilComparisonFunctionNames[StencilComparisonFunctionNameCount] =
    {
        "Greater",
        "GEqual",
        "Less",
        "LEqual",
        "Equal",
        "NotEqual",
        "Always",
        "Never",
    };

    static const D3D11_COMPARISON_FUNC StencilComparisonFunctionValues[RenderStates::StencilComparisonFunctionNameCount] = 
    {
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_ALWAYS,
        D3D11_COMPARISON_NEVER,
    };

    const std::string RenderStates::StencilOperationNames[StencilOperationNameCount] =
    {
        "Keep",
        "Zero",
        "Replace",
        "IncrSat",
        "DecrSat",
        "Invert",
        "IncrWrap",
        "DecrWrap",
    };

    static const D3D11_STENCIL_OP StencilOperationValues[RenderStates::StencilOperationNameCount] = 
    {
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_ZERO,
        D3D11_STENCIL_OP_REPLACE,
        D3D11_STENCIL_OP_INCR_SAT,
        D3D11_STENCIL_OP_DECR_SAT,
        D3D11_STENCIL_OP_INVERT,
        D3D11_STENCIL_OP_INCR,
        D3D11_STENCIL_OP_DECR,
    };

	std::string RenderStates::m_current_states[KeyCount];

	RenderStates::RenderStates():
		resterizer_state(NULL),
		depth_stencil_state(NULL),
		blend_state(NULL),
        m_stencil_ref(0)
	{
		m_values[0] = CullNames[0];
		m_values[1] = ZWriteNames[0];
		m_values[2] = ZTestNames[2];
		m_values[3] = "0, 0";
		m_values[4] = BlendNames[0];
		m_values[5] = BlendOpNames[0];
		m_values[6] = "RGBA";
		m_values[7] = "";
		m_values[8] = "";
	}

	void RenderStates::Parse(const std::string &s)
	{
		GTString str = s;
		str = str.Replace("\r\n", "\n");
		str = str.Replace("\t", "");
        str = str.Replace("    ", "");
		std::vector<GTString> lines = str.Split("\n", true);

		int stencil_index = -1;
		for(size_t i=0; i<lines.size();)
		{
			if(lines[i].StartsWith("Stencil"))
			{
				stencil_index = i;
				lines[stencil_index].str += " ";
			}
			else
			{
				if(stencil_index >= 0)
				{
					lines[stencil_index].str += "\n" + lines[i].str;
					if(lines[i].StartsWith("}"))
					{
						stencil_index = -1;
					}
					lines.erase(lines.begin() + i);

					continue;
				}
			}

			i++;
		}

		for(size_t i=0; i<lines.size(); i++)
		{
			for(int j=0; j<KeyCount; j++)
			{
				size_t key = lines[i].str.find(Keys[j]);
				if(key != std::string::npos && lines[i].str[key + Keys[j].length()] == ' ')
				{
 					std::string value = lines[i].str.substr(key + Keys[j].length());
					
					int left = 0;
					while(value[left] == ' ')
					{
						left++;
					}

					int right = (int) value.length() - 1;
					while(value[right] == ' ')
					{
						right--;
					}

					value = value.substr(left, right);

					m_values[j] = value;
					break;
				}
			}
		}
	}

	void RenderStates::Release()
	{
		SAFE_RELEASE(resterizer_state);
		SAFE_RELEASE(depth_stencil_state);
		SAFE_RELEASE(blend_state);
	}

	void RenderStates::Create()
	{
		auto device = GraphicsDevice::GetInstance()->GetDevice();

		int cull_index = -1;
		for(int i=0; i<RenderStates::CullNameCount; i++)
		{
			if(m_values[Key::Cull] == RenderStates::CullNames[i])
			{
				cull_index = i;
				break;
			}
		}

		int zwrite_index = -1;
		for(int i=0; i<RenderStates::ZWriteNameCount; i++)
		{
			if(m_values[Key::ZWrite] == RenderStates::ZWriteNames[i])
			{
				zwrite_index = i;
				break;
			}
		}

		int ztest_index = -1;
		for(int i=0; i<RenderStates::ZTestNameCount; i++)
		{
			if(m_values[Key::ZTest] == RenderStates::ZTestNames[i])
			{
				ztest_index = i;
				break;
			}
		}

		float zoffset_factor = 0;
		float zoffset_units = 0;
		GTString zoffset = m_values[Key::Offset];
		zoffset = zoffset.Replace(" ", "");
		std::vector<GTString> zoffset_values = zoffset.Split(",");
		if(zoffset_values.size() == 2)
		{
			std::stringstream ss_factor(zoffset_values[0].str);
			ss_factor >> zoffset_factor;

			std::stringstream ss_units(zoffset_values[1].str);
			ss_units >> zoffset_units;
		}

		bool blend_enable = false;
		int blends_index[4];
		blends_index[0] = -1;
		blends_index[1] = -1;
		blends_index[2] = -1;
		blends_index[3] = -1;
		GTString blend = m_values[Key::Blend];
		if(blend.str != RenderStates::BlendNames[0])
		{
			blend_enable = true;

			blend = blend.Replace(",", " ");
			std::vector<GTString> blends = blend.Split(" ");
			for(size_t i=0; i<blends.size(); )
			{
				if(blends[i].str == "")
				{
					blends.erase(blends.begin() + i);
					continue;
				}

				i++;
			}

			for(size_t j=0; j<4 && j<blends.size(); j++)
			{
				for(int i=0; i<RenderStates::BlendNameCount; i++)
				{
					if(blends[j].str == RenderStates::BlendNames[i])
					{
						blends_index[j] = i;
						break;
					}
				}
			}

			if(blends.size() == 2)
			{
				blends_index[2] = 4;
				blends_index[3] = 8;
			}
		}

		int blendop_index = -1;
		for(int i=0; i<RenderStates::BlendOpNameCount; i++)
		{
			if(m_values[Key::BlendOp] == RenderStates::BlendOpNames[i])
			{
				blendop_index = i;
				break;
			}
		}

		int color_mask = 0;
		std::string color_mask_str = m_values[Key::ColorMask];
		for(size_t i=0; i<color_mask_str.size(); i++)
		{
			if(color_mask_str[i] == 'R')
			{
				color_mask |= D3D11_COLOR_WRITE_ENABLE_RED;
			}
			else if(color_mask_str[i] == 'G')
			{
				color_mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
			}
			else if(color_mask_str[i] == 'B')
			{
				color_mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
			}
			else if(color_mask_str[i] == 'A')
			{
				color_mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
			}
		}

        bool stencil_enable = false;
        std::string stencil_str = m_values[Key::Stencil];
        int stencil_values[StencilKeyNamesCount] =
        {
            0, 255, 255,
            D3D11_COMPARISON_ALWAYS,
            D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP_KEEP
        };
        if(!stencil_str.empty())
        {
            stencil_enable = true;

            auto values = GTString(stencil_str).Replace("\r\n", "\n").Split("\n", true);
            for(auto i : values)
            {
                auto pair = GTString(i).Split(" ", true);
                if(pair.size() == 2)
                {
                    auto key = pair[0].str;

                    if(key == "Comp")
                    {
                        for(int j=0; j<StencilComparisonFunctionNameCount; j++)
                        {
                            if(pair[1].str == StencilComparisonFunctionNames[j])
                            {
                                stencil_values[StencilKey::Comp] = StencilComparisonFunctionValues[j];
                                break;
                            }
                        }

                        continue;
                    }

                    for(int j=0; j<StencilKeyNamesCount; j++)
                    {
                        if((key == "Ref" ||
                            key == "ReadMask" ||
                            key == "WriteMask") && key == StencilKeyNames[j])
                        {
                            auto value = GTString::ToType<int>(pair[1].str);
                            stencil_values[j] = value;
                            break;
                        }
                        else if((key == "Pass" ||
                            key == "Fail" ||
                            key == "ZFail") && key == StencilKeyNames[j])
                        {
                            for(int k=0; k<StencilOperationNameCount; k++)
                            {
                                if(pair[1].str == StencilOperationNames[k])
                                {
                                    stencil_values[j] = StencilOperationValues[k];
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

		bool line_mode = false;

		//rasterizer
		D3D11_RASTERIZER_DESC rd;
		ZeroMemory(&rd, sizeof(rd));
		rd.FillMode = line_mode ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		rd.CullMode = CullValues[cull_index];
		rd.FrontCounterClockwise = false;
		rd.DepthClipEnable = true;
		rd.DepthBias = (int) zoffset_units;
		rd.SlopeScaledDepthBias = zoffset_factor;
		rd.DepthBiasClamp = 0;
		rd.ScissorEnable = false;
		rd.MultisampleEnable = false;
		rd.AntialiasedLineEnable = false;

		device->CreateRasterizerState(&rd, &resterizer_state);

		//depth stencil
		D3D11_DEPTH_STENCIL_DESC dsd;
		ZeroMemory(&dsd, sizeof(dsd));
		dsd.DepthEnable = true;
		dsd.DepthWriteMask = ZWriteValues[zwrite_index];
		dsd.DepthFunc = ZTestValues[ztest_index];
        if(stencil_enable)
        {
            dsd.StencilEnable = true;
            dsd.StencilReadMask = stencil_values[StencilKey::ReadMask];
            dsd.StencilWriteMask = stencil_values[StencilKey::WriteMask];
            dsd.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC) stencil_values[StencilKey::Comp];
            dsd.FrontFace.StencilPassOp = (D3D11_STENCIL_OP) stencil_values[StencilKey::Pass];
            dsd.FrontFace.StencilFailOp = (D3D11_STENCIL_OP) stencil_values[StencilKey::Fail];
            dsd.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP) stencil_values[StencilKey::ZFail];
            dsd.BackFace = dsd.FrontFace;

            m_stencil_ref = stencil_values[StencilKey::Ref];
        }

		device->CreateDepthStencilState(&dsd, &depth_stencil_state);

		//blend
		D3D11_BLEND_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.AlphaToCoverageEnable = false;
		bd.IndependentBlendEnable = false;
		bd.RenderTarget[0].RenderTargetWriteMask = color_mask;
		bd.RenderTarget[0].BlendEnable = blend_enable;
		if(blend_enable)
		{
			bd.RenderTarget[0].SrcBlend = (D3D11_BLEND) BlendValues[blends_index[0]];
			bd.RenderTarget[0].DestBlend = (D3D11_BLEND) BlendValues[blends_index[1]];
			bd.RenderTarget[0].BlendOp = BlendOpValues[blendop_index];
			bd.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND) BlendValues[blends_index[2]];
			bd.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND) BlendValues[blends_index[3]];
			bd.RenderTarget[0].BlendOpAlpha = BlendOpValues[blendop_index];
		}

		device->CreateBlendState(&bd, &blend_state);
	}

	void RenderStates::Apply()
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		if(m_current_states[0].empty())
		{
			context->RSSetState(resterizer_state);
			context->OMSetDepthStencilState(depth_stencil_state, m_stencil_ref);
			context->OMSetBlendState(blend_state, 0, 0xffffffff);
		}
		else
		{
			if(	m_current_states[Key::Cull] != m_values[Key::Cull] ||
				m_current_states[Key::Offset] != m_values[Key::Offset])
			{
				context->RSSetState(resterizer_state);
			}

			if(	m_current_states[Key::ZWrite] != m_values[Key::ZWrite] ||
				m_current_states[Key::ZTest] != m_values[Key::ZTest] ||
                m_current_states[Key::Stencil] != m_values[Key::Stencil])
			{
				context->OMSetDepthStencilState(depth_stencil_state, m_stencil_ref);
			}

			if(	m_current_states[Key::ColorMask] != m_values[Key::ColorMask] ||
				m_current_states[Key::Blend] != m_values[Key::Blend] ||
				m_current_states[Key::BlendOp] != m_values[Key::BlendOp])
			{
				context->OMSetBlendState(blend_state, 0, 0xffffffff);
			}
		}

		//set to current
		for(int i=0; i<KeyCount; i++)
		{
			m_current_states[i] = m_values[i];
		}
	}
}