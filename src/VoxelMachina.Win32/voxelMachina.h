#pragma once

#include "application.h"

class VoxelMachinaApp : public IGameApp
{
	virtual void Startup(void) override;

	virtual void Cleanup(void) override;

	virtual bool IsDone(void) override;

	virtual void Update(float deltaT) override;

	virtual void RenderScene(void) override;

	virtual void RenderUI(void) override;
};