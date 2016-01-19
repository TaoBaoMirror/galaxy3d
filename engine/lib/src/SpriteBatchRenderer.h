#ifndef __SpriteBatchRenderer_h__
#define __SpriteBatchRenderer_h__

#include "Renderer.h"
#include "SpriteNode.h"
#include <list>

namespace Galaxy3D
{
	//	�������ƾ��飬����λ����sprite node���ƣ�����rendererӰ��
	//	��������ӿյ�sprite��Ҫ�ÿ�sprite����ʹ��RemoveSprite
	//	�޸�sprite���Ժ�Ҫ����UpdateSprites���¶���buffer
	//	SetColor��Ӱ������������sprite
	class SpriteBatchRenderer : public Renderer
	{
	public:
		SpriteBatchRenderer();
		virtual ~SpriteBatchRenderer();
		void AddSprite(const std::shared_ptr<SpriteNode> &sprite);
		void RemoveSprite(const std::shared_ptr<SpriteNode> &sprite);
		void SetColor(const Color &color) {m_color = color;}
		void UpdateSprites();//���������sprite node����Ҫ�ֶ�����

	protected:
		virtual void Render(int material_index);
		
	private:
		std::list<std::shared_ptr<SpriteNode>> m_sprites;
		std::list<std::shared_ptr<SpriteNode>> m_sprites_cache;
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		BufferObject m_vertex_buffer;
        BufferObject m_index_buffer;

		void CreateVertexBuffer();
		void UpdateVertexBuffer();
		void CreateIndexBuffer();
		void Release();
	};
}

#endif