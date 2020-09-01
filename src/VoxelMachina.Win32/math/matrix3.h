#pragma once
#include "quaternion.h"

namespace math
{
	// Represents a 3x3 matrix while occuping a 4x4 memory footprint.  The unused row and column are undefined but implicitly
	// (0, 0, 0, 1).  Constructing a Matrix4 will make those values explicit.
	__declspec(align(16)) class Matrix3
	{
	public:
		Matrix3()
		{
			m_mat[0] = math::Vector3(1, 0, 0);
			m_mat[1] = math::Vector3(0, 1, 0);
			m_mat[2] = math::Vector3(0, 0, 1);
		}
		Matrix3(Vector3 x, Vector3 y, Vector3 z) { m_mat[0] = x; m_mat[1] = y; m_mat[2] = z; }
		Matrix3(const Matrix3& m) { m_mat[0] = m.m_mat[0]; m_mat[1] = m.m_mat[1]; m_mat[2] = m.m_mat[2]; }
		Matrix3(Quaternion q) { *this = Matrix3(DirectX::XMMatrixRotationQuaternion(q)); }
		explicit Matrix3(const DirectX::XMMATRIX& m) { m_mat[0] = Vector3(m.r[0]); m_mat[1] = Vector3(m.r[1]); m_mat[2] = Vector3(m.r[2]); }

		void SetX(Vector3 x) { m_mat[0] = x; }
		void SetY(Vector3 y) { m_mat[1] = y; }
		void SetZ(Vector3 z) { m_mat[2] = z; }

		Vector3 GetX() const { return m_mat[0]; }
		Vector3 GetY() const { return m_mat[1]; }
		Vector3 GetZ() const { return m_mat[2]; }

		static Matrix3 Identity() { return Matrix3(Vector3{ 1, 0, 0 }, Vector3{ 0, 1, 0 }, Vector3{ 0, 0, 1 }); };
		static Matrix3 MakeXRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationX(angle)); }
		static Matrix3 MakeYRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationY(angle)); }
		static Matrix3 MakeZRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationZ(angle)); }
		static Matrix3 MakeScale(float scale) { return Matrix3(DirectX::XMMatrixScaling(scale, scale, scale)); }
		static Matrix3 MakeScale(float sx, float sy, float sz) { return Matrix3(DirectX::XMMatrixScaling(sx, sy, sz)); }
		static Matrix3 MakeScale(Vector3 scale) { return Matrix3(DirectX::XMMatrixScalingFromVector(scale)); }

		operator DirectX::XMMATRIX() const { return (const DirectX::XMMATRIX&)m_mat; }

		Vector3 operator* (Vector3 vec) const { return Vector3(DirectX::XMVector3TransformNormal(vec, *this)); }
		Matrix3 operator* (const Matrix3& mat) const { return Matrix3(*this * mat.GetX(), *this * mat.GetY(), *this * mat.GetZ()); }

	private:
		Vector3 m_mat[3];
	};
}