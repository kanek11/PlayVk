#pragma once

#include "Render/DebugRay.h"

struct EmptyWS {}; //for empty collider, no shape


struct OBB {
	Float3 center;
	Float3 axis[3];
	Float3 halfExtents;
};

struct SphereWS { Float3 center; float radius; };
struct CapsuleWS { Float3 p0, p1; float radius; };

// dot(p,n)+d = 0
struct PlaneWS {
	Float3 normal;
	float d;
	float width;
	float height;
	Float3 center;
	Float3 right = { 1, 0, 0 };
	Float3 forward = { 0, 0, 1 };
};

using WorldShape = std::variant<EmptyWS, SphereWS, AABB, PlaneWS, OBB>;


inline void DrawDebugSphere(const SphereWS& sphere)
{
	uint32_t segment = 6;

	float radius = sphere.radius;
	Float3 center = sphere.center;

	for (uint32_t i = 0; i < segment; ++i) {
		float theta1 = (float)i / segment * DirectX::XM_2PI;
		float theta2 = (float)(i + 1) / segment * DirectX::XM_2PI;

		//z:
		Float3 p1z = center + Float3{ radius * cosf(theta1), radius * sinf(theta1), 0 };
		Float3 p2z = center + Float3{ radius * cosf(theta2), radius * sinf(theta2), 0 };
		DebugDraw::AddLine(p1z, p2z, Color::Green);

		//y:
		Float3 p1y = center + Float3{ 0, radius * cosf(theta1), radius * sinf(theta1) };
		Float3 p2y = center + Float3{ 0, radius * cosf(theta2), radius * sinf(theta2) };
		DebugDraw::AddLine(p1y, p2y, Color::Green);

		//x:
		Float3 p1x = center + Float3{ radius * cosf(theta1), 0, radius * sinf(theta1) };
		Float3 p2x = center + Float3{ radius * cosf(theta2), 0, radius * sinf(theta2) };
		DebugDraw::AddLine(p1x, p2x, Color::Green);
	}


}



inline void DrawDebugOBB(const OBB& box)
{
	Float3 corners[8];

	//top face
	corners[0] = box.center
		+ box.axis[0] * box.halfExtents.x()
		+ box.axis[1] * box.halfExtents.y()
		+ box.axis[2] * box.halfExtents.z();
	corners[1] = box.center
		- box.axis[0] * box.halfExtents.x()
		+ box.axis[1] * box.halfExtents.y()
		+ box.axis[2] * box.halfExtents.z();

	corners[2] = box.center
		+ box.axis[0] * box.halfExtents.x()
		+ box.axis[1] * box.halfExtents.y()
		- box.axis[2] * box.halfExtents.z();
	corners[3] = box.center
		- box.axis[0] * box.halfExtents.x()
		+ box.axis[1] * box.halfExtents.y()
		- box.axis[2] * box.halfExtents.z();

	//bottom face
	corners[4] = box.center
		+ box.axis[0] * box.halfExtents.x()
		- box.axis[1] * box.halfExtents.y()
		+ box.axis[2] * box.halfExtents.z();

	corners[5] = box.center
		- box.axis[0] * box.halfExtents.x()
		- box.axis[1] * box.halfExtents.y()
		+ box.axis[2] * box.halfExtents.z();

	corners[6] = box.center
		+ box.axis[0] * box.halfExtents.x()
		- box.axis[1] * box.halfExtents.y()
		- box.axis[2] * box.halfExtents.z();

	corners[7] = box.center
		- box.axis[0] * box.halfExtents.x()
		- box.axis[1] * box.halfExtents.y()
		- box.axis[2] * box.halfExtents.z();

	// Draw y edges
	for (int i = 0; i < 4; ++i) {
		DebugDraw::AddLine(corners[i], corners[i + 4], Color::Green);
	}

	for (int i = 0; i < 8; i += 4) {
		// Draw z edges:
		DebugDraw::AddLine(corners[i], corners[i + 2], Color::Green);
		DebugDraw::AddLine(corners[i + 1], corners[i + 3], Color::Green);

		// Draw x edges:
		DebugDraw::AddLine(corners[i], corners[i + 1], Color::Green);
		DebugDraw::AddLine(corners[i + 2], corners[i + 3], Color::Green);
	}

}


inline AABB ExpandFatAABB(const AABB& box, float pad) {
	return { box.min - Float3{pad,pad,pad}, box.max + Float3{pad,pad,pad} };
}

inline AABB MakeAABB(const Float3& c, float r) {
	AABB aabb = { c - Float3{ r,r,r }, c + Float3{ r,r,r } };
	
	return ExpandFatAABB(aabb, 0.01f);
}

inline AABB MakeAABB(const Float3& center, const Float3x3& R, const Float3& he) {
	// |R| * he  
	Float3 rx = { std::abs(R[0].x()), std::abs(R[1].x()), std::abs(R[2].x()) };
	Float3 ry = { std::abs(R[0].y()), std::abs(R[1].y()), std::abs(R[2].y()) };
	Float3 rz = { std::abs(R[0].z()), std::abs(R[1].z()), std::abs(R[2].z()) };
	Float3 radius = rx * he.x() + ry * he.y() + rz * he.z();

	AABB aabb = { center - radius, center + radius };
	return ExpandFatAABB(aabb, 0.01f);
} 


inline OBB MakeOBB(const AABB& box)
{
	OBB obb;
	obb.center = (box.min + box.max) * 0.5f;
	obb.halfExtents = (box.max - box.min) * 0.5f;
	obb.axis[0] = Float3{ 1, 0, 0 };
	obb.axis[1] = Float3{ 0, 1, 0 };
	obb.axis[2] = Float3{ 0, 0, 1 };
	return obb;
}



struct Interval {
	float min, max;
};

inline bool IntervalOverlap(Interval a, Interval b, float& outOverlap) {

	//if (a.max < b.min || a.min > b.max) return false;
	outOverlap = std::min(a.max, b.max) - std::max(a.min, b.min);

	if (outOverlap < 0.0f) {
		return false;
	}
	return true;
}

//project the OBB onto a given axis
//OBB , axis is on world space, but result is local to OBB frame;
inline Interval OBBProject(const OBB& obb, const Float3& axis) {

	float center = Dot(obb.center, axis);

	float radius = 0.0f;
	radius += obb.halfExtents.x() * std::abs(Dot(obb.axis[0], axis));
	radius += obb.halfExtents.y() * std::abs(Dot(obb.axis[1], axis));
	radius += obb.halfExtents.z() * std::abs(Dot(obb.axis[2], axis));

	return { center - radius, center + radius };
}

inline float PointProject(const Float3& point, const Float3& axis) {
	return Dot(point, axis);
}


inline float ClosestPoint(const Interval& interval, float inPoint) {

	return std::clamp(inPoint, interval.min, interval.max);
}

inline Float3 ClosestPoint(const AABB& box, const Float3& inPoint) {
	Float3 closestPoint{
		ClosestPoint({ box.min.x(), box.max.x() }, inPoint.x()),
		ClosestPoint({ box.min.y(), box.max.y() }, inPoint.y()),
		ClosestPoint({ box.min.z(), box.max.z() }, inPoint.z())
	};
	return closestPoint;
}



inline Float3 ClosestPoint(const OBB& box, const Float3& inPoint) {

	//project the point onto the OBB's axes
	Interval intervals[3];
	intervals[0] = OBBProject(box, box.axis[0]);
	intervals[1] = OBBProject(box, box.axis[1]);
	intervals[2] = OBBProject(box, box.axis[2]);

	Float3 closestPoint{};
	for (int i = 0; i < 3; ++i) {
		float closest = ClosestPoint(intervals[i], PointProject(inPoint, box.axis[i]));
		closestPoint += box.axis[i] * closest;
	}

	return closestPoint;
}

inline Float3 ClipClosestPoint(const OBB& box, const Float3& inPoint) {

	//project the point onto the OBB's axes
	Interval intervals[3];
	intervals[0] = OBBProject(box, box.axis[0]);
	intervals[1] = OBBProject(box, box.axis[1]);
	intervals[2] = OBBProject(box, box.axis[2]);

	Float3 closestPoint{};
	for (int i = 0; i < 3; ++i) {
		float proj = PointProject(inPoint, box.axis[i]);
		float closest = std::abs(proj - intervals[i].min) > std::abs(proj - intervals[i].max) ?
			intervals[i].max : intervals[i].min; // clip to the closest end
		closestPoint += box.axis[i] * closest;
	}

	return closestPoint;
}




inline float SignedDist(const PlaneWS& plane, const Float3& point) {

	return Dot(plane.normal, point) + plane.d;
}

inline Float3 ProjectToPlane(const PlaneWS& plane, const Float3& point) {
	//project a point onto the plane
	float dist = SignedDist(plane, point);
	return point - plane.normal * dist;
}


bool IsInsidePlane(const PlaneWS& plane, const Float3& point, float& dist)
{
	dist = SignedDist(plane, point);
	if (dist > 0.0f) {
		return false; // point is outside the plane
	}

	//check if a point is inside the plane's rectangle
	Float3 localVec = point - plane.center;
	float u = Dot(localVec, plane.right);
	float v = Dot(localVec, plane.forward);
	return std::abs(u) <= plane.width * 0.5f && std::abs(v) <= plane.height * 0.5f;
}


Float3 ClipToPlane(const PlaneWS& plane, const Float3& point)
{
	//dist = SignedDist(plane, point);
	//if (dist > 0.0f) {
	//	return false; // point is outside the plane
	//}

	//check if a point is inside the plane's rectangle
	Float3 localVec = point - plane.center;
	float u = Dot(localVec, plane.right);
	float v = Dot(localVec, plane.forward);

	//DebugDraw::AddRay(plane.center, plane.right * plane.width * 0.5f, Color::Green);
	//DebugDraw::AddRay(plane.center, plane.forward * plane.height * 0.5f, Color::Blue);

	Float3 clippedPoint =
		plane.center
		+ plane.right * std::clamp(u, -plane.width * 0.5f, plane.width * 0.5f)
		+ plane.forward * std::clamp(v, -plane.height * 0.5f, plane.height * 0.5f);

	return clippedPoint;
}


struct Segment {
	Float3 p0, p1;
};

inline float ProjectToSegment(const Segment& seg, const Float3& point)
{
	//project a point onto a segment
	Float3 segDir = seg.p1 - seg.p0;
	float lengthSq = Dot(segDir, segDir);
	if (lengthSq < 1e-8f) {
		//out = seg.p0; // degenerate segment, return p0
		return 0.0f;
	}
	float t = Dot(point - seg.p0, segDir) / lengthSq;
	t = std::clamp(t, 0.0f, 1.0f);
	//out = seg.p0 + t * segDir; // return the closest point on the segment
	return t;
}

inline std::pair<Float3, Float3> SegmentsClosest(const Segment& seg0, const Segment& seg1)
{
	const double EPS = 1e-8;

	using std::max; using std::min;

	//the formula is routine,so we don't care readability here.

	const Float3 p0 = seg0.p0, q0 = seg1.p0;
	const Float3 u = seg0.p1 - seg0.p0; // d0
	const Float3 v = seg1.p1 - seg1.p0; // d1
	const Float3 w0 = p0 - q0;
	const double uu = Dot(u, u);
	const double b = Dot(u, v);
	const double vv = Dot(v, v);
	const double d = Dot(u, w0);
	const double e = Dot(v, w0);



	//degenerate to point;
	if (uu <= EPS && vv <= EPS) {
		return { p0, q0 };
	}
	if (uu <= EPS) {
		// seg0 degen：s = 0，t = clamp(e/c)
		double s = 0.0;
		double t = (e) / vv;
		t = std::clamp(t, 0.0, 1.0);
		return { p0 + (float)s * u, q0 + (float)t * v }; // return q0 + t*v
	}
	else if (vv <= EPS) {
		// seg1 degen：t = 0，s = clamp(-d/a)
		double t = 0.0;
		double s = (-d) / uu;
		s = std::clamp(s, 0.0, 1.0);
		return { p0 + (float)s * u, q0 + (float)t * v }; // return p0 + s*u 
	}

	// general
	double det = uu * vv - b * b;
	// almost parallel / determinant near zero
	if (det <= EPS) {
		std::cerr << "Warning: we don't expect parallel segments.\n";
		return { p0 + (float)(d / uu) * u, q0 + (float)(e / vv) * v };
	}
	//generaal case
	else {
		double sN = (b * e - vv * d);
		double tN = (uu * e - b * d);

		//  s boundary
		float s = (float)sN / (float)det;
		float t = (float)tN / (float)det;

		if (s < 0.0) {
			s = 0.0;
			t = ProjectToSegment(seg1, p0);
		}
		else if (s > 1.0) {
			s = 1.0;
			t = ProjectToSegment(seg1, p0 + u);
		}

		if (t < 0.0) {
			t = 0.0;
			s = ProjectToSegment(seg0, q0);
		}
		else if (t > 1.0) {
			t = 1.0;
			s = ProjectToSegment(seg0, q0 + v);
		}


		const Float3 c0 = p0 + (float)s * u;
		const Float3 c1 = q0 + (float)t * v;
		return { c0, c1 };
	}




}