#pragma once
#include <cmath>
#include <algorithm>
#include "Vector.h"
#include "Matrix.h"
#include "MathUtility.h"  

namespace MMath {
     
    //using Quatf = Quaternion<float>;
    //using Quat = Quatf;

    struct Quaternion {
        float x{}, y{}, z{}, w{ 1 };
    };

    inline Quaternion QuaternionIdentity()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    inline bool QuaternionEqual(const Quaternion& a, const Quaternion& b, float eps = 1e-6f)
    {
        return (fabs(a.x - b.x) < eps &&
            fabs(a.y - b.y) < eps &&
            fabs(a.z - b.z) < eps &&
            fabs(a.w - b.w) < eps);
    }

    inline Quaternion QuaternionScale(const Quaternion& q, float s)
    {
        return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
    }

    inline Quaternion QuaternionInverse(const Quaternion& q)
    {
        return Quaternion(-q.x, -q.y, -q.z, q.w);
    }

    inline Quaternion QuaternionAdd(const Quaternion& a, const Quaternion& b)
    {
        return Quaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }


    inline Quaternion QuaternionNormalize(const Quaternion& q)
    {
        float len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        if (len <= 1e-8f) return QuaternionIdentity();
        float inv = 1.0f / len;
        return Quaternion(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
    }

    //hamilton order
    inline Quaternion QuaternionMultiply(const Quaternion& a, const Quaternion& b)
    {
        return Quaternion(
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
        );
    }

    //same as xmmath, how really standard?
    inline Quaternion QuaternionRotationRollPitchYaw(float x, float y, float z)
    {
        float hx = x * 0.5f;  // pitch
        float hy = y * 0.5f;  // yaw
        float hz = z * 0.5f;  // roll

        float sx = sinf(hx), cx = cosf(hx);
        float sy = sinf(hy), cy = cosf(hy);
        float sz = sinf(hz), cz = cosf(hz);

        Quaternion q;
        q.x = sx * cy * cz + cx * sy * sz; // pitch (x)
        q.y = cx * sy * cz - sx * cy * sz; // yaw   (y)
        q.z = cx * cy * sz - sx * sy * cz; // roll  (z)
        q.w = cx * cy * cz + sx * sy * sz;
        return q;
    }


    inline Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t)
    {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

        // If dot < 0, invert one quaternion
        Quaternion bb = b;
        if (dot < 0.0f) {
            dot = -dot;
            bb = Quaternion(-b.x, -b.y, -b.z, -b.w);
        }

        if (dot > 0.9995f) {
            // Lerp fallback
            Quaternion r(
                a.x + t * (bb.x - a.x),
                a.y + t * (bb.y - a.y),
                a.z + t * (bb.z - a.z),
                a.w + t * (bb.w - a.w));
            return QuaternionNormalize(r);
        }

        float theta = acosf(dot);
        float sinT = sinf(theta);
        float w1 = sinf((1.0f - t) * theta) / sinT;
        float w2 = sinf(t * theta) / sinT;

        return Quaternion(
            a.x * w1 + bb.x * w2,
            a.y * w1 + bb.y * w2,
            a.z * w1 + bb.z * w2,
            a.w * w1 + bb.w * w2
        );
    }


    inline Float3x3 MatrixRotationQuaternion(const Quaternion& q)
    {
        Float3x3 R;

        float xx = q.x * q.x;  float yy = q.y * q.y;  float zz = q.z * q.z;
        float xy = q.x * q.y;  float xz = q.x * q.z;  float yz = q.y * q.z;
        float wx = q.w * q.x;  float wy = q.w * q.y;  float wz = q.w * q.z;

        // Directly write the transposed matrix
        R[0][0] = 1.0f - 2.0f * (yy + zz);
        R[0][1] = 2.0f * (xy + wz);
        R[0][2] = 2.0f * (xz - wy);

        R[1][0] = 2.0f * (xy - wz);
        R[1][1] = 1.0f - 2.0f * (xx + zz);
        R[1][2] = 2.0f * (yz + wx);

        R[2][0] = 2.0f * (xz + wy);
        R[2][1] = 2.0f * (yz - wx);
        R[2][2] = 1.0f - 2.0f * (xx + yy);

        return R;
    }

    inline Float3 Vector3Rotate(const Quaternion& q, const Float3& v)
    {
        // Extract vector part of quaternion
        Float3 u{ q.x, q.y, q.z };
        float s = q.w;

        // cross(u, v)
        Float3 uv{
            u.y() * v.z() - u.z() * v.y(),
            u.z() * v.x() - u.x() * v.z(),
            u.x() * v.y() - u.y() * v.x()
        };

        // cross(u, uv)
        Float3 uuv{
            u.y() * uv.z() - u.z() * uv.y(),
            u.z() * uv.x() - u.x() * uv.z(),
            u.x() * uv.y() - u.y() * uv.x()
        };

        // v + 2.0f * (s * uv + uuv)
        return Float3{
            v.x() + 2.0f * (s * uv.x() + uuv.x()),
            v.y() + 2.0f * (s * uv.y() + uuv.y()),
            v.z() + 2.0f * (s * uv.z() + uuv.z())
        };
    }

 
    inline DirectX::XMVECTOR ToXMVECTOR(const Quaternion& q)
    {
        return DirectX::XMVectorSet(q.x, q.y, q.z, q.w);
    }

    // XMVECTOR -> Quaternion
    inline Quaternion FromXMVECTOR(DirectX::XMVECTOR v)
    {
        Quaternion q;
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&q), v);
        return q;
    }

    //inline Float3x3 QuaternionToRotationMatrix(const DirectX::XMVECTOR& q)
//{
//	using namespace DirectX;
//	// Convert quaternion to rotation matrix
//	XMMATRIX R_ = XMMatrixRotationQuaternion(q);
//	Float3x3 R;
//	R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
//	R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
//	R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };
//	return R;
//}

}