#include "pch.h"
#include "enemy.h"
#include "colors.h"

std::set<Enemy*> g_activeEnemies;

void Enemy::SetDetected()
{
	m_isDetected = true;
	//m_meshRenderer->SetAlbedoTexture(m_detectedTexture);
	m_gameObject->GetMeshRenderer()->SetEmission(Color::MediumVioletRed);
}