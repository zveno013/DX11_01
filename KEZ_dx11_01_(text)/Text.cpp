
#include "stdafx.h"
#include "macros.h"
#include "Text.h"

Text::Text(MyRender *render, BitmapFont *font)
{
	m_render = render;
	m_font = font;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_numindex = 0;
}

bool Text::Init(const std::wstring &text, int screenWidth, int screenHeight)
{
	if (!m_InitBuffers(text, screenWidth, screenHeight))
		return false;

	return true;
}

bool Text::m_InitBuffers(const std::wstring &text, int screenWidth, int screenHeight)
{
	int vertnum = text.size() * 6;
	VertexFont *vertex = new VertexFont[vertnum];
	if (!vertex)
		return false;

	m_font->BuildVertexArray(vertex, text.c_str(), screenWidth, screenHeight);

	m_numindex = text.size() * 6;
	unsigned long *indices = new unsigned long[m_numindex];

	for (int i = 0; i <m_numindex; i++)
		indices[i] = i;

	HRESULT result;
	D3D11_BUFFER_DESC BufferDesc;
	D3D11_SUBRESOURCE_DATA Data;

	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.ByteWidth = sizeof(VertexFont) * vertnum;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;

	Data.pSysMem = vertex;
	Data.SysMemPitch = 0;
	Data.SysMemSlicePitch = 0;

	result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, &Data, &m_vertexBuffer);
	if (FAILED(result))
		return false;

	BufferDesc.ByteWidth = sizeof(unsigned long) * m_numindex;
	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	Data.pSysMem = indices;

	result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, &Data, &m_indexBuffer);
	if (FAILED(result))
		return false;

	_DELETE_ARRAY(vertex);
	_DELETE_ARRAY(indices);

	return true;
}

void Text::Render(float r, float g, float b, float x, float y)
{
	m_RenderBuffers();
	m_font->Render(m_numindex, r/255, g/255, b/255, x, y);
}

void Text::Close()
{
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_indexBuffer);
}

void Text::m_RenderBuffers()
{
	unsigned int stride = sizeof(VertexFont);
	unsigned int offset = 0;
	m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}