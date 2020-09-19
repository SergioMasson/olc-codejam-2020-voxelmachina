//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard
//
#pragma once
#include "vector3.h"
#include "matrix3.h"

namespace math
{
	// This transform strictly prohibits non-uniform scale.  Scale itself is barely tolerated.
	__declspec(align(16)) struct Transform
	{
	private:
		Quaternion m_rotation;
		Vector3 m_translation;
		Vector3 m_scale;

	public:
		INLINE Transform() : m_rotation(), m_translation(0, 0, 0), m_scale(1, 1, 1) {}
		INLINE Transform(Quaternion rotate) : m_rotation(rotate), m_translation(0, 0, 0), m_scale(1, 1, 1) {}
		INLINE Transform(Vector3 translate) : m_rotation(), m_translation(translate), m_scale(1, 1, 1) {}
		INLINE Transform(Quaternion rotate, Vector3 translate) : m_rotation(rotate), m_translation(translate), m_scale(1, 1, 1) {}
		INLINE Transform(Quaternion rotate, Vector3 translate, Vector3 scale) : m_rotation(rotate), m_translation(translate), m_scale(scale) {}
		INLINE Transform(const Matrix3& mat) : m_rotation(mat), m_translation(0, 0, 0) {}
		INLINE Transform(const Matrix3& mat, Vector3 translate) : m_rotation(mat), m_translation(translate) {}

		INLINE explicit Transform(const DirectX::XMMATRIX& mat)
		{
			*this = Transform(Matrix3(mat), Vector3(mat.r[3]));
		}

		INLINE void SetRotation(Quaternion q) { m_rotation = q; }
		INLINE void SetTranslation(Vector3 v)
		{
			m_translation = v;
		}
		INLINE void SetScale(Vector3 scale) { m_scale = scale; }

		INLINE Quaternion GetRotation() const { return m_rotation; }
		INLINE Vector3 GetTranslation() const { return m_translation; }
		INLINE Vector3 GetScale() const { return m_scale; }

		INLINE const Vector3 GetRightVec() const { return Matrix3{ m_rotation }.GetX(); }
		INLINE const Vector3 GetUpVec() const { return Matrix3{ m_rotation }.GetY(); }
		INLINE const Vector3 GetForwardVec() const { return -Matrix3{ m_rotation }.GetZ(); };

		INLINE static Transform MakeXRotation(float angle) { return Transform(Quaternion(Vector3(1, 0, 0), angle)); }
		INLINE static Transform MakeYRotation(float angle) { return Transform(Quaternion(Vector3(0, 1, 0), angle)); }
		INLINE static Transform MakeZRotation(float angle) { return Transform(Quaternion(Vector3(0, 0, 1), angle)); }
		INLINE static Transform MakeTranslation(Vector3 translate) { return Transform(translate); }

		INLINE Vector3 operator* (Vector3 vec) const { return (m_rotation * vec) + m_translation; }

		INLINE Transform operator* (const Transform& xform) const
		{
			return Transform(m_rotation * xform.m_rotation, m_rotation * xform.m_translation + m_translation, xform.m_scale * m_scale);
		}

		INLINE Transform operator~ () const {
			Quaternion invertedRotation = m_rotation.Inverse();
			return Transform(invertedRotation, invertedRotation * -m_translation);
		}
	};
}