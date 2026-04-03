#include "surf.h"
#include "vertexrecorder.h"
using namespace std;

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

	    inline unsigned vertexIndex(unsigned ring, unsigned sample, unsigned ringSize)
	    {
	        return ring * ringSize + sample;
	    }

	    void addTriangleStripFaces(Surface& surface, unsigned rings, unsigned ringSize)
	    {
	        for (unsigned r = 0; r + 1 < rings; ++r)
	        {
	            for (unsigned i = 0; i + 1 < ringSize; ++i)
	            {
	                const unsigned a = vertexIndex(r, i, ringSize);
	                const unsigned b = vertexIndex(r + 1, i, ringSize);
	                const unsigned c = vertexIndex(r, i + 1, ringSize);
	                const unsigned d = vertexIndex(r + 1, i + 1, ringSize);
	                surface.VF.push_back(Tup3u(a, c, b));
	                surface.VF.push_back(Tup3u(b, c, d));
	            }
	        }
	    }

	    Matrix3f frameRotation(const CurvePoint& frame)
	    {
	        Matrix3f rotation;
	        rotation.setCol(0, frame.N);
	        rotation.setCol(1, frame.B);
	        rotation.setCol(2, frame.T);
	        return rotation;
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

    const unsigned ringSize = profile.size();
    const unsigned rings = steps + 1;
    surface.VV.reserve(rings * ringSize);
    surface.VN.reserve(rings * ringSize);

    for (unsigned r = 0; r <= steps; ++r)
    {
        const float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(r) / static_cast<float>(steps);
        const Matrix3f rotation = Matrix3f::rotateY(theta);

        for (unsigned i = 0; i < ringSize; ++i)
        {
            surface.VV.push_back(rotation * profile[i].V);
            surface.VN.push_back((rotation * (-profile[i].N)).normalized());
        }
    }

    addTriangleStripFaces(surface, rings, ringSize);
    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;

    if (!checkFlat(profile))
    {
        cerr << "genCyl profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    const unsigned ringSize = profile.size();
    const unsigned rings = sweep.size();
    surface.VV.reserve(rings * ringSize);
    surface.VN.reserve(rings * ringSize);

    for (unsigned r = 0; r < rings; ++r)
    {
        const Matrix3f rotation = frameRotation(sweep[r]);
        for (unsigned i = 0; i < ringSize; ++i)
        {
            surface.VV.push_back(sweep[r].V + rotation * profile[i].V);
            surface.VN.push_back((rotation * profile[i].N).normalized());
        }
    }

    addTriangleStripFaces(surface, rings, ringSize);
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
