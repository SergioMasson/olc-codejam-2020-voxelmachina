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
#include "../mathHelpers.h"

namespace math
{
	class BoundingSphere
	{
	public:
		BoundingSphere() {}
		INLINE BoundingSphere(Vector3 center, Scalar radius)
		{
			m_repr = Vector4(center);
			m_repr.SetW(radius);
		}
		INLINE explicit BoundingSphere(Vector4 sphere) : m_repr(sphere)
		{
		}

		INLINE Vector3 GetCenter(void) const
		{
			return m_repr.Convert();
		};

		INLINE Scalar GetRadius(void) const { return m_repr.GetW(); };

		INLINE friend BoundingSphere  operator* (const Matrix4& mtx, const BoundingSphere& frustum)
		{
			Vector3 position{ frustum.m_repr };
			position = Vector3{ mtx * position };
			return BoundingSphere{ position , frustum.GetRadius() };
		}

	private:
		Vector4 m_repr;
	};
}