#include "maths.h"

#include <math.h>

#include <string.h>


Vec2 v2(float x) {
	Vec2 v;
	v.x = x;
	v.y = x;
	return v;
}
Vec3 v3(float x) {
	Vec3 v;
	v.x = x;
	v.y = x;
	v.z = x;
	return v;
}
Vec4 v4(float x) {
	Vec4 v;
	v.x = x;
	v.y = x;
	v.z = x;
	v.w = x;
	return v;
}



Vec2 v2(float x, float y) {
	Vec2 v;
	v.x = x;
	v.y = y;
	return v;
}
Vec3 v3(float x, float y, float z) {
	Vec3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}
Vec4 v4(float x, float y, float z, float w) {
	Vec4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}





Mat4 m4(float trace) {
	Mat4 mat;
	mat.m00 = trace;
	mat.m10 = 0;
	mat.m20 = 0;
	mat.m30 = 0;

	mat.m01 = 0;
	mat.m11 = trace;
	mat.m21 = 0;
	mat.m31 = 0;

	mat.m02 = 0;
	mat.m12 = 0;
	mat.m22 = trace;
	mat.m32 = 0;

	mat.m03 = 0;
	mat.m13 = 0;
	mat.m23 = 0;
	mat.m33 = trace;
	return mat;
}


Mat4 m4(float m00, float m10, float m20, float m30,
		float m01, float m11, float m21, float m31,
		float m02, float m12, float m22, float m32,
		float m03, float m13, float m23, float m33) {
	Mat4 mat;
	mat.m00 = m00;
	mat.m10 = m10;
	mat.m20 = m20;
	mat.m30 = m30;

	mat.m01 = m01;
	mat.m11 = m11;
	mat.m21 = m21;
	mat.m31 = m31;

	mat.m02 = m02;
	mat.m12 = m12;
	mat.m22 = m22;
	mat.m32 = m32;

	mat.m03 = m03;
	mat.m13 = m13;
	mat.m23 = m23;
	mat.m33 = m33;
	return mat;
}



Mat4 m4(float* m) {
	Mat4 mat;
	memcpy(mat.m, m, sizeof(mat.m));
	return mat;
}


// math



float m_sqrt(float x) {
	return sqrtf(x);
}
float m_sinf(float x) {
	return sinf(x * PI / 180);
}
float m_cosf(float x) {
    return cosf(x * PI / 180);
}
float m_tanf(float x) {
    return tanf(x * PI / 180);
}




float vec_length(Vec2 v) {
	return m_sqrt(v.x * v.x + v.y * v.y);
}

float vec_length(Vec3 v) {
	return m_sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vec_length(Vec4 v) {
	return m_sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}




Vec2 vec_add(Vec2 a, Vec2 b) {
	return v2(a.x + b.x, a.y + b.y);
}

Vec3 vec_add(Vec3 a, Vec3 b) {
	return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec4 vec_add(Vec4 a, Vec4 b) {
	return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}


Vec2 vec_sub(Vec2 a, Vec2 b) {
	return v2(a.x - b.x, a.y - b.y);
}

Vec3 vec_sub(Vec3 a, Vec3 b) {
	return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec4 vec_sub(Vec4 a, Vec4 b) {
	return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}


Vec2 vec_mul(Vec2 a, Vec2 b) {
	return v2(a.x * b.x, a.y * b.y);
}

Vec3 vec_mul(Vec3 a, Vec3 b) {
	return v3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec4 vec_mul(Vec4 a, Vec4 b) {
	return v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}


Vec2 vec_div(Vec2 a, Vec2 b) {
	return v2(a.x / b.x, a.y / b.y);
}

Vec3 vec_div(Vec3 a, Vec3 b) {
	return v3(a.x / b.x, a.y / b.y, a.z / b.z);
}

Vec4 vec_div(Vec4 a, Vec4 b) {
	return v4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}



Vec3 vec_cross(Vec3 a, Vec3 b) {
	return v3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}






float vec_dist(Vec2 a, Vec2 b) {
	return vec_length(vec_sub(a, b));
}
float vec_dist(Vec3 a, Vec3 b) {
	return vec_length(vec_sub(a, b));
}
float vec_dist(Vec4 a, Vec4 b) {
	return vec_length(vec_sub(a, b));
}

float vec_dot(Vec2 a, Vec2 b) {
	return a.x * b.x + a.y * b.y;
}
float vec_dot(Vec3 a, Vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
float vec_dot(Vec4 a, Vec4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vec2 vec_normalize(Vec2 a) {
	float l = vec_length(a);
	if (l == 0) l = 1;
	return vec_div(a, v2(l));
}
Vec3 vec_normalize(Vec3 a) {
	float l = vec_length(a);
	if (l == 0) l = 1;
	return vec_div(a, v3(l));
}
Vec4 vec_normalize(Vec4 a) {
	float l = vec_length(a);
	if (l == 0) l = 1;
	return vec_div(a, v4(l));
}




Vec4 vec_transform(Mat4 left, Vec4 right) {
	return v4(
			  vec_dot(left.rows[0], right),			  
			  vec_dot(left.rows[1], right),
			  vec_dot(left.rows[2], right),
			  vec_dot(left.rows[3], right)
			  );
}




Vec4 mat_column(Mat4 mat, s32 col) {
	return v4(
			  mat.m[0 + col],
			  mat.m[4 + col],
			  mat.m[8 + col],
			  mat.m[12 + col]
			  );
}


Mat4 mat_mul(Mat4 left, Mat4 right) {
	Mat4 result;
	for (u8 col = 0; col < 4; col++) {
		Vec4 col_dat = mat_column(right, col);
		for (u8 row = 0; row < 4; row++) {
			Vec4 row_dat = left.rows[row];
			result.m[col + row * 4] = vec_dot(row_dat, col_dat);
		}
	}
	return result;
}

Mat4 mat_transpose(Mat4 mat) {
	Mat4 result;
    for (u8 col = 0; col < 4; col++) {
        for (u8 row = 0; row < 4; row++) {
            result.m[col + row * 4] = mat.m[row + col * 4];
        }
    }
    return result;
}

Mat4 mat_inverse(Mat4 mat) {
    // NOTE:
    // we have to transpose the matrix because this function is based off
    // a stackoverflow result, and for some reason, everyone likes to use column major matrices
    // be we use row major, so we have to transpose it, which essentially makes our
    // matrix column major, because i am wayyy to lazy to go change the array indices to make the calculation row major
    // -sean 2020-02-08
    Mat4 input = mat_transpose(mat);
 
 	float* m = input.m;
 	float inv[16], det, inv_out[16];
 	int i;
 
 	inv[0] = m[5] * m[10] * m[15] -
 		m[5] * m[11] * m[14] -
 		m[9] * m[6] * m[15] +
 		m[9] * m[7] * m[14] +
 		m[13] * m[6] * m[11] -
 		m[13] * m[7] * m[10];
 
 	inv[4] = -m[4] * m[10] * m[15] +
 		m[4] * m[11] * m[14] +
 		m[8] * m[6] * m[15] -
 		m[8] * m[7] * m[14] -
 		m[12] * m[6] * m[11] +
 		m[12] * m[7] * m[10];
 
 	inv[8] = m[4] * m[9] * m[15] -
 		m[4] * m[11] * m[13] -
 		m[8] * m[5] * m[15] +
 		m[8] * m[7] * m[13] +
 		m[12] * m[5] * m[11] -
 		m[12] * m[7] * m[9];
 
 	inv[12] = -m[4] * m[9] * m[14] +
 		m[4] * m[10] * m[13] +
 		m[8] * m[5] * m[14] -
 		m[8] * m[6] * m[13] -
 		m[12] * m[5] * m[10] +
 		m[12] * m[6] * m[9];
 
 	inv[1] = -m[1] * m[10] * m[15] +
 		m[1] * m[11] * m[14] +
 		m[9] * m[2] * m[15] -
 		m[9] * m[3] * m[14] -
 		m[13] * m[2] * m[11] +
 		m[13] * m[3] * m[10];
 
 	inv[5] = m[0] * m[10] * m[15] -
 		m[0] * m[11] * m[14] -
 		m[8] * m[2] * m[15] +
 		m[8] * m[3] * m[14] +
 		m[12] * m[2] * m[11] -
 		m[12] * m[3] * m[10];
 
 	inv[9] = -m[0] * m[9] * m[15] +
 		m[0] * m[11] * m[13] +
 		m[8] * m[1] * m[15] -
 		m[8] * m[3] * m[13] -
 		m[12] * m[1] * m[11] +
 		m[12] * m[3] * m[9];
 
 	inv[13] = m[0] * m[9] * m[14] -
 		m[0] * m[10] * m[13] -
 		m[8] * m[1] * m[14] +
 		m[8] * m[2] * m[13] +
 		m[12] * m[1] * m[10] -
 		m[12] * m[2] * m[9];
 
 	inv[2] = m[1] * m[6] * m[15] -
 		m[1] * m[7] * m[14] -
 		m[5] * m[2] * m[15] +
 		m[5] * m[3] * m[14] +
 		m[13] * m[2] * m[7] -
 		m[13] * m[3] * m[6];
 
 	inv[6] = -m[0] * m[6] * m[15] +
 		m[0] * m[7] * m[14] +
 		m[4] * m[2] * m[15] -
 		m[4] * m[3] * m[14] -
 		m[12] * m[2] * m[7] +
 		m[12] * m[3] * m[6];
 
 	inv[10] = m[0] * m[5] * m[15] -
 		m[0] * m[7] * m[13] -
 		m[4] * m[1] * m[15] +
 		m[4] * m[3] * m[13] +
 		m[12] * m[1] * m[7] -
 		m[12] * m[3] * m[5];
 
 	inv[14] = -m[0] * m[5] * m[14] +
 		m[0] * m[6] * m[13] +
 		m[4] * m[1] * m[14] -
 		m[4] * m[2] * m[13] -
 		m[12] * m[1] * m[6] +
 		m[12] * m[2] * m[5];
 
 	inv[3] = -m[1] * m[6] * m[11] +
 		m[1] * m[7] * m[10] +
 		m[5] * m[2] * m[11] -
 		m[5] * m[3] * m[10] -
 		m[9] * m[2] * m[7] +
 		m[9] * m[3] * m[6];
 
 	inv[7] = m[0] * m[6] * m[11] -
 		m[0] * m[7] * m[10] -
 		m[4] * m[2] * m[11] +
 		m[4] * m[3] * m[10] +
 		m[8] * m[2] * m[7] -
 		m[8] * m[3] * m[6];
 
 	inv[11] = -m[0] * m[5] * m[11] +
 		m[0] * m[7] * m[9] +
 		m[4] * m[1] * m[11] -
 		m[4] * m[3] * m[9] -
 		m[8] * m[1] * m[7] +
 		m[8] * m[3] * m[5];
 
 	inv[15] = m[0] * m[5] * m[10] -
 		m[0] * m[6] * m[9] -
 		m[4] * m[1] * m[10] +
 		m[4] * m[2] * m[9] +
 		m[8] * m[1] * m[6] -
 		m[8] * m[2] * m[5];
 
 	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
 
 	if (det == 0) {
 		//TODO: something about stuff
		// ASSERT(false);
 	}
 
 	det = 1.0 / det;
 
 	for (i = 0; i < 16; i++)
 	    inv_out[i] = inv[i] * det;
 
	Mat4 result = m4(inv_out);
	// NOTE:
	// make our matrix back into row major
	// more info: see note at start of function
 	return mat_transpose(result);
}

Mat4 mat_translation(Vec3 translation) {
	return m4(1, 0, 0, translation.x,
			  0, 1, 0, translation.y,
			  0, 0, 1, translation.z,
			  0, 0, 0, 1);
}

Mat4 mat_rotation_axis_angle(Vec3 axis_in, float angle) {
	Vec3 axis = vec_normalize(axis_in);
	float s   = m_sinf(angle);
	float c   = m_cosf(angle);
	float oc  = 1 - c;

	return m4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
			  oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
			  oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
			  0.0, 0.0, 0.0, 1.0);

}

Mat4 mat_rotation_euler(Vec3 rotation) {
	Mat4 z_rot = mat_rotation_axis_angle(v3(0, 0, 1), rotation.z);
	Mat4 y_rot = mat_rotation_axis_angle(v3(0, 1, 0), rotation.y);
	Mat4 x_rot = mat_rotation_axis_angle(v3(1, 0, 0), rotation.x);
	
	return mat_mul(y_rot, mat_mul(x_rot, z_rot));
}

Mat4 mat_scale(Vec3 scale) {
	return m4(scale.x, 0, 0, 0,
			  0, scale.y, 0, 0,
			  0, 0, scale.z, 0,
			  0, 0, 0, 1);
}

Mat4 mat_project_ortho(float left, float right, float bottom, float top, float start, float end) {
	return m4(
		2 / (right - left), 0, 0, -(right + left) / (right - left),
		0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
		0, 0, 1 / (end - start), -start / (end - start),
		0, 0, 0, 1
	);
}

Mat4 mat_project_perspective(float fovy, float aspect, float start, float end) {
	float tfov2 = m_tanf(fovy / 2.0f);
	return m4(
		1.f / (aspect * tfov2), 0, 0, 0,
		0, 1.f / (tfov2), 0, 0,
		0, 0, (end) / (end - start), -(end * start) / (end - start),
		0, 0, 1, 0
	);
}
