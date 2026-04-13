#include "surf.h"
#include "vertexrecorder.h"
using namespace std;

const float c_pi = 3.14159265358979323846f;

namespace
{
		
	// We're only implenting swept surfaces where the profile curve is
	// flat on the xy-plane.  This is a check function.
	static bool checkFlat(const Curve &profile)
	{
		for (unsigned i=0; i<profile.size(); i++)
			if (profile[i].V[2] != 0.0 ||
				profile[i].T[2] != 0.0 ||
				profile[i].N[2] != 0.0)
				return false;
		
		return true;
	}
}

// DEBUG HELPER
Surface quad() { 
	Surface ret;
	ret.VV.push_back(Vector3f(-1, -1, 0));
	ret.VV.push_back(Vector3f(+1, -1, 0));
	ret.VV.push_back(Vector3f(+1, +1, 0));
	ret.VV.push_back(Vector3f(-1, +1, 0));

	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));
	ret.VN.push_back(Vector3f(0, 0, 1));

	ret.VF.push_back(Tup3u(0, 1, 2));
	ret.VF.push_back(Tup3u(0, 2, 3));
	return ret;
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
	Surface surface;
		
	if (!checkFlat(profile))
	{
		cerr << "surfRev profile curve must be flat on xy plane." << endl;
		exit(0);
	}

	float dtheta = 2 * c_pi / steps;

	unsigned points = profile.size();

	vector< Vector3f > VV(points);
	vector< Vector3f > VN(points);

	for (unsigned i = 0; i < points; ++i)
	{
		VV[i] = profile[i].V;
		VN[i] = -profile[i].N;
	}

	for (unsigned i = 0; i < steps; ++i)
	{
		Matrix3f Ry(Vector3f( cos(dtheta),            0, -sin(dtheta)),
					Vector3f(           0,            1,            0),
					Vector3f( sin(dtheta),            0,  cos(dtheta)));
		Matrix3f Ry_inv(Ry);

		vector< Vector3f > new_VV(points);
		vector< Vector3f > new_VN(points);

		vector< Tup3u > VF;

		unsigned len = surface.VV.size();

		for (unsigned j = 0; j < points; ++j)
		{
			new_VV[j] = Ry * VV[j];
			new_VN[j] = (Ry_inv * VN[j]).normalized();

			if (j < points - 1)
			{
				Tup3u tri_1, tri_2;
				if (i < steps - 1)
				{
					tri_1 = Tup3u(len + j, len + j + 1, len + points + j);
					tri_2 = Tup3u(len + points + j, len + j + 1, len + points + j + 1);
				}
				else
				{
					tri_1 = Tup3u(len + j, len + j + 1, j);
					tri_2 = Tup3u(j, len + j + 1, j + 1);
				}
				VF.push_back(tri_1);
				VF.push_back(tri_2);
			}
		}

		surface.VV.insert(surface.VV.end(), VV.begin(), VV.end());
		surface.VN.insert(surface.VN.end(), VN.begin(), VN.end());
		surface.VF.insert(surface.VF.end(), VF.begin(), VF.end());

		VV = new_VV;
		VN = new_VN;
	}

	return surface;
}

inline static bool approx(const Vector3f& lhs, const Vector3f& rhs)
{
	const float eps = 1e-8f;
	return (lhs - rhs).absSquared() < eps;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
	Surface surface;

	if (!checkFlat(profile))
	{
		cerr << "genCyl profile curve must be flat on xy plane." << endl;
		exit(0);
	}

	unsigned points = profile.size();

	vector< Vector3f > orig_VV(points);
	vector< Vector3f > orig_VN(points);

	for (unsigned i = 0; i < points; ++i)
	{
		orig_VV[i] = profile[i].V;
	orig_VN[i] = -profile[i].N;
	}

	float alpha = 0;
	if (approx(sweep[0].V, sweep[sweep.size() - 1].V))
	{
		if (approx(sweep[0].T, sweep[sweep.size() - 1].T))
		{
			Vector3f a(sweep[0].N);
			Vector3f b(sweep[sweep.size() - 1].N);

			Vector3f cross_prod = Vector3f::cross(a, b);
			float dot_prod_1 = Vector3f::dot(cross_prod, sweep[0].T);
			float dot_prod_2 = Vector3f::dot(a, b);

			alpha = atan2(dot_prod_1, dot_prod_2);
		}
	}

	float dtheta = -alpha / (sweep.size() - 1);

	for (unsigned i = 0; i < sweep.size(); ++i)
	{
		float theta = dtheta * i;

		const CurvePoint& Point = sweep[i];

		Matrix4f M(Vector4f( cos(theta) * Point.N + sin(theta) * Point.B, 0),
				   Vector4f(-sin(theta) * Point.N + cos(theta) * Point.B, 0),
				   Vector4f(                                     Point.T, 0),
				   Vector4f(                                     Point.V, 1));
		Matrix4f M_inv = M.inverse().transposed();

		vector< Vector3f > VV(points);
		vector< Vector3f > VN(points);
		vector< Tup3u > VF;

		unsigned len = surface.VV.size();

		for (unsigned j = 0; j < points; ++j)
		{
			VV[j] = (M * Vector4f(orig_VV[j], 1)).xyz();
			VN[j] = (M_inv * Vector4f(orig_VN[j], 0)).xyz().normalized();

			if (j < points - 1)
			{
				Tup3u tri_1, tri_2;
				if (i < sweep.size() - 1)
				{
					tri_1 = Tup3u(len + j, len + j + 1, len + points + j);
					tri_2 = Tup3u(len + points + j, len + j + 1, len + points + j + 1);
				}
				else
				{
					tri_1 = Tup3u(len + j, len + j + 1, j);
					tri_2 = Tup3u(j, len + j + 1, j + 1);
				}
				VF.push_back(tri_1);
				VF.push_back(tri_2);
			}
		}

		surface.VV.insert(surface.VV.end(), VV.begin(), VV.end());
		surface.VN.insert(surface.VN.end(), VN.begin(), VN.end());
		surface.VF.insert(surface.VF.end(), VF.begin(), VF.end());
	}

	return surface;
}

void recordSurface(const Surface &surface, VertexRecorder* recorder) {
	const Vector3f WIRECOLOR(0.4f, 0.4f, 0.4f);
	for (int i=0; i<(int)surface.VF.size(); i++)
	{
		recorder->record(surface.VV[surface.VF[i][0]], surface.VN[surface.VF[i][0]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][1]], surface.VN[surface.VF[i][1]], WIRECOLOR);
		recorder->record(surface.VV[surface.VF[i][2]], surface.VN[surface.VF[i][2]], WIRECOLOR);
	}
}

void recordNormals(const Surface &surface, VertexRecorder* recorder, float len)
{
	const Vector3f NORMALCOLOR(0, 1, 1);
	for (int i=0; i<(int)surface.VV.size(); i++)
	{
		recorder->record_poscolor(surface.VV[i], NORMALCOLOR);
		recorder->record_poscolor(surface.VV[i] + surface.VN[i] * len, NORMALCOLOR);
	}
}

void outputObjFile(ostream &out, const Surface &surface)
{
		
	for (int i=0; i<(int)surface.VV.size(); i++)
		out << "v  "
			<< surface.VV[i][0] << " "
			<< surface.VV[i][1] << " "
			<< surface.VV[i][2] << endl;

	for (int i=0; i<(int)surface.VN.size(); i++)
		out << "vn "
			<< surface.VN[i][0] << " "
			<< surface.VN[i][1] << " "
			<< surface.VN[i][2] << endl;

	out << "vt  0 0 0" << endl;
		
	for (int i=0; i<(int)surface.VF.size(); i++)
	{
		out << "f  ";
		for (unsigned j=0; j<3; j++)
		{
			unsigned a = surface.VF[i][j]+1;
			out << a << "/" << "1" << "/" << a << " ";
		}
		out << endl;
	}
}
