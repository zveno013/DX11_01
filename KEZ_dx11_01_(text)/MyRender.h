#pragma once

#include "Framework.h"
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
};