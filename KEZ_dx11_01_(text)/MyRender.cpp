#include "stdafx.h"
#include "MyRender.h"
#include "BitmapFont.h"
#include "Text.h"
#include <windef.h>
#include "macros.h"
//#include <string>
#include <mmsystem.h>                 //timeGetTime()

MyRender::MyRender()
{
	m_font = nullptr;
	text1 = nullptr;
	text2 = nullptr;
	text3 = nullptr;
}

bool MyRender::Init(HWND hwnd)
{
	D3D11_BLEND_DESC bSDesc;
	ZeroMemory(&bSDesc, sizeof(D3D11_BLEND_DESC));
	bSDesc.RenderTarget[0].BlendEnable = TRUE;
	bSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	HRESULT result = m_pd3dDevice->CreateBlendState(&bSDesc, &m_alphaEnableBlendingState);
	if (FAILED(result))
		return false;

	bSDesc.RenderTarget[0].BlendEnable = FALSE;
	result = m_pd3dDevice->CreateBlendState(&bSDesc, &m_alphaDisableBlendingState);
	if (FAILED(result))
		return false;

	m_Ortho = XMMatrixOrthographicLH(800.0f, 600.0f, 0.0f, 1000.0f);

	m_font = new BitmapFont(this);
	if (!m_font->Init("font.fnt"))
		return false;

	text1 = new Text(this, m_font);
	text1->Init(L"Hello", 800, 600);

	text2 = new Text(this, m_font);
	text2->Init(L"World", 800, 600);

	text3 = new Text(this, m_font);
	text3->Init(L"Привет Мир", 800, 600);


	//FPS
	fps_a = 0;
	fps_b = 0;
	fps_time = 0;
	fps_frame = 0;
	textFPS = new Text(this, m_font);
	textFPS->Init(L"FPS", 400, 300);


	/// -Текстурированный куб

	XMFLOAT4                            g_vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);
	HRESULT hr = S_OK;
	//Получаю клиентские координаты окна
	
	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = m_compileshaderfromfile(L"Tutorial07.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	m_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = m_compileshaderfromfile(L"Tutorial07.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Create index buffer
	// Create vertex buffer
	WORD indices[] =
	{
		3, 1, 0,
		2, 1, 3,

		6, 4, 5,
		7, 4, 6,

		11, 9, 8,
		10, 9, 11,

		14, 12, 13,
		15, 12, 14,

		19, 17, 16,
		18, 17, 19,

		22, 20, 21,
		23, 20, 22
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	m_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffers
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBNeverChanges);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBNeverChanges);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangeOnResize);
	hr = m_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBChangeOnResize);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	hr = m_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBChangesEveryFrame);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, L"seafloor.dds", NULL, NULL, &g_pTextureRV, NULL);
	if (FAILED(hr))
		return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	CBNeverChanges cbNeverChanges;
	cbNeverChanges.mView = XMMatrixTranspose(g_View);
	m_pImmediateContext->UpdateSubresource(g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

	CBChangeOnResize cbChangesOnResize;
	cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);
	m_pImmediateContext->UpdateSubresource(g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0);
	/// ---Текстурированный куб
	return true;
}

bool MyRender::Draw()
{
	//Log::Get()->Debug("Draw");
	TurnZBufferOff();
	TurnOnAlphaBlending();

	text1->Render(100.0, 100.0, 0.0, 100, 100);
	text2->Render(100.0, 0.0, 100.0, 290, 100);
	text3->Render(0.0, 100.0, 100.0, 100, 180);

	//-FPS
	fps_frame++;
	fps_time = (float)timeGetTime(); //#include <mmsystem.h>
	float fps_z = (fps_time - fps_b)*0.001f;
	fps_a =+ fps_z;
	fps_b = fps_time;
	if (fps_a > 0.04f)
	{
		fps = fps_frame / fps_a;
		fps_a = 0.0f;
		fps_frame = 0;
		textFPS->Init(std::to_wstring(fps), 800, 600);
	}
	textFPS->Render(209, 180, 117, 50, 10);
	//---FPS

	/// -Текстурированный куб

	// Update our time
	static float t = 0.0f;
	if (m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	// Rotate cube around the origin
	g_World = XMMatrixRotationY(t);

	// Modify the color
	g_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
	g_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
	g_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

	//
	// Clear the back buffer
	//

	//В Render.cpp уже сделано

	//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	//m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);

	////
	//// Clear the depth buffer to 1.0 (max depth)
	////
	//m_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//
	// Update variables that change once per frame
	//
	CBChangesEveryFrame cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.vMeshColor = g_vMeshColor;
	m_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);

	//
	// Render the cube
	//
	m_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
	m_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
	m_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	m_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	m_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
	m_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	m_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	//В Render.cpp уже сделано

	//
	// Present our back buffer to our front buffer
	//
	//m_pSwapChain->Present(0, 0);
	/// ---Текстурированный куб
	TurnOffAlphaBlending();
	TurnZBufferOn();
	return true;
}

void MyRender::Close()
{
	_RELEASE(m_alphaEnableBlendingState);
	_RELEASE(m_alphaDisableBlendingState);
	_CLOSE(text1);
	_CLOSE(text2);
	_CLOSE(text3);
	_CLOSE(m_font);
}

void MyRender::TurnOnAlphaBlending()
{
	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	m_pImmediateContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor, 0xffffffff);
}

void MyRender::TurnOffAlphaBlending()
{
	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	m_pImmediateContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);
}