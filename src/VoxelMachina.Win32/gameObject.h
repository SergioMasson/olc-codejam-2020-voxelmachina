#pragma once

#include "math/transform.h"

typedef uint32_t ComponentType;

class Component;
class BehaviourComponent;

extern std::vector<BehaviourComponent*> g_activeBehaviours;

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

	template<typename T, typename ...Param> T* AddComponent(Param... params)
	{
		T* component = new T(this, std::forward<Param>(params)...);
		m_components.push_back(component);
		return component;
	}

	template<class T> T* GetComponent()
	{
		for (auto component : m_components)
		{
			T* target = dynamic_cast<T*>(component);

			if (target != nullptr)
				return target;
		}

		return nullptr;
	}

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
	std::vector<Component*> m_components;
	bool m_isActive;
};

class Component
{
protected:
	Component(GameObject* gameObject) : m_gameObject{ gameObject }
	{
		m_active = true;
	}

protected:
	GameObject* m_gameObject;
	bool m_active;
	friend GameObject;
};

class BehaviourComponent : public Component
{
public:
	BehaviourComponent(GameObject* gameObject) : Component(gameObject)
	{
		g_activeBehaviours.push_back(this);
	};

	virtual void Update(float daltaT) = 0;
};