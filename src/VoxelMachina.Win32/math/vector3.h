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
		Vector3() { m_vec = DirectX::XMVectorSet(0, 0, 0, 1); }
		Vector3(float x, float y, float z) { m_vec = DirectX::XMVectorSet(x, y, z, z); }
		Vector3(const DirectX::XMFLOAT3& v) { m_vec = DirectX::XMLoadFloat3(&v); }
		Vector3(const Vector3& v) { m_vec = v; }
		Vector3(Scalar s) { m_vec = s; }
		explicit Vector3(Vector4 v);
		explicit Vector3(DirectX::FXMVECTOR vec) { m_vec = vec; }

		operator DirectX::XMVECTOR() const { return m_vec; }

		Scalar GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		Scalar GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		Scalar GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		void SetX(Scalar x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, x); }
		void SetY(Scalar y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, y); }
		void SetZ(Scalar z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, z); }

		Vector3 operator- () const { return Vector3(DirectX::XMVectorNegate(m_vec)); }
		Vector3 operator+ (Vector3 v2) const { return Vector3(DirectX::XMVectorAdd(m_vec, v2)); }
		Vector3 operator- (Vector3 v2) const { return Vector3(DirectX::XMVectorSubtract(m_vec, v2)); }
		Vector3 operator* (Vector3 v2) const { return Vector3(DirectX::XMVectorMultiply(m_vec, v2)); }
		Vector3 operator/ (Vector3 v2) const { return Vector3(DirectX::XMVectorDivide(m_vec, v2)); }
		Vector3 operator* (Scalar  v2) const { return *this * Vector3(v2); }
		Vector3 operator/ (Scalar  v2) const { return *this / Vector3(v2); }
		Vector3 operator* (float  v2) const { return *this * Scalar(v2); }
		Vector3 operator/ (float  v2) const { return *this / Scalar(v2); }

		Vector3& operator += (Vector3 v) { *this = *this + v; return *this; }
		Vector3& operator -= (Vector3 v) { *this = *this - v; return *this; }
		Vector3& operator *= (Vector3 v) { *this = *this * v; return *this; }
		Vector3& operator /= (Vector3 v) { *this = *this / v; return *this; }

		friend Vector3 operator* (Scalar  v1, Vector3 v2) { return Vector3(v1) * v2; }
		friend Vector3 operator/ (Scalar  v1, Vector3 v2) { return Vector3(v1) / v2; }
		friend Vector3 operator* (float   v1, Vector3 v2) { return Scalar(v1) * v2; }
		friend Vector3 operator/ (float   v1, Vector3 v2) { return Scalar(v1) / v2; }

	protected:
		DirectX::XMVECTOR m_vec;
	};

	// A 4-vector, completely defined.
	class Vector4
	{
	public:
		Vector4() { m_vec = DirectX::XMVectorSet(0, 0, 0, 0); }
		Vector4(float x, float y, float z, float w) { m_vec = DirectX::XMVectorSet(x, y, z, w); }
		Vector4(Vector3 xyz, float w) { m_vec = DirectX::XMVectorSetW(xyz, w); }
		Vector4(const Vector4& v) { m_vec = v; }
		Vector4(const Scalar& s) { m_vec = s; }
		explicit Vector4(DirectX::FXMVECTOR vec) { m_vec = vec; }

		operator DirectX::XMVECTOR() const { return m_vec; }

		Scalar GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		Scalar GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		Scalar GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		Scalar GetW() const { return Scalar(DirectX::XMVectorSplatW(m_vec)); }
		void SetX(Scalar x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, x); }
		void SetY(Scalar y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, y); }
		void SetZ(Scalar z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, z); }
		void SetW(Scalar w) { m_vec = DirectX::XMVectorPermute<0, 1, 2, 7>(m_vec, w); }

		Vector4 operator- () const { return Vector4(DirectX::XMVectorNegate(m_vec)); }
		Vector4 operator+ (Vector4 v2) const { return Vector4(DirectX::XMVectorAdd(m_vec, v2)); }
		Vector4 operator- (Vector4 v2) const { return Vector4(DirectX::XMVectorSubtract(m_vec, v2)); }
		Vector4 operator* (Vector4 v2) const { return Vector4(DirectX::XMVectorMultiply(m_vec, v2)); }
		Vector4 operator/ (Vector4 v2) const { return Vector4(DirectX::XMVectorDivide(m_vec, v2)); }
		Vector4 operator* (Scalar  v2) const { return *this * Vector4(v2); }
		Vector4 operator/ (Scalar  v2) const { return *this / Vector4(v2); }
		Vector4 operator* (float   v2) const { return *this * Scalar(v2); }
		Vector4 operator/ (float   v2) const { return *this / Scalar(v2); }

		void operator*= (float   v2) { *this = *this * Scalar(v2); }
		void operator/= (float   v2) { *this = *this / Scalar(v2); }

		friend Vector4 operator* (Scalar  v1, Vector4 v2) { return Vector4(v1) * v2; }
		friend Vector4 operator/ (Scalar  v1, Vector4 v2) { return Vector4(v1) / v2; }
		friend Vector4 operator* (float   v1, Vector4 v2) { return Scalar(v1) * v2; }
		friend Vector4 operator/ (float   v1, Vector4 v2) { return Scalar(v1) / v2; }

	protected:
		DirectX::XMVECTOR m_vec;
	};

	inline Vector3::Vector3(Vector4 v)
	{
		Scalar W = v.GetW();
		m_vec = DirectX::XMVectorSelect(DirectX::XMVectorDivide(v, W), v, DirectX::XMVectorEqual(W, DirectX::XMVectorZero()));
	}
}