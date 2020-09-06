#pragma once

#include "graphics/meshRenderer.h"

class Enemy
{
public:
	Enemy(graphics::MeshRenderer* meshRenderer, graphics::Texture2D* texture) :
		m_meshRenderer{ meshRenderer },
		m_detectedTexture{ texture }
	{}

	void SetDetected();
	bool IsDetected() { return m_isDetected; }
	math::Vector3 GetPosition() { return m_meshRenderer->GetPosition(); }

private:
	graphics::MeshRenderer* m_meshRenderer;
	graphics::Texture2D* m_detectedTexture;

	bool m_isDetected;
};