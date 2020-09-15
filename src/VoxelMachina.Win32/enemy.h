#pragma once

#include "graphics/meshRenderer.h"

class Enemy
{
public:
	Enemy(graphics::MeshRenderer* meshRenderer, graphics::Texture2D* texture) :
		m_meshRenderer{ meshRenderer },
		m_detectedTexture{ texture },
		m_isDetected{ false }
	{}

	void SetDetected();
	bool IsDetected() { return m_isDetected; }
	math::Vector3 GetPosition() { return m_meshRenderer->GetPosition(); }
	math::BoudingBox WBoudingBox() const { return m_meshRenderer->GetWorldBoudingSphere(); }

private:
	graphics::MeshRenderer* m_meshRenderer;
	graphics::Texture2D* m_detectedTexture;

	bool m_isDetected;
};