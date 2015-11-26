#include "FrustumBounds.h"

namespace Galaxy3D
{
	FrustumBounds::FrustumBounds(const Matrix4x4 &mat)
	{
		Vector4 left_plane;
		left_plane.x = mat.m30 + mat.m00;
		left_plane.y = mat.m31 + mat.m01;
		left_plane.z = mat.m32 + mat.m02;
		left_plane.w = mat.m33 + mat.m03;

		Vector4 right_plane;
		right_plane.x = mat.m30 - mat.m00;
		right_plane.y = mat.m31 - mat.m01;
		right_plane.z = mat.m32 - mat.m02;
		right_plane.w = mat.m33 - mat.m03;

		Vector4 bottom_plane;
		bottom_plane.x = mat.m30 + mat.m10;
		bottom_plane.y = mat.m31 + mat.m11;
		bottom_plane.z = mat.m32 + mat.m12;
		bottom_plane.w = mat.m33 + mat.m13;

		Vector4 top_plane;
		top_plane.x = mat.m30 - mat.m10;
		top_plane.y = mat.m31 - mat.m11;
		top_plane.z = mat.m32 - mat.m12;
		top_plane.w = mat.m33 - mat.m13;

		Vector4 near_plane;
		near_plane.x = mat.m30 + mat.m20;
		near_plane.y = mat.m31 + mat.m21;
		near_plane.z = mat.m32 + mat.m22;
		near_plane.w = mat.m33 + mat.m23;

		Vector4 far_plane;
		far_plane.x = mat.m30 - mat.m20;
		far_plane.y = mat.m31 - mat.m21;
		far_plane.z = mat.m32 - mat.m22;
		far_plane.w = mat.m33 - mat.m23;

		m_frustum_planes[0] = left_plane;
		m_frustum_planes[1] = right_plane;
		m_frustum_planes[2] = bottom_plane;
		m_frustum_planes[3] = top_plane;
		m_frustum_planes[4] = near_plane;
		m_frustum_planes[5] = far_plane;

		//normalize
		for(int i=0; i<6; i++)
		{
			Vector4 &plane = m_frustum_planes[i];

			float div = 1.0f / Vector3(plane.x, plane.y, plane.z).Magnitude();
			plane *= div;
		}
	}

	bool FrustumBounds::ContainsPoint(const Vector3 &point) const
	{
		for(int i=0; i<6; i++)
		{
			float distance = m_frustum_planes[i].x * point.x + m_frustum_planes[i].y * point.y + m_frustum_planes[i].z * point.z + m_frustum_planes[i].w;
			if(distance < 0)
			{
				return false;
			}
		}

		return true;
	}

	int FrustumBounds::ContainsSphere(const Vector3 &center, float radius) const
	{
		for(int i=0; i<6; i++)
		{
			float distance = m_frustum_planes[i].x * center.x + m_frustum_planes[i].y * center.y + m_frustum_planes[i].z * center.z + m_frustum_planes[i].w;
			if(distance < -radius)
			{
				return ContainsResult::Out;
			}

			if(fabs(distance) < radius)
			{
				return ContainsResult::Cross;
			}
		}

		return ContainsResult::In;
	}

    int FrustumBounds::ContainsBounds(const Vector3 &center, const Vector3 &extents) const
    {
        Vector3 corners[8];
        corners[0] = center + Vector3(extents.x, extents.y, extents.z);
        corners[1] = center + Vector3(-extents.x, extents.y, extents.z);
        corners[2] = center + Vector3(-extents.x, extents.y, -extents.z);
        corners[3] = center + Vector3(extents.x, extents.y, -extents.z);
        corners[4] = center + Vector3(extents.x, -extents.y, extents.z);
        corners[5] = center + Vector3(-extents.x, -extents.y, extents.z);
        corners[6] = center + Vector3(-extents.x, -extents.y, -extents.z);
        corners[7] = center + Vector3(extents.x, -extents.y, -extents.z);

        int in_plane_count = 0;

        for(int i=0; i<6; i++)
        {
            int in_count = 0;

            for(int j=0; j<8; j++)
            {
                Vector3 point = corners[j];
                float distance = m_frustum_planes[i].x * point.x + m_frustum_planes[i].y * point.y + m_frustum_planes[i].z * point.z + m_frustum_planes[i].w;
                if(distance >= 0)
                {
                    in_count++;
                }
            }

            // all points in same side to one plane
            if(in_count == 0)
            {
                return ContainsResult::Out;
            }
            else if(in_count == 8)
            {
                in_plane_count++;
            }
        }

        if(in_plane_count == 6)
        {
            return ContainsResult::In;
        }

        return ContainsResult::Cross;
    }
}