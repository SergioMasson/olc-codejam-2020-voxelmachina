#pragma once
#include "DirectXMath.h"
#include "common.h"

namespace math
{
	//Structure for holding a single scalar value for SIMD operations.
	struct Scalar
	{
	public:
		Scalar() { m_vec = DirectX::XMVectorReplicate(0); }
		Scalar(const Scalar& s) { m_vec = s; }
		Scalar(float f) { m_vec = DirectX::XMVectorReplicate(f); }
		explicit Scalar(DirectX::FXMVECTOR vec) { m_vec = vec; }

		operator DirectX::XMVECTOR() const { return m_vec; }
		operator float() const { return DirectX::XMVectorGetX(m_vec); }

	private:
		DirectX::XMVECTOR m_vec;
	};

	inline Scalar operator- (Scalar s) { return Scalar(DirectX::XMVectorNegate(s)); }
	inline Scalar operator+ (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorAdd(s1, s2)); }
	inline Scalar operator- (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorSubtract(s1, s2)); }
	inline Scalar operator* (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorMultiply(s1, s2)); }
	inline Scalar operator/ (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorDivide(s1, s2)); }
	inline Scalar operator+ (Scalar s1, float s2) { return s1 + Scalar(s2); }
	inline Scalar operator- (Scalar s1, float s2) { return s1 - Scalar(s2); }
	inline Scalar operator* (Scalar s1, float s2) { return s1 * Scalar(s2); }
	inline Scalar operator/ (Scalar s1, float s2) { return s1 / Scalar(s2); }
	inline Scalar operator+ (float s1, Scalar s2) { return Scalar(s1) + s2; }
	inline Scalar operator- (float s1, Scalar s2) { return Scalar(s1) - s2; }
	inline Scalar operator* (float s1, Scalar s2) { return Scalar(s1) * s2; }
	inline Scalar operator/ (float s1, Scalar s2) { return Scalar(s1) / s2; }
}