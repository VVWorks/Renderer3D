#include <vector>
#include <string>
#include <fstream>
#include <strstream>

struct FaceR {
	int vert = 0, texture = 0, normal = 0;
};

FaceR GetFace(std::string &str) {
	FaceR rezult;

	int r[3] = { 0,0,0 };

	int index = 2, numIndex = 1, dsCount = 0;

	for (int i = str.length() - 1; i >= 0; i--) {
		if (str[i] == '/') {
			index--;
			numIndex = 1;

			dsCount++;

			if (index < 0) {
				i = -1;
			}
		}
		else {
			r[index] += (str[i] - 48) * numIndex;
			numIndex *= 10;
		}
	}

	if (r[1] == 0) {
		r[1] = r[2];
		r[2] = 0;
	}

	if (r[0] == 0) {
		r[0] = r[1];
		r[1] = r[2];
		r[2] = 0;
	}

	rezult.vert = r[0];
	rezult.texture = r[1];
	rezult.normal = r[2];

	return rezult;
}

struct Mesh {
	std::vector<triangle> tris;
	Image Texture;

	Mesh(int = 0) {

	}

	//only suports triangles, not polygons (add a triangulation function to solve!)
	bool loadMesh(std::string path) {

		std::ifstream f(path);
		if (!f.is_open())
			return false;

		std::vector<vec3d> verts;
		std::vector<vec2d> texs;
		std::string line;

		while (!f.eof()) {
			std::getline(f, line);

			std::strstream s;
			s << line;

			char junk;

			if (line != "" && line.length() >= 2) {
				if (line[0] == 'v' && line[1] == ' ') {
					vec3d v;
					s >> junk >> v.x >> v.y >> v.z;
					verts.push_back(v);
				}

				if (line[0] == 'v' && line[1] == 't') {
					vec2d v;
					s >> junk >> junk >> v.u >> v.v;
					texs.push_back(v);
				}

				if (line[0] == 'f' && line[1] == ' ') {
					FaceR f1, f2, f3;
					std::string s1, s2, s3;

					s >> junk >> s1 >> s2 >> s3;

					f1 = GetFace(s1);
					f2 = GetFace(s2);
					f3 = GetFace(s3);

					if (texs.size() != 0) {
						tris.push_back({ verts[f1.vert - 1], verts[f2.vert - 1], verts[f3.vert - 1], texs[f1.texture - 1], texs[f2.texture - 1], texs[f3.texture - 1], RGBc(255,255,255) });
					}
					else {
						tris.push_back({ verts[f1.vert - 1], verts[f2.vert - 1], verts[f3.vert - 1], 0,0,0, 0,0,0, 0,0,0, RGBc(255,255,255) });
					}
				}
			}
		}



		return true;
	}
};

struct Camera {
	vec3d cameraPos;
	vec3d lookDir;

	float fYaw;
	float fPitch;
};

struct Object
{
	Mesh mesh;
	vec3d pos;
	vec3d rotation;

	Object(int = 0) {}
	Object(Mesh& m, vec3d& p, vec3d& rot) {
		mesh = m;
		pos = p;
		rotation = rot;
	}
};