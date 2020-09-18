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
#include "quaternion.h"

namespace math
{
	// Represents a 3x3 matrix while occuping a 4x4 memory footprint.  The unused row and column are undefined but implicitly
	// (0, 0, 0, 1).  Constructing a Matrix4 will make those values explicit.
	__declspec(align(16)) class Matrix3
	{
	public:
		INLINE Matrix3()
		{
			m_mat[0] = math::Vector3(1, 0, 0);
			m_mat[1] = math::Vector3(0, 1, 0);
			m_mat[2] = math::Vector3(0, 0, 1);
		}
		INLINE Matrix3(Vector3 x, Vector3 y, Vector3 z) { m_mat[0] = x; m_mat[1] = y; m_mat[2] = z; }
		INLINE Matrix3(const Matrix3& m) { m_mat[0] = m.m_mat[0]; m_mat[1] = m.m_mat[1]; m_mat[2] = m.m_mat[2]; }
		INLINE Matrix3(Quaternion q) { *this = Matrix3(DirectX::XMMatrixRotationQuaternion(q)); }
		INLINE explicit Matrix3(const DirectX::XMMATRIX& m) { m_mat[0] = Vector3(m.r[0]); m_mat[1] = Vector3(m.r[1]); m_mat[2] = Vector3(m.r[2]); }

		INLINE void SetX(Vector3 x) { m_mat[0] = x; }
		INLINE void SetY(Vector3 y) { m_mat[1] = y; }
		INLINE void SetZ(Vector3 z) { m_mat[2] = z; }

		INLINE Vector3 GetX() const { return m_mat[0]; }
		INLINE Vector3 GetY() const { return m_mat[1]; }
		INLINE Vector3 GetZ() const { return m_mat[2]; }

		INLINE static Matrix3 Identity() { return Matrix3(Vector3{ 1, 0, 0 }, Vector3{ 0, 1, 0 }, Vector3{ 0, 0, 1 }); };
		INLINE static Matrix3 MakeXRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationX(angle)); }
		INLINE static Matrix3 MakeYRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationY(angle)); }
		INLINE static Matrix3 MakeZRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationZ(angle)); }
		INLINE static Matrix3 MakeScale(float scale) { return Matrix3(DirectX::XMMatrixScaling(scale, scale, scale)); }
		INLINE static Matrix3 MakeScale(float sx, float sy, float sz) { return Matrix3(DirectX::XMMatrixScaling(sx, sy, sz)); }
		INLINE static Matrix3 MakeScale(Vector3 scale) { return Matrix3(DirectX::XMMatrixScalingFromVector(scale)); }

		INLINE operator DirectX::XMMATRIX() const { return (const DirectX::XMMATRIX&)m_mat; }
		INLINE operator DirectX::XMFLOAT3X3() const
		{
			DirectX::XMFLOAT3X3 result;
			DirectX::XMStoreFloat3x3(&result, (const DirectX::XMMATRIX&)m_mat);
			return result;
		}

		INLINE Vector3 operator* (Vector3 vec) const { return Vector3(DirectX::XMVector3TransformNormal(vec, *this)); }
		INLINE Matrix3 operator* (const Matrix3& mat) const { return Matrix3(*this * mat.GetX(), *this * mat.GetY(), *this * mat.GetZ()); }

	private:
		Vector3 m_mat[3];
	};
}