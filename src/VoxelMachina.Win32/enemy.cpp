#include "pch.h"
#include "enemy.h"

void Enemy::SetDetected()
{
	m_isDetected = true;
	m_meshRenderer->SetAlbedoTexture(m_detectedTexture);
}