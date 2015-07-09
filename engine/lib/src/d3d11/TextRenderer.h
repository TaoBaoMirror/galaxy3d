#ifndef __TextRenderer_h__
#define __TextRenderer_h__

#include "Renderer.h"
#include "Label.h"

namespace Galaxy3D
{
	class TextRenderer : public Renderer
	{
	public:
		TextRenderer();
		virtual ~TextRenderer();
		void SetLabel(const std::shared_ptr<Label> &label);
		std::shared_ptr<Label> GetLabel() const {return m_label;}
		void UpdateLabel();//����label����Ҫ�ֶ�����

	protected:
		virtual void Render();

	private:
		std::shared_ptr<Label> m_label;
		ID3D11Buffer *m_vertex_buffer;
		ID3D11Buffer *m_index_buffer;
		int m_vertex_count;

		void CreateVertexBuffer();
		void UpdateVertexBuffer();
		void CreateIndexBuffer();
		void Release();
	};
}

#endif