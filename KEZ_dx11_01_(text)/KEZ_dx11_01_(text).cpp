// KEZ_dx11_01_(text).cpp: определяет точку входа для консольного приложения.
//

#include "macros.h"
#include "stdafx.h"
#include "Framework.h"
#include "InputListener.h"
#include "Render.h"
#include "StaticMesh.h"
#include "MyRender.h"
#include <vector>


int _tmain(int argc, _TCHAR* argv[])
{
	Framework framework;

	//std::vector< Render* > render;
	MyRender *render = new MyRender();
	//render.push_back(k_render);
	FrameworkDesc desc;
	desc.render = render;

	framework.Init(desc);

	framework.Run();

	framework.Close();
	
	return 0;
}

