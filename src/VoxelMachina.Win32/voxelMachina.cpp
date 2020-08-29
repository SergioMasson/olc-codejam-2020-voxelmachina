#include "pch.h"
#include "voxelMachina.h"
#include "input.h"

static bool g_isDone = false;

void VoxelMachinaApp::Startup(void)
{
}

void VoxelMachinaApp::Cleanup(void)
{
}

bool VoxelMachinaApp::IsDone(void)
{
	return g_isDone;
}

void VoxelMachinaApp::Update(float deltaT)
{
	g_isDone = Input::IsPressed(Input::KeyCode::Key_escape);
}

void VoxelMachinaApp::RenderScene(void)
{
}

void VoxelMachinaApp::RenderUI(void)
{
}