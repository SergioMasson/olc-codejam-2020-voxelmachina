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
#include "scalar.h"

namespace math
{
	class Vector4;

	// A 3-vector with an unspecified fourth component.  Depending on the context, the W can be 0 or 1, but both are implicit.
	// The actual value of the fourth component is undefined for performance reasons.
	class Vector3
	{
	public:
		INLINE Vector3() { m_vec = DirectX::XMVectorSet(0, 0, 0, 0); }
		INLINE Vector3(float x, float y, float z) { m_vec = DirectX::XMVectorSet(x, y, z, 0); }
		INLINE Vector3(const DirectX::XMFLOAT3& v) { m_vec = DirectX::XMLoadFloat3(&v); }
		INLINE Vector3(const Vector3& v) { m_vec = v; }
		INLINE Vector3(Scalar s) { m_vec = s; }
		INLINE explicit Vector3(Vector4 v);
		INLINE explicit Vector3(DirectX::FXMVECTOR vec) { m_vec = vec; }

		INLINE operator DirectX::XMVECTOR() const { return m_vec; }
		INLINE operator DirectX::XMFLOAT3() const
		{
			DirectX::XMFLOAT3 result;
			DirectX::XMStoreFloat3(&result, m_vec);
			return result;
		}

		INLINE Scalar GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		INLINE Scalar GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		INLINE Scalar GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		INLINE void SetX(Scalar x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, x); }
		INLINE void SetY(Scalar y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, y); }
		INLINE void SetZ(Scalar z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, z); }

		INLINE Vector3 operator- () const { return Vector3(DirectX::XMVectorNegate(m_vec)); }
		INLINE Vector3 operator+ (Vector3 v2) const { return Vector3(DirectX::XMVectorAdd(m_vec, v2)); }
		INLINE Vector3 operator- (Vector3 v2) const { return Vector3(DirectX::XMVectorSubtract(m_vec, v2)); }
		INLINE Vector3 operator* (Vector3 v2) const { return Vector3(DirectX::XMVectorMultiply(m_vec, v2)); }
		INLINE Vector3 operator/ (Vector3 v2) const { return Vector3(DirectX::XMVectorDivide(m_vec, v2)); }
		INLINE Vector3 operator* (Scalar  v2) const { return *this * Vector3(v2); }
		INLINE Vector3 operator/ (Scalar  v2) const { return *this / Vector3(v2); }
		INLINE Vector3 operator* (float  v2) const { return *this * Scalar(v2); }
		INLINE Vector3 operator/ (float  v2) const { return *this / Scalar(v2); }

		INLINE Vector3& operator += (Vector3 v) { *this = *this + v; return *this; }
		INLINE Vector3& operator -= (Vector3 v) { *this = *this - v; return *this; }
		INLINE Vector3& operator *= (Vector3 v) { *this = *this * v; return *this; }
		INLINE Vector3& operator /= (Vector3 v) { *this = *this / v; return *this; }

		INLINE friend Vector3 operator* (Scalar  v1, Vector3 v2) { return Vector3(v1) * v2; }
		INLINE friend Vector3 operator/ (Scalar  v1, Vector3 v2) { return Vector3(v1) / v2; }
		INLINE friend Vector3 operator* (float   v1, Vector3 v2) { return Scalar(v1) * v2; }
		INLINE friend Vector3 operator/ (float   v1, Vector3 v2) { return Scalar(v1) / v2; }

	protected:
		DirectX::XMVECTOR m_vec;
	};

	// A 4-vector, completely defined.
	class Vector4
	{
	public:
		INLINE Vector4() { m_vec = DirectX::XMVectorSet(0, 0, 0, 0); }
		INLINE Vector4(float x, float y, float z, float w) { m_vec = DirectX::XMVectorSet(x, y, z, w); }
		INLINE Vector4(Vector3 xyz, float w) { m_vec = DirectX::XMVectorSetW(xyz, w); }
		INLINE Vector4(const Vector4& v) { m_vec = v; }
		INLINE Vector4(const Scalar& s) { m_vec = s; }
		INLINE explicit Vector4(DirectX::FXMVECTOR vec) { m_vec = vec; }

		INLINE operator DirectX::XMVECTOR() const { return m_vec; }
		INLINE operator DirectX::XMFLOAT4() const
		{
			DirectX::XMFLOAT4 result;
			DirectX::XMStoreFloat4(&result, m_vec);
			return result;
		}

		INLINE Scalar GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		INLINE Scalar GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		INLINE Scalar GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		INLINE Scalar GetW() const { return Scalar(DirectX::XMVectorSplatW(m_vec)); }
		INLINE void SetX(Scalar x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, x); }
		INLINE void SetY(Scalar y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, y); }
		INLINE void SetZ(Scalar z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, z); }
		INLINE void SetW(Scalar w) { m_vec = DirectX::XMVectorPermute<0, 1, 2, 7>(m_vec, w); }

		INLINE Vector3 Convert() const
		{
			Vector4 copy(m_vec);
			copy.SetW(0);
			return Vector3(copy.m_vec);
		}

		INLINE Vector4 operator- () const { return Vector4(DirectX::XMVectorNegate(m_vec)); }
		INLINE Vector4 operator+ (Vector4 v2) const { return Vector4(DirectX::XMVectorAdd(m_vec, v2)); }
		INLINE Vector4 operator- (Vector4 v2) const { return Vector4(DirectX::XMVectorSubtract(m_vec, v2)); }
		INLINE Vector4 operator* (Vector4 v2) const { return Vector4(DirectX::XMVectorMultiply(m_vec, v2)); }
		INLINE Vector4 operator/ (Vector4 v2) const { return Vector4(DirectX::XMVectorDivide(m_vec, v2)); }
		INLINE Vector4 operator* (Scalar  v2) const { return *this * Vector4(v2); }
		INLINE Vector4 operator/ (Scalar  v2) const { return *this / Vector4(v2); }
		INLINE Vector4 operator* (float   v2) const { return *this * Scalar(v2); }
		INLINE Vector4 operator/ (float   v2) const { return *this / Scalar(v2); }

		INLINE void operator*= (float   v2) { *this = *this * Scalar(v2); }
		INLINE void operator/= (float   v2) { *this = *this / Scalar(v2); }

		INLINE friend Vector4 operator* (Scalar  v1, Vector4 v2) { return Vector4(v1) * v2; }
		INLINE friend Vector4 operator/ (Scalar  v1, Vector4 v2) { return Vector4(v1) / v2; }
		INLINE friend Vector4 operator* (float   v1, Vector4 v2) { return Scalar(v1) * v2; }
		INLINE friend Vector4 operator/ (float   v1, Vector4 v2) { return Scalar(v1) / v2; }

	protected:
		DirectX::XMVECTOR m_vec;
	};

	INLINE Vector3::Vector3(Vector4 v)
	{
		Scalar W = v.GetW();
		m_vec = DirectX::XMVectorSelect(DirectX::XMVectorDivide(v, W), v, DirectX::XMVectorEqual(W, DirectX::XMVectorZero()));
	}

	class BoolVector
	{
	public:
		INLINE BoolVector(FXMVECTOR vec) { m_vec = vec; }
		INLINE operator XMVECTOR() const { return m_vec; }
		INLINE operator bool() const { return DirectX::XMVectorGetW(DirectX::XMVectorSum(m_vec)) != 0; }
		INLINE BoolVector operator&& (BoolVector v2) const { return BoolVector(DirectX::XMVectorAndInt(m_vec, v2)); }
		INLINE BoolVector operator!= (BoolVector v2) const { return BoolVector(DirectX::XMVectorNotEqualInt(m_vec, v2)); }
		INLINE BoolVector operator== (BoolVector v2) const { return BoolVector(DirectX::XMVectorEqualInt(m_vec, v2)); }
		INLINE BoolVector operator|| (BoolVector v2) const { return BoolVector(DirectX::XMVectorOrInt(m_vec, v2)); }
	protected:
		XMVECTOR m_vec;
	};
}