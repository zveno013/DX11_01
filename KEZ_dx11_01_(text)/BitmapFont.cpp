#pragma once
#include "stdafx.h"
#include "BitmapFont.h"

#include <fstream>
#include <string>
#include <sstream>

using namespace std;

inline wchar_t* CharToWChar(char *mbString)
{
	int len = 0;
	len = (int)strlen(mbString) + 1;
	wchar_t *ucString = new wchar_t[len];
	mbstowcs(ucString, mbString, len);
	return ucString;
}

BitmapFont::BitmapFont(MyRender *render)
{
	m_render = render;
	m_texture = nullptr;
	m_constantBuffer = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_sampleState = nullptr;
	m_pixelBuffer = nullptr;
	m_WidthTex = 0;
	m_HeightTex = 0;
}

bool BitmapFont::Init(char *fontFilename)
{
	if (!m_parse(fontFilename))
		return false;

	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(m_render->m_pd3dDevice, m_file.c_str(), NULL, NULL, &m_texture, NULL);
	if (FAILED(hr))
		return false;

	if (!m_InitShader(L"BitmapFont.vs", L"BitmapFont.ps"))
		return false;

	return true;
}

bool BitmapFont::m_parse(char *fontFilename)
{
	ifstream fin;
	fin.open(fontFilename);
	if (fin.fail())
		return false;

	string Line;
	string Read, Key, Value;
	size_t i;
	while (!fin.eof())
	{
		std::stringstream LineStream;
		std::getline(fin, Line);
		LineStream << Line;

		LineStream >> Read;
		if (Read == "common")
		{
			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				Converter << Value;
				if (Key == "scaleW")
					Converter >> m_WidthTex;
				else if (Key == "scaleH")
					Converter >> m_HeightTex;
			}
		}
		else if (Read == "page")
		{
			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				std::string str;
				Converter << Value;
				if (Key == "file")
				{
					Converter >> str;
					wchar_t *name = CharToWChar((char*)str.substr(1, Value.length() - 2).c_str());
					m_file = name;
					_DELETE_ARRAY(name);
				}
			}
		}
		else if (Read == "char")
		{
			unsigned short CharID = 0;
			CharDesc chard;

			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				Converter << Value;
				if (Key == "id")
					Converter >> CharID;
				else if (Key == "x")
					Converter >> chard.srcX;
				else if (Key == "y")
					Converter >> chard.srcY;
				else if (Key == "width")
					Converter >> chard.srcW;
				else if (Key == "height")
					Converter >> chard.srcH;
				else if (Key == "xoffset")
					Converter >> chard.xOff;
				else if (Key == "yoffset")
					Converter >> chard.yOff;
				else if (Key == "xadvance")
					Converter >> chard.xAdv;
			}
			m_Chars.insert(std::pair<int, CharDesc>(CharID, chard));
		}
	}

	fin.close();

	return true;
}

bool BitmapFont::m_InitShader(wchar_t *vsFilename, wchar_t *psFilename)
{
	ID3DBlob *vertexShaderBuffer = nullptr;
	HRESULT hr = m_render->m_compileshaderfromfile(vsFilename, "VS", "vs_4_0", &vertexShaderBuffer);
	if (FAILED(hr))
		return false;

	ID3DBlob *pixelShaderBuffer = nullptr;
	HRESULT result = m_render->m_compileshaderfromfile(psFilename, "PS", "ps_4_0", &pixelShaderBuffer);
	if (FAILED(hr))
		return false;

	result = m_render->m_pd3dDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	result = m_render->m_pd3dDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = m_render->m_pd3dDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	_RELEASE(vertexShaderBuffer);
	_RELEASE(pixelShaderBuffer);

	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.ByteWidth = sizeof(ConstantBuffer);
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;

	result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, NULL, &m_constantBuffer);
	if (FAILED(result))
		return false;

	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.ByteWidth = sizeof(PixelBufferType);
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;

	result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, NULL, &m_pixelBuffer);
	if (FAILED(result))
		return false;

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_render->m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
		return false;

	return true;
}

void BitmapFont::BuildVertexArray(VertexFont *vertices, const wchar_t *sentence, int screenWidth, int screenHeight)
{
	int numLetters = (int)wcslen(sentence);

	float drawX = (float)-screenWidth / 2;
	float drawY = (float)screenHeight / 2;

	int index = 0;
	for (int i = 0; i<numLetters; i++)
	{
		float CharX = m_Chars[sentence[i]].srcX;
		float CharY = m_Chars[sentence[i]].srcY;
		float Width = m_Chars[sentence[i]].srcW;
		float Height = m_Chars[sentence[i]].srcH;
		float OffsetX = m_Chars[sentence[i]].xOff;
		float OffsetY = m_Chars[sentence[i]].yOff;

		float left = drawX + OffsetX;
		float right = left + Width;
		float top = drawY - OffsetY;
		float bottom = top - Height;
		float lefttex = CharX / m_WidthTex;
		float righttex = (CharX + Width) / m_WidthTex;
		float toptex = CharY / m_HeightTex;
		float bottomtex = (CharY + Height) / m_HeightTex;

		vertices[index].pos = XMFLOAT3(left, top, 0.0f);
		vertices[index].tex = XMFLOAT2(lefttex, toptex);
		index++;
		vertices[index].pos = XMFLOAT3(right, bottom, 0.0f);
		vertices[index].tex = XMFLOAT2(righttex, bottomtex);
		index++;
		vertices[index].pos = XMFLOAT3(left, bottom, 0.0f);
		vertices[index].tex = XMFLOAT2(lefttex, bottomtex);
		index++;
		vertices[index].pos = XMFLOAT3(left, top, 0.0f);
		vertices[index].tex = XMFLOAT2(lefttex, toptex);
		index++;
		vertices[index].pos = XMFLOAT3(right, top, 0.0f);
		vertices[index].tex = XMFLOAT2(righttex, toptex);
		index++;
		vertices[index].pos = XMFLOAT3(right, bottom, 0.0f);
		vertices[index].tex = XMFLOAT2(righttex, bottomtex);
		index++;

		drawX += m_Chars[sentence[i]].xAdv;
	}
}

void BitmapFont::Render(unsigned int index, float r, float g, float b, float x, float y)
{
	m_SetShaderParameters(r, g, b, x, y);
	m_RenderShader(index);
}

void BitmapFont::m_SetShaderParameters(float r, float g, float b, float x, float y)
{
	XMMATRIX objmatrix = XMMatrixTranslation(x, -y, 0);
	XMMATRIX wvp = objmatrix*m_render->m_Ortho;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(wvp);
	m_render->m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);

	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	m_render->m_pImmediateContext->PSSetShaderResources(0, 1, &m_texture);

	PixelBufferType pb;
	pb.pixelColor = XMFLOAT4(r, g, b, 1.0f);
	m_render->m_pImmediateContext->UpdateSubresource(m_pixelBuffer, 0, NULL, &pb, 0, 0);

	m_render->m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pixelBuffer);
}

void BitmapFont::m_RenderShader(unsigned int index)
{
	m_render->m_pImmediateContext->IASetInputLayout(m_layout);
	m_render->m_pImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_render->m_pImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
	m_render->m_pImmediateContext->PSSetSamplers(0, 1, &m_sampleState);
	m_render->m_pImmediateContext->DrawIndexed(index, 0, 0);
}

void BitmapFont::Close()
{
	_RELEASE(m_constantBuffer);
	_RELEASE(m_pixelBuffer);
	_RELEASE(m_vertexShader);
	_RELEASE(m_pixelShader);
	_RELEASE(m_layout);
	_RELEASE(m_sampleState);
	_RELEASE(m_texture);
}