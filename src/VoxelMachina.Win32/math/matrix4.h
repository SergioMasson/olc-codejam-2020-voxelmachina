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
#include "transform.h"

namespace math
{
	__declspec(align(16)) class Matrix4
	{
	public:
		INLINE Matrix4() { m_mat = DirectX::XMMatrixIdentity(); }
		INLINE Matrix4(Vector3 x, Vector3 y, Vector3 z, Vector3 w)
		{
			m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
			m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
		}
		INLINE Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
		INLINE Matrix4(const Matrix4& mat) { m_mat = mat.m_mat; }
		INLINE Matrix4(const Matrix3& mat)
		{
			m_mat.r[0] = SetWToZero(mat.GetX());
			m_mat.r[1] = SetWToZero(mat.GetY());
			m_mat.r[2] = SetWToZero(mat.GetZ());
			m_mat.r[3] = CreateWUnitVector();
		}
		INLINE Matrix4(const Matrix3& xyz, Vector3 w)
		{
			m_mat.r[0] = SetWToZero(xyz.GetX());
			m_mat.r[1] = SetWToZero(xyz.GetY());
			m_mat.r[2] = SetWToZero(xyz.GetZ());
			m_mat.r[3] = SetWToOne(w);
		}
		INLINE Matrix4(const Transform& xform)
		{
			*this = Matrix4{ DirectX::XMMatrixAffineTransformation(xform.GetScale(), math::Vector3(0, 0, 0), xform.GetRotation(), xform.GetTranslation()) };
		}

		INLINE explicit Matrix4(const DirectX::XMMATRIX& mat) { m_mat = mat; }

		INLINE const Matrix3& Get3x3() const { return (const Matrix3&)*this; }

		INLINE Vector4 GetX() const { return Vector4(m_mat.r[0]); }
		INLINE Vector4 GetY() const { return Vector4(m_mat.r[1]); }
		INLINE Vector4 GetZ() const { return Vector4(m_mat.r[2]); }
		INLINE Vector4 GetW() const { return Vector4(m_mat.r[3]); }

		INLINE void SetX(Vector4 x) { m_mat.r[0] = x; }
		INLINE void SetY(Vector4 y) { m_mat.r[1] = y; }
		INLINE void SetZ(Vector4 z) { m_mat.r[2] = z; }
		INLINE void SetW(Vector4 w) { m_mat.r[3] = w; }

		INLINE Matrix4 InverseTranspose() const
		{
			// Inverse-transpose is just applied to normals.  So zero out
			// translation row so that it doesn't get into our inverse-transpose
			// calculation--we don't want the inverse-transpose of the translation.
			XMMATRIX A = m_mat;
			A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

			XMVECTOR det = XMMatrixDeterminant(A);
			return Matrix4{ XMMatrixTranspose(XMMatrixInverse(&det, A)) };
		}

		INLINE Matrix4 Transpose() const
		{
			XMMATRIX A = m_mat;
			return Matrix4{ XMMatrixTranspose(A) };
		}

		INLINE bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const
		{
			DirectX::XMVECTOR lscale;
			DirectX::XMVECTOR lrotation;
			DirectX::XMVECTOR ltranslation;

			auto succced = DirectX::XMMatrixDecompose(&lscale, &lrotation, &ltranslation, m_mat);

			if (succced)
			{
				scale = Vector3(lscale);
				rotation = Quaternion(lrotation);
				translation = Vector3(ltranslation);
			}

			return succced;
		}

		INLINE operator DirectX::XMMATRIX() const { return m_mat; }
		INLINE operator DirectX::XMFLOAT4X4() const
		{
			DirectX::XMFLOAT4X4 result;
			DirectX::XMStoreFloat4x4(&result, m_mat);
			return result;
		}

		INLINE Vector4 operator* (Vector3 vec) const { return Vector4(DirectX::XMVector3Transform(vec, m_mat)); }
		INLINE Vector4 operator* (Vector4 vec) const { return Vector4(DirectX::XMVector4Transform(vec, m_mat)); }
		INLINE Matrix4 operator* (const Matrix4& mat) const { return Matrix4(DirectX::XMMatrixMultiply(mat, m_mat)); }

		INLINE static  Matrix4 MakeScale(float scale) { return Matrix4(DirectX::XMMatrixScaling(scale, scale, scale)); }
		INLINE static  Matrix4 MakeScale(Vector3 scale) { return Matrix4(DirectX::XMMatrixScalingFromVector(scale)); }

	private:
		DirectX::XMMATRIX m_mat;
	};
}