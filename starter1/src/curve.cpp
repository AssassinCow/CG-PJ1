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


}


Curve evalBezier(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	unsigned segments = P.size() / 3;
	
	Curve R(segments * steps + 1);

	const Matrix4f M(Vector4f( 1,  0,  0,  0),
					 Vector4f(-3,  3,  0,  0),
					 Vector4f( 3, -6,  3,  0),
					 Vector4f(-1,  3, -3,  1));

	float dt = 1.0 / steps;
	for (unsigned i = 0; i < segments; ++i)
	{
		float t = 0;
		Matrix4f G(Vector4f(P[i * 3], 0),
				   Vector4f(P[i * 3 + 1], 0),
				   Vector4f(P[i * 3 + 2], 0),
				   Vector4f(P[i * 3 + 3], 0));
		for (unsigned j = 0; j <= steps; ++j)
		{
			if (i < segments - 1 && j == steps)
			{
				continue;
			}
			Vector4f T(1, t, t * t, t * t * t);
			Vector4f dT(0, 1, 2 * t, 3 * t * t);
			CurvePoint& Point = R[i * steps + j];
			Point.V = (G * M * T).xyz();
			Point.T = (G * M * dT).xyz().normalized();
			if (i == 0 && j == 0)
			{
				Vector3f B0(0, 0, 1);
				Point.N = Vector3f::cross(B0, Point.T).normalized();
			}
			else
			{
				Point.N = Vector3f::cross(R[i * steps + j - 1].B, Point.T).normalized();
			}
			Point.B = Vector3f::cross(Point.T, Point.N).normalized();

			t += dt;
		}
	}

	return R;
}

Curve evalBspline(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4)
	{
		cerr << "evalBspline must be called with 4 or more control points." << endl;
		exit(0);
	}

	Curve R;
	R.reserve((P.size() - 3) * steps + 1);

	const Matrix4f M(Vector4f( 1,  4,  1,  0),
					 Vector4f(-3,  0,  3,  0),
					 Vector4f( 3, -6,  3,  0),
					 Vector4f(-1,  3, -3,  1));

	const Matrix4f Mbez_inv(Vector4f(3, 0, 0, 0),
							Vector4f(3, 1, 0, 0),
							Vector4f(3, 2, 1, 0),
							Vector4f(3, 3, 3, 3));

	for (unsigned i = 3; i < P.size(); ++i)
	{
		Matrix4f G(Vector4f(P[i - 3], 0),
				   Vector4f(P[i - 2], 0),
				   Vector4f(P[i - 1], 0),
				   Vector4f(P[i], 0));
		
		Matrix4f G_trans(G * M * Mbez_inv);

		vector< Vector3f > P_trans(4);
		P_trans[0] = G_trans.getCol(0).xyz() / 18.0f;
		P_trans[1] = G_trans.getCol(1).xyz() / 18.0f;
		P_trans[2] = G_trans.getCol(2).xyz() / 18.0f;
		P_trans[3] = G_trans.getCol(3).xyz() / 18.0f;
		
		Curve seg = evalBezier(P_trans, steps);

		if (i == P.size() - 1)
		{
			R.insert(R.end(), seg.begin(), seg.end());
		}
		else
		{
			R.insert(R.end(), seg.begin(), seg.end() - 1);
		}
	}

	// Since curve R is composed of separate B'ezier curves,
	// re-calc N & B to make sure they're continuous
	for (unsigned i = 0; i < R.size(); i++)
	{
		CurvePoint& Point = R[i];

		if (i == 0)
		{
			Vector3f B0(0, 0, 1);
			Point.N = Vector3f::cross(B0, Point.T).normalized();
		}
		else
		{
			Point.N = Vector3f::cross(R[i - 1].B, Point.T).normalized();
		}
		Point.B = Vector3f::cross(Point.T, Point.N).normalized();
	}

	return R;
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

