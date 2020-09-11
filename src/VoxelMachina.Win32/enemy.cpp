#include "pch.h"
#include "enemy.h"
#include "colors.h"

void Enemy::SetDetected()
{
	m_isDetected = true;
	//m_meshRenderer->SetAlbedoTexture(m_detectedTexture);
	m_meshRenderer->SetEmission(Color::MediumVioletRed);
}