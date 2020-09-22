#include "pch.h"
#include "gameObject.h"
#include "graphics/meshRenderer.h"

std::set<BehaviourComponent*> g_activeBehaviours;
std::set<GameObject*> g_activeGameObjects;

GameObject::GameObject(math::Vector3 position, math::Quaternion rotation, GameObject* parent) :
	m_transform{ rotation, position },
	m_meshRenderer{ nullptr },
	m_parent{ parent },
	m_isActive{ true }
{
	g_activeGameObjects.insert(this);
}

GameObject::~GameObject()
{
	for (Component* component : m_components)
		delete component;

	if (m_meshRenderer != nullptr)
		delete m_meshRenderer;

	g_activeGameObjects.erase(this);
}

math::Vector3 GameObject::GetLocalPosition() const
{
	return m_transform.GetTranslation();
}

math::Vector3 GameObject::GetPosition() const
{
	if (m_parent != nullptr)
		return m_parent->GetTransform() * m_transform.GetTranslation();

	return m_transform.GetTranslation();
}

math::Quaternion GameObject::GetLocalRotation() const
{
	return m_transform.GetRotation();
}

math::Quaternion GameObject::GetRotation() const
{
	if (m_parent != nullptr)
		return m_parent->GetRotation() * m_transform.GetRotation();

	return m_transform.GetRotation();
}

math::Transform GameObject::GetLocalTransform() const
{
	return m_transform;
}

void GameObject::SetParent(GameObject* parent)
{
	m_parent = parent;
}

void GameObject::SetPosition(math::Vector3 position)
{
	m_transform.SetTranslation(position);
}

void GameObject::SetRotation(math::Quaternion rotation)
{
	m_transform.SetRotation(rotation);
}

void GameObject::SetScale(math::Vector3 scale)
{
	m_transform.SetScale(scale);
}

graphics::MeshRenderer* GameObject::AddMeshRenderer(graphics::MeshData* data, graphics::Material material)
{
	m_meshRenderer = new graphics::MeshRenderer(this, data, material);
	return m_meshRenderer;
}

math::Transform GameObject::GetTransform() const
{
	if (m_parent != nullptr)
		return m_parent->GetTransform() * m_transform;

	return m_transform;
}