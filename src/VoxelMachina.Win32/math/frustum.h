#pragma once

#include "boundingSphere.h"
#include "boundingPlane.h"
#include "boudingBox.h"

namespace math
{
	class Frustum
	{
	public:
		Frustum() {}

		Frustum(const Matrix4& ProjectionMatrix);

		enum CornerID
		{
			kNearLowerLeft, kNearUpperLeft, kNearLowerRight, kNearUpperRight,
			kFarLowerLeft, kFarUpperLeft, kFarLowerRight, kFarUpperRight
		};

		enum PlaneID
		{
			kNearPlane, kFarPlane, kLeftPlane, kRightPlane, kTopPlane, kBottomPlane
		};

		Vector3         GetFrustumCorner(CornerID id) const { return m_FrustumCorners[id]; }
		BoundingPlane   GetFrustumPlane(PlaneID id) const { return m_FrustumPlanes[id]; }

		// Test whether the bounding sphere intersects the frustum.  Intersection is defined as either being
		// fully contained in the frustum, or by intersecting one or more of the planes.
		INLINE bool IntersectSphere(BoundingSphere sphere) const
		{
			float radius = sphere.GetRadius();

			for (int i = 0; i < 6; ++i)
			{
				auto distance = m_FrustumPlanes[i].DistanceFromPoint(sphere.GetCenter());

				if (distance - (radius) >= 0.0f)
					return false;
			}
			return true;
		}

		// We don't officially have a BoundingBox class yet, but let's assume it's forthcoming.  (There is a
		// simple struct in the Model project.)
		INLINE bool IntersectBoundingBox(const Vector3 minBound, const Vector3 maxBound) const
		{
			for (int i = 0; i < 6; ++i)
			{
				BoundingPlane p = m_FrustumPlanes[i];

				m_FrustumPlanes[i].DistanceFromPoint(minBound);

				Vector3 farCorner = Select(minBound, maxBound, p.GetNormal() > Vector3(0, 0, 0));

				if (p.DistanceFromPoint(farCorner) >= 0.0f)
					return false;
			}

			return true;
		}

		INLINE bool IntersectBoundingBox(const BoudingBox& box) const
		{
			return IntersectBoundingBox(box.m_topCorner, box.m_bottomCorner);
		}

		INLINE friend Frustum  operator* (const Transform& xform, const Frustum& frustum)
		{
			Frustum result;

			for (int i = 0; i < 8; ++i)
				result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

			Matrix4 XForm = Transpose(Invert(Matrix4(xform)));

			for (int i = 0; i < 6; ++i)
				result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

			return result;
		}// Slow

		INLINE friend Frustum  operator* (const Matrix4& mtx, const Frustum& frustum)
		{
			Frustum result;

			for (int i = 0; i < 8; ++i)
				result.m_FrustumCorners[i] = Vector3(mtx * frustum.m_FrustumCorners[i]);

			Matrix4 XForm = Transpose(Invert(mtx));

			for (int i = 0; i < 6; ++i)
				result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

			return result;
		}                // Slowest (and most general)

	private:

		// Perspective frustum constructor (for pyramid-shaped frusta)
		void ConstructPerspectiveFrustum(float HTan, float VTan, float NearClip, float FarClip);

		// Orthographic frustum constructor (for box-shaped frusta)
		void ConstructOrthographicFrustum(float Left, float Right, float Top, float Bottom, float NearClip, float FarClip);

		Vector3 m_FrustumCorners[8];        // the corners of the frustum
		BoundingPlane m_FrustumPlanes[6];            // the bounding planes
	};
}