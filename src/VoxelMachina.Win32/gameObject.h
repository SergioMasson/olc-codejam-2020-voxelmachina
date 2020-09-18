#pragma once
#include "math/transform.h"

typedef uint32_t ComponentType;

class Component;

class GameObject
{
public:
	template<typename T, typename ... Param> T* const& AddComponent(const Param& ... params)
	{
		T* component = new T(this, params);
		uint32_t hash = component->GetHash();
		m_components.insert({ hash, component });
		return component;
	}
	template<class T> T const& GetComponent(ComponentType type)
	{
		auto find = m_components.find(type);

		if (find != m_components.end())
			return &find;

		return nullptr;
	}
	bool IsActive() { return m_isActive; }
	void SetActive(bool active) { m_isActive = active; }

private:
	math::Transform m_transform;
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