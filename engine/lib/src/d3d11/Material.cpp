#include "Material.h"
#include "Debug.h"
#include "Texture2D.h"
#include "Renderer.h"
#include "Guid.h"
#include "RenderTexture.h"
#include "Camera.h"
#include "Cubemap.h"

static const std::string MAIN_TEXTURE_NAME = "_MainTex";
static const std::string MAIN_COLOR_NAME = "_Color";

namespace Galaxy3D
{
	std::shared_ptr<Material> Material::Create(const std::string &shader)
	{
		auto s = Shader::Find(shader);
		if(s)
		{
			return Create(s);
		}
		
		return std::shared_ptr<Material>();
	}

	std::shared_ptr<Material> Material::Create(const std::shared_ptr<Shader> &shader)
	{
		std::shared_ptr<Material> mat(new Material());
		mat->SetShader(shader);
        mat->SetGuid(Guid::NewGuid());

		return mat;
	}

	Material::Material():
		m_render_queue(-1)
	{
        SetMainColor(Color(1, 1, 1, 1));
	}

	void Material::SetShader(const std::shared_ptr<Shader> &shader)
	{
		m_shader = shader;
		SetName(m_shader->GetName());
	}

	void Material::SetRenderQueue(int queue)
	{
		m_render_queue = queue;

        Renderer::SortAllBatches();
	}

    int Material::GetRenderQueue() const
    {
        int queue = m_render_queue;

        if(queue < 0)
        {
            queue = m_shader->GetRenderQueue();
        }

        return queue;
    }

	void Material::SetVector(const std::string &name, const Vector4 &vector)
	{
		m_vectors[name] = vector;
	}

	void Material::SetColor(const std::string &name, const Color &color)
	{
		m_colors[name] = color;
	}

    void Material::SetMainColor(const Color &color)
    {
        m_colors[MAIN_COLOR_NAME] = color;
    }

	void Material::SetMatrix(const std::string &name, const Matrix4x4 &matrix)
	{
		m_matrices[name] = matrix;
	}

	void Material::SetMatrixArray(const std::string &name, const std::vector<Matrix4x4> &matrices)
	{
		m_matrix_arrays[name] = matrices;
	}

    void Material::SetVectorArray(const std::string &name, const std::vector<Vector4> &vectors)
    {
        m_vector_arrays[name] = vectors;
    }

	void Material::SetTexture(const std::string &name, const std::shared_ptr<Texture> &texture)
	{
		m_textures[name] = texture;
	}

	void Material::SetMainTexture(const std::shared_ptr<Texture> &texture)
	{
		m_textures[MAIN_TEXTURE_NAME] = texture;
	}

	std::shared_ptr<Texture> Material::GetMainTexture()
	{
		return GetTexture(MAIN_TEXTURE_NAME);
	}

	std::shared_ptr<Texture> Material::GetTexture(const std::string &name)
	{
		std::shared_ptr<Texture> tex;

		auto find = m_textures.find(name);
		if(find != m_textures.end())
		{
			tex = find->second;
		}

		return tex;
	}

	void Material::Copy(const Material &mat)
	{
		*this = mat;
	}

	static void set_constant_buffer(const std::string &name, void *data, int size, ShaderPass *pass)
	{
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		auto find = pass->vs->cbuffers.find(name);
		if(find != pass->vs->cbuffers.end())
		{
            void *p;

            if(size < find->second.size)
            {
                find->second.FillCPUBuffer(data, size);
                p = find->second.cpu_buffer;
            }
            else
            {
                p = data;
            }

			context->UpdateSubresource(find->second.buffer, 0, NULL, p, find->second.size, 0);
		}

        if(pass->ps != NULL)
        {
            find = pass->ps->cbuffers.find(name);
            if(find != pass->ps->cbuffers.end())
            {
                void *p;

                if(size < find->second.size)
                {
                    find->second.FillCPUBuffer(data, size);
                    p = find->second.cpu_buffer;
                }
                else
                {
                    p = data;
                }

                context->UpdateSubresource(find->second.buffer, 0, NULL, p, find->second.size, 0);
            }
        }
	}

    void Material::SetVectorDirectlyVS(const std::string &name, const Vector4 &vector, int pass)
    {
        SetVector(name, vector);

        auto shader_pass = m_shader->GetPass(pass);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        auto find = shader_pass->vs->cbuffers.find(name);
        if(find != shader_pass->vs->cbuffers.end())
        {
            context->UpdateSubresource(find->second.buffer, 0, NULL, &vector, sizeof(Vector4), 0);
        }
    }

    void Material::SetMatrixDirectlyVS(const std::string &name, const Matrix4x4 &matrix, int pass)
    {
        SetMatrix(name, matrix);

        auto shader_pass = m_shader->GetPass(pass);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        auto find = shader_pass->vs->cbuffers.find(name);
        if(find != shader_pass->vs->cbuffers.end())
        {
            context->UpdateSubresource(find->second.buffer, 0, NULL, &matrix, sizeof(Matrix4x4), 0);
        }
    }

    void Material::SetTextureDirectlyPS(const std::string &name, const std::shared_ptr<Texture> &texture, int pass)
    {
        SetTexture(name, texture);

        auto shader_pass = m_shader->GetPass(pass);
        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

        if(shader_pass->ps != NULL)
        {
            auto find = shader_pass->ps->textures.find(name);
            if(find != shader_pass->ps->textures.end())
            {
                auto tex = std::dynamic_pointer_cast<Texture2D>(texture);
                if(tex)
                {
                    find->second.texture = tex->GetTexture();
                    context->PSSetShaderResources(find->second.slot, 1, &find->second.texture);

                    auto find_sampler = shader_pass->ps->samplers.find(name + "_Sampler");
                    if(find_sampler != shader_pass->ps->samplers.end())
                    {
                        find_sampler->second.sampler = tex->GetSampler();
                        context->PSSetSamplers(find_sampler->second.slot, 1, &find_sampler->second.sampler);
                    }
                }
            }
        }
    }

	void Material::ReadyPass(int pass)
	{
		auto shader_pass = m_shader->GetPass(pass);

		for(auto &i : m_vectors)
		{
			set_constant_buffer(i.first, (void *) &i.second, sizeof(Vector4), shader_pass);
		}

		for(auto &i : m_colors)
		{
			set_constant_buffer(i.first, (void *) &i.second, sizeof(Color), shader_pass);
		}
		
		for(auto &i : m_matrices)
		{
			set_constant_buffer(i.first, (void *) &i.second, sizeof(Matrix4x4), shader_pass);
		}

		for(auto &i : m_matrix_arrays)
		{
			int size = i.second.size();
			if(size > 54)
			{
				Debug::Log("matrix array size is too big:%d", size);
			}
			else
			{
				set_constant_buffer(i.first, (void *) &i.second[0], sizeof(Matrix4x4) * size, shader_pass);
			}
		}

        for(auto &i : m_vector_arrays)
        {
            int size = i.second.size();
            if(size > 216)
            {
                Debug::Log("vector array size is too big:%d", size);
            }
            else
            {
                set_constant_buffer(i.first, (void *) &i.second[0], sizeof(Vector4) * size, shader_pass);
            }
        }

		if(shader_pass->ps != NULL)
		{
			for(auto &i : shader_pass->ps->textures)
			{
				i.second.texture = NULL;

				auto find = m_textures.find(i.first);
				if(find != m_textures.end())
				{
					if(find->second)
					{
						auto tex = std::dynamic_pointer_cast<Texture2D>(find->second);
						if(tex)
						{
							i.second.texture = tex->GetTexture();

							auto find_sampler = shader_pass->ps->samplers.find(i.first + "_Sampler");
							if(find_sampler != shader_pass->ps->samplers.end())
							{
								find_sampler->second.sampler = tex->GetSampler();
							}

							continue;
						}

						auto render_texture = std::dynamic_pointer_cast<RenderTexture>(find->second);
						if(render_texture)
						{
							if(i.first == "_CameraDepthTexture")
							{
								i.second.texture = render_texture->GetShaderResourceViewDepth();
							}
							else
							{
								i.second.texture = render_texture->GetShaderResourceViewColor();
							}

							auto find_sampler = shader_pass->ps->samplers.find(i.first + "_Sampler");
							if(find_sampler != shader_pass->ps->samplers.end())
							{
								find_sampler->second.sampler = render_texture->GetSamplerState();
							}

							continue;
						}

						auto cubemap = std::dynamic_pointer_cast<Cubemap>(find->second);
						if(cubemap)
						{
							i.second.texture = cubemap->GetTexture();

							auto find_sampler = shader_pass->ps->samplers.find(i.first + "_Sampler");
							if(find_sampler != shader_pass->ps->samplers.end())
							{
								find_sampler->second.sampler = cubemap->GetSampler();
							}

							continue;
						}
					}
				}

				if(i.second.texture == NULL)
				{
					auto tex = Texture2D::GetDefaultTexture();
					i.second.texture = tex->GetTexture();

					auto find_sampler = shader_pass->ps->samplers.find(i.first + "_Sampler");
					if(find_sampler != shader_pass->ps->samplers.end())
					{
						find_sampler->second.sampler = tex->GetSampler();
					}
				}
			}
		}
	}

	void Material::ApplyPass(int pass)
	{
		auto shader_pass = m_shader->GetPass(pass);
		auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

		context->VSSetShader(shader_pass->vs->shader, NULL, 0);
        if(shader_pass->ps != NULL)
        {
            context->PSSetShader(shader_pass->ps->shader, NULL, 0);
        }
        else
        {
            context->PSSetShader(NULL, NULL, 0);
        }

		for(auto &i : shader_pass->vs->cbuffers)
		{
			context->VSSetConstantBuffers(i.second.slot, 1, &i.second.buffer);
		}

        if(shader_pass->ps != NULL)
        {
            for(auto &i : shader_pass->ps->cbuffers)
            {
                context->PSSetConstantBuffers(i.second.slot, 1, &i.second.buffer);
            }

            for(auto &i : shader_pass->ps->textures)
            {
                context->PSSetShaderResources(i.second.slot, 1, &i.second.texture);
            }

            for(auto &i : shader_pass->ps->samplers)
            {
                context->PSSetSamplers(i.second.slot, 1, &i.second.sampler);
            }
        }
	}

    void Material::SetZBufferParams(std::shared_ptr<Camera> &cam)
    {
        float cam_far = cam->GetClipFar();
        float cam_near = cam->GetClipNear();

#if defined(WINPC) || defined(WINPHONE)
        float zx = (1.0f - cam_far / cam_near) / 2;
        float zy = (1.0f + cam_far / cam_near) / 2;
#else
        float zx = (1.0f - cam_far / cam_near);
        float zy = (cam_far / cam_near);
#endif

        SetVector("_ZBufferParams", Vector4(zx, zy, zx / cam_far, zy / cam_near));
    }

    void Material::SetProjectionParams(std::shared_ptr<Camera> &cam)
    {
        float cam_far = cam->GetClipFar();
        float cam_near = cam->GetClipNear();

        // x = 1 or -1 (-1 if projection is flipped)
        // y = near plane
        // z = far plane
        // w = 1/far plane
        SetVector("_ProjectionParams", Vector4(1, cam_near, cam_far, 1 / cam_far));
    }

    void Material::SetMainTexTexelSize(const std::shared_ptr<Texture> &tex)
    {
        SetVector("_MainTex_TexelSize", Vector4(1.0f / tex->GetWidth(), 1.0f / tex->GetHeight(), (float) tex->GetWidth(), (float) tex->GetHeight()));
    }
}