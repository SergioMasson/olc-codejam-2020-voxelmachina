#include "pch.h"
#include "voxelMachina.h"
#include "LightVoxelMachina.h"

INT __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	IGameApp* app = new LightVoxelMachinaApp();
	RunApplication(*app, hInstance, L"VoxelMachina");
	delete app;
	return 0;
}