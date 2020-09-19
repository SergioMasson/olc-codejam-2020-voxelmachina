#pragma once

#include "math/transform.h"

typedef uint32_t ComponentType;

class Component;

namespace graphics
{
	class MeshRenderer;
	class Material;
	struct MeshData;
}

class GameObject
{
public:
	GameObject(math::Vector3 position = math::Vector3(0, 0, 0), math::Quaternion rotation = math::Quaternion(0, 0, 0), GameObject* parent = nullptr);
	//template<typename T, typename ... Param> T* const& AddComponent(const Param& ... params)
	//{
	//	T* component = new T(this, params);
	//	uint32_t hash = component->GetHash();
	//	m_components.insert({ hash, component });
	//	return component;
	//}
	//template<class T> T* GetComponent(ComponentType type)
	//{
	//	auto find = m_components.find(type);

	//	if (find != m_components.end())
	//		return &find;

	//	return nullptr;
	//}
	bool IsActive() { return m_isActive; }
	void SetActive(bool active) { m_isActive = active; }

	math::Vector3 GetLocalPosition() const;
	math::Vector3 GetPosition() const;

	math::Quaternion GetLocalRotation() const;
	math::Quaternion GetRotation() const;

	math::Transform GetTransform() const;
	math::Transform GetLocalTransform() const;

	void SetParent(GameObject* parent);
	void SetPosition(math::Vector3 position);
	void SetRotation(math::Quaternion rotation);
	void SetScale(math::Vector3 scale);

	graphics::MeshRenderer* AddMeshRenderer(graphics::MeshData* data, graphics::Material material);

	graphics::MeshRenderer* GetMeshRenderer()
	{
		return m_meshRenderer;
	}

	bool HasRenderer() {
		return m_meshRenderer != nullptr;
	}

private:
	math::Transform m_transform;
	graphics::MeshRenderer* m_meshRenderer;
	GameObject* m_parent;
	std::map<ComponentType, Component*> m_components;
	bool m_isActive;
};

class Component
{
public:
	virtual uint32_t GetHash(void) = 0;
protected:
	Component(GameObject* gameobject)
	{
		m_gameObject = gameobject;
		m_active = true;
	}
protected:
	GameObject* m_gameObject;
	bool m_active;
	friend GameObject;
};

class BehaviurComponent : public Component
{
public:
	virtual void Start(void) = 0;
	virtual void Update(void) = 0;
};