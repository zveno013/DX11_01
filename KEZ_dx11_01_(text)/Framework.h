#pragma once

#include "InputListener.h"
#include "Render.h"
#include "StaticMesh.h"
#include "Window.h"
#include "Render.h"
#include "InputMgr.h"
#include "Log.h"
#include <vector>

namespace D3D11Framework
{
	//------------------------------------------------------------------

	struct FrameworkDesc
	{
		DescWindow wnd;
		Render *render;
		//std::vector< Render* > *render;
	};

	class Framework
	{
	public:
		Framework();
		~Framework();

		bool Init(const FrameworkDesc &desc);
		void Run();
		void Close();

		void AddInputListener(InputListener *listener);
	protected:
		bool m_frame();

		FrameworkDesc m_desc;
		Window *m_wnd;
		Render *m_render;
		InputMgr *m_input;
		Log m_log;
		bool m_init;		// ���� ���� ����������������
	};

	//------------------------------------------------------------------
}