#include "curve.h"
#include "vertexrecorder.h"
using namespace std;

const float c_pi = 3.14159265358979323846f;

namespace
{
// Approximately equal to.  We don't want to use == because of
// precision issues with floating point.
inline bool approx(const Vector3f& lhs, const Vector3f& rhs)
{
	const float eps = 1e-8f;
	return (lhs - rhs).absSquared() < eps;
}

inline bool isNearlyZero(const Vector3f& v)
{
	return v.absSquared() < 1e-12f;
}

inline Vector3f safeNormalized(const Vector3f& v)
{
	if (isNearlyZero(v))
	{
		return Vector3f(0, 0, 0);
	}
	return v.normalized();
}

inline bool isFlatXY(const vector<Vector3f>& points)
{
	for (size_t i = 0; i < points.size(); ++i)
	{
		if (fabs(points[i][2]) > 1e-6f)
		{
			return false;
		}
	}
	return true;
}

inline Vector3f chooseInitialBinormal(const Vector3f& tangent)
{
	Vector3f axis = Vector3f::cross(Vector3f(0, 0, 1), tangent);
	if (isNearlyZero(axis))
	{
		axis = Vector3f::cross(Vector3f(0, 1, 0), tangent);
	}
	if (isNearlyZero(axis))
	{
		axis = Vector3f::cross(Vector3f(1, 0, 0), tangent);
	}
	return axis.normalized();
}

inline float clampDot(float x)
{
	return max(-1.0f, min(1.0f, x));
}

inline float signedAngleAroundAxis(const Vector3f& from, const Vector3f& to, const Vector3f& axis)
{
	return atan2(Vector3f::dot(axis, Vector3f::cross(from, to)), clampDot(Vector3f::dot(from, to)));
}

inline void rotateFrameAroundTangent(CurvePoint& cp, float theta)
{
	const float c = cos(theta);
	const float s = sin(theta);
	const Vector3f rotatedN = (c * cp.N + s * cp.B).normalized();
	const Vector3f rotatedB = Vector3f::cross(cp.T, rotatedN).normalized();
	cp.N = rotatedN;
	cp.B = rotatedB;
}

void buildFrames(Curve& curve, bool forcePlanarFrames)
{
	if (curve.empty())
	{
		return;
	}

	for (size_t i = 0; i < curve.size(); ++i)
	{
		curve[i].T = safeNormalized(curve[i].T);
	}

	if (forcePlanarFrames)
	{
		for (size_t i = 0; i < curve.size(); ++i)
		{
			curve[i].B = Vector3f(0, 0, 1);
			curve[i].N = Vector3f::cross(curve[i].B, curve[i].T).normalized();
		}
		return;
	}

	curve[0].B = chooseInitialBinormal(curve[0].T);
	curve[0].N = Vector3f::cross(curve[0].B, curve[0].T).normalized();
	curve[0].B = Vector3f::cross(curve[0].T, curve[0].N).normalized();

	for (size_t i = 1; i < curve.size(); ++i)
	{
		Vector3f Bi = Vector3f::cross(curve[i].T, curve[i - 1].N);
		if (isNearlyZero(Bi))
		{
			Bi = curve[i - 1].B;
		}
		else
		{
			Bi.normalize();
		}

		Vector3f Ni = Vector3f::cross(Bi, curve[i].T);
		if (isNearlyZero(Ni))
		{
			Ni = curve[i - 1].N;
		}
		else
		{
			Ni.normalize();
		}

		curve[i].B = Vector3f::cross(curve[i].T, Ni).normalized();
		curve[i].N = Vector3f::cross(curve[i].B, curve[i].T).normalized();
	}

	const bool isClosed =
		curve.size() > 2 &&
		approx(curve.front().V, curve.back().V) &&
		clampDot(Vector3f::dot(curve.front().T, curve.back().T)) > 1.0f - 1e-4f;
	if (!isClosed)
	{
		return;
	}

	const float alpha = signedAngleAroundAxis(curve.back().N, curve.front().N, curve.back().T);
	if (fabs(alpha) < 1e-5f)
	{
		return;
	}

	for (size_t i = 0; i < curve.size(); ++i)
	{
		const float theta = alpha * static_cast<float>(i) / static_cast<float>(curve.size() - 1);
		rotateFrameAroundTangent(curve[i], theta);
	}
}

CurvePoint evalBezierPoint(const Vector3f& p0,
						   const Vector3f& p1,
						   const Vector3f& p2,
						   const Vector3f& p3,
						   float t)
{
	const float u = 1.0f - t;
	const float b0 = u * u * u;
	const float b1 = 3.0f * u * u * t;
	const float b2 = 3.0f * u * t * t;
	const float b3 = t * t * t;

	CurvePoint cp;
	cp.V = b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;
	cp.T = 3.0f * u * u * (p1 - p0)
	     + 6.0f * u * t * (p2 - p1)
	     + 3.0f * t * t * (p3 - p2);
	return cp;
}

}


Curve evalBezier(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	// TODO:
	// You should implement this function so that it returns a Curve
	// (e.g., a vector< CurvePoint >).  The variable "steps" tells you
	// the number of points to generate on each piece of the spline.
	// At least, that's how the sample solution is implemented and how
	// the SWP files are written.  But you are free to interpret this
	// variable however you want, so long as you can control the
	// "resolution" of the discretized spline curve with it.

	// Make sure that this function computes all the appropriate
	// Vector3fs for each CurvePoint: V,T,N,B.
	// [NBT] should be unit and orthogonal.

	// Also note that you may assume that all Bezier curves that you
	// receive have G1 continuity.  Otherwise, the TNB will not be
	// be defined at points where this does not hold.

	const unsigned segments = (P.size() - 1) / 3;
	Curve curve;
	curve.reserve(segments * steps + 1);

	for (unsigned seg = 0; seg < segments; ++seg)
	{
		const Vector3f& p0 = P[3 * seg + 0];
		const Vector3f& p1 = P[3 * seg + 1];
		const Vector3f& p2 = P[3 * seg + 2];
		const Vector3f& p3 = P[3 * seg + 3];

		for (unsigned i = 0; i <= steps; ++i)
		{
			if (seg > 0 && i == 0)
			{
				continue;
			}

			const float t = static_cast<float>(i) / static_cast<float>(steps);
			curve.push_back(evalBezierPoint(p0, p1, p2, p3, t));
		}
	}

	buildFrames(curve, isFlatXY(P));
	return curve;
}

Curve evalBspline(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4)
	{
		cerr << "evalBspline must be called with 4 or more control points." << endl;
		exit(0);
	}

	// TODO:
	// It is suggested that you implement this function by changing
	// basis from B-spline to Bezier.  That way, you can just call
	// your evalBezier function.

	vector<Vector3f> bezierCtrlPoints;
	bezierCtrlPoints.reserve(3 * (P.size() - 3) + 1);

	for (size_t i = 0; i + 3 < P.size(); ++i)
	{
		const Vector3f b0 = (P[i] + 4.0f * P[i + 1] + P[i + 2]) / 6.0f;
		const Vector3f b1 = (4.0f * P[i + 1] + 2.0f * P[i + 2]) / 6.0f;
		const Vector3f b2 = (2.0f * P[i + 1] + 4.0f * P[i + 2]) / 6.0f;
		const Vector3f b3 = (P[i + 1] + 4.0f * P[i + 2] + P[i + 3]) / 6.0f;

		if (i == 0)
		{
			bezierCtrlPoints.push_back(b0);
			bezierCtrlPoints.push_back(b1);
			bezierCtrlPoints.push_back(b2);
			bezierCtrlPoints.push_back(b3);
		}
		else
		{
			bezierCtrlPoints.push_back(b1);
			bezierCtrlPoints.push_back(b2);
			bezierCtrlPoints.push_back(b3);
		}
	}

	return evalBezier(bezierCtrlPoints, steps);
}

Curve evalCircle(float radius, unsigned steps)
{
	// This is a sample function on how to properly initialize a Curve
	// (which is a vector< CurvePoint >).

	// Preallocate a curve with steps+1 CurvePoints
	Curve R(steps + 1);

	// Fill it in counterclockwise
	for (unsigned i = 0; i <= steps; ++i)
	{
		// step from 0 to 2pi
		float t = 2.0f * c_pi * float(i) / steps;

		// Initialize position
		// We're pivoting counterclockwise around the y-axis
		R[i].V = radius * Vector3f(cos(t), sin(t), 0);

		// Tangent vector is first derivative
		R[i].T = Vector3f(-sin(t), cos(t), 0);

		// Normal vector is second derivative
		R[i].N = Vector3f(-cos(t), -sin(t), 0);

		// Finally, binormal is facing up.
		R[i].B = Vector3f(0, 0, 1);
	}

	return R;
}

void recordCurve(const Curve& curve, VertexRecorder* recorder)
{
	const Vector3f WHITE(1, 1, 1);
	for (int i = 0; i < (int)curve.size() - 1; ++i)
	{
		recorder->record_poscolor(curve[i].V, WHITE);
		recorder->record_poscolor(curve[i + 1].V, WHITE);
	}
}
void recordCurveFrames(const Curve& curve, VertexRecorder* recorder, float framesize)
{
	Matrix4f T;
	const Vector3f RED(1, 0, 0);
	const Vector3f GREEN(0, 1, 0);
	const Vector3f BLUE(0, 0, 1);
	
	const Vector4f ORGN(0, 0, 0, 1);
	const Vector4f AXISX(framesize, 0, 0, 1);
	const Vector4f AXISY(0, framesize, 0, 1);
	const Vector4f AXISZ(0, 0, framesize, 1);

	for (int i = 0; i < (int)curve.size(); ++i)
	{
		T.setCol(0, Vector4f(curve[i].N, 0));
		T.setCol(1, Vector4f(curve[i].B, 0));
		T.setCol(2, Vector4f(curve[i].T, 0));
		T.setCol(3, Vector4f(curve[i].V, 1));
 
		// Transform orthogonal frames into model space
		Vector4f MORGN  = T * ORGN;
		Vector4f MAXISX = T * AXISX;
		Vector4f MAXISY = T * AXISY;
		Vector4f MAXISZ = T * AXISZ;

		// Record in model space
		recorder->record_poscolor(MORGN.xyz(), RED);
		recorder->record_poscolor(MAXISX.xyz(), RED);

		recorder->record_poscolor(MORGN.xyz(), GREEN);
		recorder->record_poscolor(MAXISY.xyz(), GREEN);

		recorder->record_poscolor(MORGN.xyz(), BLUE);
		recorder->record_poscolor(MAXISZ.xyz(), BLUE);
	}
}
