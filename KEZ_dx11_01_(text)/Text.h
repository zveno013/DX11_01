#pragma once

#include <string>
#include "BitmapFont.h"

class Text
{
public:
	Text(MyRender *render, BitmapFont *font);

	bool Init(const std::wstring &text, int scrW, int scrH);
	void Render(float r, float g, float b, float x, float y);
	void Close();

private:
	bool m_InitBuffers(const std::wstring &text, int scrW, int scrH);
	void m_RenderBuffers();

	MyRender *m_render;

	BitmapFont *m_font;
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;
	int m_numindex;
};