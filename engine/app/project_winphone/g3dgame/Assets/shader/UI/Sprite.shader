UI/Sprite
{
	Tags
	{
		Queue Transparent
	}

	Pass
	{
		VS vs
		PS ps
		RenderStates rs
	}

	RenderStates rs
	{
		Cull Off
		ZWrite Off
		ZTest Always
		Blend SrcAlpha OneMinusSrcAlpha
	}

	HLVS vs
	{
		cbuffer cbuffer0 : register(b0)
		{
			matrix WorldViewProjection;
		};

		struct VS_INPUT
		{
			float4 Position : POSITION;
			float4 Color : COLOR;
			float2 Texcoord0 : TEXCOORD0;
		};

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR;
            float4 v_pos_proj : TEXCOORD1;
		};

		PS_INPUT main(VS_INPUT input)
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul(input.Position, WorldViewProjection);
			output.v_uv = input.Texcoord0;
			output.v_color = input.Color;
            output.v_pos_proj = output.v_pos;

			return output;
		}
	}

	HLPS ps
	{
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Color;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 ClipRect;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 ClipSoft;
        };
        
		Texture2D _MainTex : register(t0);
		SamplerState _MainTex_Sampler : register(s0);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR;
            float4 v_pos_proj : TEXCOORD1;
		};

		float4 main(PS_INPUT input) : SV_Target
		{
			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color * _Color;

            float2 pos = input.v_pos_proj.xy / input.v_pos_proj.w;

            float alpha = 1.0;
            if(pos.y > ClipSoft.y)
            {
                alpha = (ClipRect.y - pos.y) / (ClipRect.y - ClipSoft.y);
            }
            else if(pos.y < ClipSoft.w)
            {
                alpha = (ClipRect.w - pos.y) / (ClipRect.w - ClipSoft.w);
            }
            if(pos.x > ClipSoft.z)
            {
                alpha *= (ClipRect.z - pos.x) / (ClipRect.z - ClipSoft.z);
            }
            else if(pos.x < ClipSoft.x)
            {
                alpha *= (ClipRect.x - pos.x) / (ClipRect.x - ClipSoft.x);
            }
            c.a *= alpha;

            if( pos.x - ClipRect.x < 0 ||
                ClipRect.z - pos.x < 0 ||
                ClipRect.y - pos.y < 0 ||
                pos.y - ClipRect.w < 0)
            {
                clip(-1);
            }

			return c;
		}
	}

	GLVS vs
	{
		uniform mat4 WorldViewProjection;

		attribute vec4 Position;
		attribute vec2 Texcoord0;
		attribute vec4 Color;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
			gl_Position = WorldViewProjection * Position;
			v_uv = Texcoord0;
			v_color = Color;
		}
	}

	GLPS ps
	{
		uniform sampler2D _MainTex;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
			vec4 color = texture2D(_MainTex, v_uv) * v_color;

			gl_FragColor = color;
		}
	}
}