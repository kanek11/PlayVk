#pragma once
#include <cmath>
#include <algorithm>
#include "Vector.h"
#include "Matrix.h"
#include "MathUtility.h"  

namespace MMath {

    template <FLOP_t T>
    struct Quaternion { 
        T x{}, y{}, z{}, w{ 1 };  

        
        constexpr Quaternion() = default;
        constexpr Quaternion(T xi, T yi, T zi, T wi) : x(xi), y(yi), z(zi), w(wi) {}

        static constexpr Quaternion Identity() { return { T(0), T(0), T(0), T(1) }; }
    };

    using Quatf = Quaternion<float>;
    using Quat = Quatf;  
     
    template <FLOP_t T>
    inline T QDot(const Quaternion<T>& a, const Quaternion<T>& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    template <FLOP_t T>
    inline T QLen(const Quaternion<T>& q) {
        return std::sqrt(QDot(q, q));
    }

    template <FLOP_t T>
    inline Quaternion<T> QNormalize(const Quaternion<T>& q) {
        const T len = QLen(q);
        if (len <= T(0)) return Quaternion<T>::Identity();
        const T inv = T(1) / len;
        return { q.x * inv, q.y * inv, q.z * inv, q.w * inv };
    }

    template <FLOP_t T>
    inline Quaternion<T> QConjugate(const Quaternion<T>& q) {
        return { -q.x, -q.y, -q.z, q.w };
    }

    template <FLOP_t T>
    inline Quaternion<T> QInverse(const Quaternion<T>& q) {
        const T n2 = QDot(q, q);
        if (n2 <= T(0)) return Quaternion<T>::Identity();
        const T inv = T(1) / n2;
        return { -q.x * inv, -q.y * inv, -q.z * inv, q.w * inv };
    }

    // XMQuaternionMultiply(A,B)
    template <FLOP_t T>
    inline Quaternion<T> QMul(const Quaternion<T>& a, const Quaternion<T>& b) {
        // Hamilton product  
        return {
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
        };
    }

    //  
    template <FLOP_t T>
    inline Quaternion<T> QFromAxisAngle(const Vector<T, 3>& axis, T angleRad) {
        Vector<T, 3> n = Normalize(axis);  
        const T s = std::sin(angleRad * T(0.5));
        const T c = std::cos(angleRad * T(0.5));
        return QNormalize<T>({ n[0] * s, n[1] * s, n[2] * s, c });
    }

    //  XMQuaternionRotationRollPitchYaw(pitch yaw roll) 
    template <FLOP_t T>
    inline Quaternion<T> QFromRollPitchYaw(T pitch, T yaw, T roll) {
        // q = qz(roll) * qy(yaw) * qx(pitch)
        const T hp = pitch * T(0.5), hy = yaw * T(0.5), hr = roll * T(0.5);
        const T sp = std::sin(hp), cp = std::cos(hp);
        const T sy = std::sin(hy), cy = std::cos(hy);
        const T sr = std::sin(hr), cr = std::cos(hr);

        // Yaw * Pitch * Roll 
        Quaternion<T> qx{ sp, 0, 0, cp };
        Quaternion<T> qy{ 0, sy, 0, cy };
        Quaternion<T> qz{ 0, 0, sr, cr };
        return QNormalize(QMul(QMul(qz, qy), qx));
    }

    // 
    template <FLOP_t T>
    inline Vector<T, 3> QRotate(const Quaternion<T>& q_, const Vector<T, 3>& v) {
        // v' = q * (v,0) * inv(q)
        auto q = QNormalize(q_);
        Quaternion<T> p{ v[0], v[1], v[2], T(0) };
        auto r = QMul(QMul(q, p), QConjugate(q));
        return { r.x, r.y, r.z };
    }

    // 
    template <FLOP_t T>
    inline Matrix<T, 3, 3> QToMat3(const Quaternion<T>& q_) {
        auto q = QNormalize(q_);
        const T x = q.x, y = q.y, z = q.z, w = q.w;
        const T xx = x * x, yy = y * y, zz = z * z;
        const T xy = x * y, xz = x * z, yz = y * z;
        const T wx = w * x, wy = w * y, wz = w * z;

        //row major
        const T r00 = 1 - 2 * (yy + zz);
        const T r01 = 2 * (xy + wz);
        const T r02 = 2 * (xz - wy);

        const T r10 = 2 * (xy - wz);
        const T r11 = 1 - 2 * (xx + zz);
        const T r12 = 2 * (yz + wx);

        const T r20 = 2 * (xz + wy);
        const T r21 = 2 * (yz - wx);
        const T r22 = 1 - 2 * (xx + yy);

        // 
        Matrix<T, 3, 3> m;
        m[0] = { r00, r10, r20 };  
        m[1] = { r01, r11, r21 };  
        m[2] = { r02, r12, r22 };  
        return m;
    }

    template <FLOP_t T>
    inline Matrix<T, 4, 4> QToMat4(const Quaternion<T>& q_) {
        auto R3 = QToMat3(q_);
        Matrix<T, 4, 4> M = MatrixIdentity<T, 4>();
        M[0] = { R3[0][0], R3[0][1], R3[0][2], T(0) };
        M[1] = { R3[1][0], R3[1][1], R3[1][2], T(0) };
        M[2] = { R3[2][0], R3[2][1], R3[2][2], T(0) };
        M[3] = { T(0), T(0), T(0), T(1) };
        return M;
    }

    // 
    template <FLOP_t T>
    inline Quaternion<T> QLerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) {
        //nlerp 
        Quaternion<T> r{
            (1 - t) * a.x + t * b.x,
            (1 - t) * a.y + t * b.y,
            (1 - t) * a.z + t * b.z,
            (1 - t) * a.w + t * b.w
        };
        return QNormalize(r);
    }

    template <FLOP_t T>
    inline Quaternion<T> QSlerp(Quaternion<T> a, Quaternion<T> b, T t) {
        T cosTheta = QDot(a, b);
		// shortest path
        if (cosTheta < T(0)) {
            b = { -b.x, -b.y, -b.z, -b.w };
            cosTheta = -cosTheta;
        }
        const T EPS = T(1e-6);
        if (cosTheta > T(1) - EPS) return QLerp(a, b, t);
        const T theta = std::acos(cosTheta);
        const T sinTheta = std::sin(theta);
        const T w1 = std::sin((T(1) - t) * theta) / sinTheta;
        const T w2 = std::sin(t * theta) / sinTheta;
        return { a.x * w1 + b.x * w2, a.y * w1 + b.y * w2, a.z * w1 + b.z * w2, a.w * w1 + b.w * w2 };
    }

    //  
    template <FLOP_t T>
    inline Quaternion<T> QIntegrateAngularVelocity(const Quaternion<T>& q, const Vector<T, 3>& omega, T dt) {
        // dq = 0.5 * (omega_quat) * q  omega_quat = ωx,ωy,ωz,0  
        Quaternion<T> wq{ omega[0], omega[1], omega[2], T(0) };
        auto dq = QMul(wq, q);
        Quaternion<T> qNext{
            q.x + dq.x * (T(0.5) * dt),
            q.y + dq.y * (T(0.5) * dt),
            q.z + dq.z * (T(0.5) * dt),
            q.w + dq.w * (T(0.5) * dt)
        };
        return QNormalize(qNext);
    }

 
    // XMQuaternionIdentity()
    inline Quat QuaternionIdentity() { return Quat::Identity(); }

    // XMQuaternionMultiply(a,b)
    inline Quat QuaternionMultiply(const Quat& a, const Quat& b) { return QMul(a, b); }

    // XMQuaternionNormalize(q)
    inline Quat QuaternionNormalize(const Quat& q) { return QNormalize(q); }

    // XMQuaternionRotationRollPitchYaw(pitch,yaw,roll)
    inline Quat QuaternionRotationRollPitchYaw(float pitch, float yaw, float roll) {
        return QFromRollPitchYaw<float>(pitch, yaw, roll);
    }

    // XMMatrixRotationQuaternion(q) ~ 4x4
    inline Float4x4 MatrixRotationQuaternion(const Quat& q) { return QToMat4(q); }

    // Quaternion ~ 3x3
    inline Float3x3 QuaternionToRotationMatrix(const Quat& q) { return QToMat3(q); }

}  
