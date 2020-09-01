#pragma once

#include "math/vector3.h"
#include "math/transform.h"
#include "math/matrix4.h"
#include "DirectXMath.h"
#include "math/functions.h"

namespace math
{
	constexpr DirectX::XMFLOAT4X4 Identity4x4()
	{
		return DirectX::XMFLOAT4X4
		{ 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f };
	}

	INLINE Scalar Length(Vector3 v) { return Scalar(DirectX::XMVector3Length(v)); }
	INLINE Scalar LengthSquare(Vector3 v) { return Scalar(DirectX::XMVector3LengthSq(v)); }
	INLINE Scalar LengthRecip(Vector3 v) { return Scalar(DirectX::XMVector3ReciprocalLength(v)); }
	INLINE Scalar Dot(Vector3 v1, Vector3 v2) { return Scalar(DirectX::XMVector3Dot(v1, v2)); }
	INLINE Scalar Dot(Vector4 v1, Vector4 v2) { return Scalar(DirectX::XMVector4Dot(v1, v2)); }
	INLINE Vector3 Cross(Vector3 v1, Vector3 v2) { return Vector3(DirectX::XMVector3Cross(v1, v2)); }
	INLINE Vector3 Normalize(Vector3 v) { return Vector3(DirectX::XMVector3Normalize(v)); }
	INLINE Vector4 Normalize(Vector4 v) { return Vector4(DirectX::XMVector4Normalize(v)); }
	INLINE Quaternion Normalize(Quaternion q) { return Quaternion(DirectX::XMQuaternionNormalize(q)); }
	INLINE Matrix3 Transpose(const Matrix3& mat) { return Matrix3(DirectX::XMMatrixTranspose(mat)); }

	constexpr float Pi = 3.1415926535f;
}