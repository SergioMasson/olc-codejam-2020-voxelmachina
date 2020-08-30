#pragma once

#include "math/vector3.h"
#include "math/transform.h"
#include "math/matrix4.h"
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
}