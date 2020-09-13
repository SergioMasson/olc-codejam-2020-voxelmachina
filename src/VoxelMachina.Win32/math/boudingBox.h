#pragma once

#include "vector3.h"

namespace math
{
	class Frustum;

	//An axis-aligned bounding box.
	class BoudingBox
	{
		BoudingBox(Vector3 top, Vector3 bottom) : m_topCorner{ top }, m_bottomCorner{ bottom }{}

		INLINE bool IsPointInside(const Vector3& point)
		{
			//TODO(Sergio): Do this using SIMD.
			return	(point.GetX() >= m_bottomCorner.GetX()) &&
				(point.GetX() <= m_topCorner.GetX()) &&
				(point.GetY() >= m_bottomCorner.GetY()) &&
				(point.GetY() <= m_topCorner.GetY()) &&
				(point.GetZ() >= m_bottomCorner.GetZ()) &&
				(point.GetZ() <= m_topCorner.GetZ());
		}

		INLINE bool IsOverlaping(const BoudingBox& other)
		{
			//TODO(Sergio): Do this using SIMD.
			return (m_bottomCorner.GetX() <= other.m_topCorner.GetX() && m_topCorner.GetX() >= other.m_bottomCorner.GetX()) &&
				(m_bottomCorner.GetY() <= other.m_topCorner.GetY() && m_topCorner.GetY() >= other.m_bottomCorner.GetY()) &&
				(m_bottomCorner.GetZ() <= other.m_topCorner.GetZ() && m_topCorner.GetZ() >= other.m_bottomCorner.GetZ());
		}

	private:
		Vector3 m_topCorner;
		Vector3 m_bottomCorner;

		friend Frustum;
	};
}