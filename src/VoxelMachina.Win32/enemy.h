#pragma once

#include "graphics/meshRenderer.h"
#include "gameObject.h"

class Enemy
{
public:
	Enemy(GameObject* meshRenderer, graphics::Texture2D* texture) :
		m_meshRenderer{ meshRenderer },
		m_detectedTexture{ texture },
		m_isDetected{ false }
	{}

	void SetDetected();
	bool IsDetected() { return m_isDetected; }
	math::Vector3 GetPosition() { return m_meshRenderer->GetPosition(); }
	math::BoudingBox WBoudingBox() const { return m_meshRenderer->GetMeshRenderer()->WBoudingBox(); }

private:
	GameObject* m_meshRenderer;
	graphics::Texture2D* m_detectedTexture;

	bool m_isDetected;
};