#include <math.h>
#include <vector>

struct RGBc {
	int r;
	int g;
	int b;

	RGBc(int = 0) {
		r = 0; g = 0; b = 0;
	}

	RGBc(int r_i, int g_i, int b_i) {
		r = r_i; g = g_i; b = g_i;
	}
};

struct vec2d {
	float u = 0, v = 0, w = 1;
};

struct vec3d{
	float x, y, z, w;

	vec3d(int = 0) {
		x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f;
	}

	vec3d(float x0, float y0, float z0) {
		x = x0; y = y0; z = z0; w = 1.0f;
	}

	vec3d getNormal() {
		float h = sqrt(x * x + y * y + z * z);
		return vec3d(x / h, y / h, z / h);
	}

	void Normalize() {
		float h = sqrt(x * x + y * y + z * z);
		x /= h; y /= h; z /= h;
	}

	vec3d GetInverse() {
		return vec3d(-x, -y, -z);
	}

	vec3d operator += (const float& add) {//adition
		x += add; y += add; z += add;

		return *this;
	}

	vec3d operator -= (const float& add) {
		x -= add; y -= add; z -= add;

		return *this;
	}

	vec3d operator + (const float& add) {//adition
		return (x + add, y + add, z + add);
	}

	vec3d operator - (const float& add) {
		return (x - add, y - add, z - add);
	}

	vec3d operator * (const float& scalar) {//scalar muliplication
		return vec3d(x * scalar, y * scalar, z * scalar);
	}

	vec3d operator *=(const float& scalarThis) {
		x *= scalarThis; y *= scalarThis; z *= scalarThis;

		return *this;
	}

	vec3d operator + (const vec3d& other) {//vector adition
		return vec3d(x + other.x, y + other.y, z + other.z);
	}

	vec3d operator - (const vec3d& other) {
		return vec3d(x - other.x, y - other.y, z - other.z);
	}

	vec3d operator +=(const vec3d& other) {
		x += other.x; y += other.y; z += other.z;

		return *this;
	}

	vec3d operator -=(const vec3d& other) {
		x -= other.x; y -= other.y; z -= other.z;

		return *this;
	}

	vec3d operator /=(const vec3d& other) {
		x /= other.x; y /= other.y; z /= other.z;

		return *this;
	}

	vec3d operator *(const vec3d& other) {//cross product
		return vec3d(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}
};

float DotProduct(const vec3d& a, const vec3d& b) {//dot product
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct triangle{
	vec3d p[3];
	vec2d texture[3];

	RGBc color;

	triangle operator += (const float& other) {
		p[0] += other;
		p[1] += other;
		p[3] += other;

		return *this;
	}

	inline triangle operator -= (const float& other) {
		p[0] -= other;
		p[1] -= other;
		p[3] -= other;

		return *this;
	}
};

struct matrix4x4 {
	float m[4][4] = { 0 };

	matrix4x4 operator *(const matrix4x4& mat) {
		matrix4x4 r;
		for (int c = 0; c < 4; c++) {
			for (int k = 0; k < 4; k++) {
				r.m[k][c] = m[k][0] * mat.m[0][c] + m[k][1] * mat.m[1][c] + m[k][2] * mat.m[2][c] + m[k][3] * mat.m[3][c];
			}
		}

		return r;
	}
};

void RotateZ(float Angle, matrix4x4& o) {
	o.m[0][0] = cosf(Angle);
	o.m[0][1] = sinf(Angle);
	o.m[1][0] = -sinf(Angle);
	o.m[1][1] = cosf(Angle);
	o.m[2][2] = 1;
	o.m[3][3] = 1;
}

void RotateX(float Angle, matrix4x4& o) {
	o.m[0][0] = 1;
	o.m[1][1] = cosf(Angle );
	o.m[1][2] = sinf(Angle );
	o.m[2][1] = -sinf(Angle );
	o.m[2][2] = cosf(Angle );
	o.m[3][3] = 1;
}

void RotateY(float Angle, matrix4x4& o) {
	o.m[0][0] = cosf(Angle);
	o.m[0][2] = sinf(Angle);
	o.m[2][0] = -sinf(Angle);
	o.m[1][1] = 1.0f;
	o.m[2][2] = cosf(Angle);
	o.m[3][3] = 1.0f;
}

void MultiplyMatrixVector(vec3d& i, vec3d& o, matrix4x4& m) {
	o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
	o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
	o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
	float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

	if (w != 0.0f) {
		o.x /= w;
		o.y /= w;
		o.z /= w;
	}
}

vec3d Matrix_MuliplyVector(matrix4x4& m, vec3d& i) {
	vec3d v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}

matrix4x4 Matrix_Translation(float x, float y, float z) {
	matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;

	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;

	return matrix;
}

matrix4x4 Matrix_identity() {
	matrix4x4 mat;
	mat.m[0][0] = 1.0f;
	mat.m[1][1] = 1.0f;
	mat.m[2][2] = 1.0f;
	mat.m[3][3] = 1.0f;
	return mat;
}

matrix4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up) {
	vec3d newForward = target - pos;
	newForward.Normalize();
	
	vec3d a = newForward * DotProduct(up, newForward);
	vec3d newUp = up - a;
	newUp.Normalize();

	vec3d newRight = newUp * newForward;

	matrix4x4 matrix;
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;
}

matrix4x4 Matrix_PseudeInverse(matrix4x4& m) {//only rotation/translation matrices
	matrix4x4 matrix;
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd, float &t_out)
{
	plane_n.Normalize();
	float plane_d = -DotProduct(plane_n, plane_p);
	float ad = DotProduct(lineStart, plane_n);
	float bd = DotProduct(lineEnd, plane_n);
	t_out = (-plane_d - ad) / (bd - ad);
	vec3d lineStartToEnd = lineEnd - lineStart;
	vec3d lineToIntersect = lineStartToEnd * t_out;
	return lineStart + lineToIntersect;
}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
{
	plane_n.Normalize();

	auto dist = [&](vec3d& p)
	{
		vec3d n = p.getNormal();
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - DotProduct(plane_n, plane_p));
	};
	vec3d* inside_points[3];  int nInsidePointCount = 0;
	vec3d* outside_points[3]; int nOutsidePointCount = 0;
	vec2d* inside_tex[3]; int nInsideTexCount = 0;
	vec2d* outside_tex[3]; int nOutsideTexCount = 0;


	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; inside_tex[nInsideTexCount++] = &in_tri.texture[0]; }
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[0]; outside_tex[nOutsideTexCount++] = &in_tri.texture[0];
	}
	if (d1 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri.p[1]; inside_tex[nInsideTexCount++] = &in_tri.texture[1];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[1];  outside_tex[nOutsideTexCount++] = &in_tri.texture[1];
	}
	if (d2 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri.p[2]; inside_tex[nInsideTexCount++] = &in_tri.texture[2];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[2];  outside_tex[nOutsideTexCount++] = &in_tri.texture[2];
	}

	if (nInsidePointCount == 0)
	{
		return 0; 
	}

	if (nInsidePointCount == 3)
	{
		out_tri1 = in_tri;

		return 1; 
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		out_tri1.p[0] = *inside_points[0];
		out_tri1.texture[0] = *inside_tex[0];

		float t;
		out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.texture[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.texture[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.texture[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

		out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
		out_tri1.texture[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.texture[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.texture[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;

		return 1;
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		out_tri1.color = in_tri.color;

		out_tri2.color = in_tri.color;

		out_tri1.p[0] = *inside_points[0];
		out_tri1.p[1] = *inside_points[1];
		out_tri1.texture[0] = *inside_tex[0];
		out_tri1.texture[1] = *inside_tex[1];

		float t;
		out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.texture[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.texture[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.texture[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

		out_tri2.p[0] = *inside_points[1];
		out_tri2.texture[0] = *inside_tex[1];
		out_tri2.p[1] = out_tri1.p[2];
		out_tri2.texture[1] = out_tri1.texture[2];
		out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
		out_tri2.texture[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
		out_tri2.texture[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
		out_tri2.texture[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;
		return 2;
	}
}


