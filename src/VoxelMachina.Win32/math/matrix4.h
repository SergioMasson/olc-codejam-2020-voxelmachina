#pragma once
#include "transform.h"

namespace math
{
	__declspec(align(16)) class Matrix4
	{
	public:
		Matrix4() { m_mat = DirectX::XMMatrixIdentity(); }
		Matrix4(Vector3 x, Vector3 y, Vector3 z, Vector3 w)
		{
			m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
			m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
		}
		Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
		Matrix4(const Matrix4& mat) { m_mat = mat.m_mat; }
		Matrix4(const Matrix3& mat)
		{
			m_mat.r[0] = SetWToZero(mat.GetX());
			m_mat.r[1] = SetWToZero(mat.GetY());
			m_mat.r[2] = SetWToZero(mat.GetZ());
			m_mat.r[3] = CreateWUnitVector();
		}
		Matrix4(const Matrix3& xyz, Vector3 w)
		{
			m_mat.r[0] = SetWToZero(xyz.GetX());
			m_mat.r[1] = SetWToZero(xyz.GetY());
			m_mat.r[2] = SetWToZero(xyz.GetZ());
			m_mat.r[3] = SetWToOne(w);
		}
		Matrix4(const Transform& xform) { *this = Matrix4(Matrix3(xform.GetRotation()), xform.GetTranslation()); }
		explicit Matrix4(const DirectX::XMMATRIX& mat) { m_mat = mat; }

		const Matrix3& Get3x3() const { return (const Matrix3&)*this; }

		Vector4 GetX() const { return Vector4(m_mat.r[0]); }
		Vector4 GetY() const { return Vector4(m_mat.r[1]); }
		Vector4 GetZ() const { return Vector4(m_mat.r[2]); }
		Vector4 GetW() const { return Vector4(m_mat.r[3]); }

		void SetX(Vector4 x) { m_mat.r[0] = x; }
		void SetY(Vector4 y) { m_mat.r[1] = y; }
		void SetZ(Vector4 z) { m_mat.r[2] = z; }
		void SetW(Vector4 w) { m_mat.r[3] = w; }

		operator DirectX::XMMATRIX() const { return m_mat; }

		Vector4 operator* (Vector3 vec) const { return Vector4(DirectX::XMVector3Transform(vec, m_mat)); }
		Vector4 operator* (Vector4 vec) const { return Vector4(DirectX::XMVector4Transform(vec, m_mat)); }
		Matrix4 operator* (const Matrix4& mat) const { return Matrix4(DirectX::XMMatrixMultiply(mat, m_mat)); }

		static  Matrix4 MakeScale(float scale) { return Matrix4(DirectX::XMMatrixScaling(scale, scale, scale)); }
		static  Matrix4 MakeScale(Vector3 scale) { return Matrix4(DirectX::XMMatrixScalingFromVector(scale)); }

	private:
		DirectX::XMMATRIX m_mat;
	};
}