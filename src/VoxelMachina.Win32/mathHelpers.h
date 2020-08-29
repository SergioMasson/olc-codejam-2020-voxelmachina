#pragma once

#include "DirectXMath.h"

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

	constexpr float Pi = 3.1415926535f;

	struct Vector3
	{
		float X;
		float Y;
		float Z;

		// Overload + operator to add two Box objects.
		Vector3 operator+(const Vector3& b)
		{
			return Vector3{ this->X + b.X, this->Y + b.Y, this->Z + b.Z };
		}

		// Overload + operator to add two Box objects.
		Vector3 operator-(const Vector3& b)
		{
			return Vector3{ this->X - b.X, this->Y - b.Y, this->Z - b.Z };
		}

		// Overload + operator to add two Box objects.
		Vector3 operator+(const float& b)
		{
			return Vector3{ this->X + b, this->Y + b, this->Z + b };
		}

		// Overload + operator to add two Box objects.
		Vector3 operator-(const float& b)
		{
			return Vector3{ this->X - b, this->Y - b, this->Z - b };
		}

		// Overload + operator to add two Box objects.
		Vector3 operator*(const float& b)
		{
			return Vector3{ this->X * b, this->Y * b, this->Z * b };
		}

		// Overload + operator to add two Box objects.
		Vector3 operator/(const float& b)
		{
			return Vector3{ this->X / b, this->Y / b, this->Z / b };
		}

		// Overload + operator to add two Box objects.
		Vector3& operator-=(const Vector3& b)
		{
			this->X = this->X - b.X;
			this->Y = this->Y - b.Y;
			this->Z = this->Z - b.Z;

			return *this;
		}

		// Overload + operator to add two Box objects.
		Vector3& operator+=(const Vector3& b)
		{
			this->X = this->X + b.X;
			this->Y = this->Y + b.Y;
			this->Z = this->Z + b.Z;

			return *this;
		}

		inline operator DirectX::XMVECTOR()
		{
			return DirectX::XMVectorSet(this->X, this->Y, this->Z, 1.0f);
		}
	};
}