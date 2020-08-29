#include "pch.h"
#include "voxelMachina.h"

INT __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	IGameApp* app = new VoxelMachinaApp();
	RunApplication(*app, hInstance, L"VoxelMachina");
	delete app;
	return 0;
}