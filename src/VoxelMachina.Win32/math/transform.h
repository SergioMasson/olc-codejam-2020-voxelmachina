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

	public:
		Transform() : m_rotation(), m_translation(0, 0, 0) {}
		Transform(Quaternion rotate) : m_rotation(rotate), m_translation(0, 0, 0) {}
		Transform(Vector3 translate) : m_rotation(), m_translation(translate) {}
		Transform(Quaternion rotate, Vector3 translate) : m_rotation(rotate), m_translation(translate) {}
		Transform(const Matrix3& mat) : m_rotation(mat), m_translation(0, 0, 0) {}
		Transform(const Matrix3& mat, Vector3 translate) : m_rotation(mat), m_translation(translate) {}
		explicit Transform(const DirectX::XMMATRIX& mat) { *this = Transform(Matrix3(mat), Vector3(mat.r[3])); }

		void SetRotation(Quaternion q) { m_rotation = q; }
		void SetTranslation(Vector3 v) { m_translation = v; }

		Quaternion GetRotation() const { return m_rotation; }
		Vector3 GetTranslation() const { return m_translation; }

		static Transform MakeXRotation(float angle) { return Transform(Quaternion(Vector3(1, 0, 0), angle)); }
		static Transform MakeYRotation(float angle) { return Transform(Quaternion(Vector3(0, 1, 0), angle)); }
		static Transform MakeZRotation(float angle) { return Transform(Quaternion(Vector3(0, 0, 1), angle)); }
		static Transform MakeTranslation(Vector3 translate) { return Transform(translate); }

		Vector3 operator* (Vector3 vec) const { return (m_rotation * vec) + m_translation; }

		Transform operator* (const Transform& xform) const {
			return Transform(m_rotation * xform.m_rotation, m_rotation * xform.m_translation + m_translation);
		}

		Transform operator~ () const {
			Quaternion invertedRotation = m_rotation.Inverse();
			return Transform(invertedRotation, invertedRotation * -m_translation);
		}
	};
}