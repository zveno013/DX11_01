#pragma once

#include "Framework.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
 

using namespace D3D11Framework;

class BitmapFont;
class Text;

class MyRender : public Render
{
public:
	MyRender();
	bool Init(HWND hwnd);
	bool Draw();
	void Close();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

private:
	friend BitmapFont;
	friend Text;

	BitmapFont *m_font;
	Text *text1;
	Text *text2;
	Text *text3;

	XMMATRIX m_Ortho;

	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;

	//FPS
	float fps_a, fps_b, fps_time, fps_frame, fps;
	Text *textFPS;


	/// -Текстурированный куб

	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
	};

	// никогда не меняется
	struct CBNeverChanges
	{
		XMMATRIX mView;
	};

	// Меняется при изменении размера
	struct CBChangeOnResize
	{
		XMMATRIX mProjection;
	};

	// Меняется каждый кадр
	struct CBChangesEveryFrame
	{
		XMMATRIX mWorld;
		XMFLOAT4 vMeshColor;
	};

	ID3D11Texture2D*                    g_pDepthStencil = NULL;
	ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
	ID3D11VertexShader*                 g_pVertexShader = NULL;
	ID3D11PixelShader*                  g_pPixelShader = NULL;
	ID3D11InputLayout*                  g_pVertexLayout = NULL;
	ID3D11Buffer*                       g_pVertexBuffer = NULL;
	ID3D11Buffer*                       g_pIndexBuffer = NULL;
	ID3D11Buffer*                       g_pCBNeverChanges = NULL;
	ID3D11Buffer*                       g_pCBChangeOnResize = NULL;
	ID3D11Buffer*                       g_pCBChangesEveryFrame = NULL;
	ID3D11ShaderResourceView*           g_pTextureRV = NULL;
	ID3D11SamplerState*                 g_pSamplerLinear = NULL;
	XMMATRIX                            g_World;
	XMMATRIX                            g_View;
	XMMATRIX                            g_Projection;
	XMFLOAT4                            g_vMeshColor;

	///--- Текстурированный куб
};