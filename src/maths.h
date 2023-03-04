#pragma once

#include "std.h"
#define PI 3.14159265358979

struct Vec2 {
	float x;
	float y;
};

Vec2 v2(float x);
Vec2 v2(float x, float y);


struct Vec3 {
	float x;
	float y;
	float z;
};

Vec3 v3(float x);
Vec3 v3(float x, float y, float z);


struct Vec4 {
	float x;
	float y;
	float z;
	float w;
};

Vec4 v4(float x);
Vec4 v4(float x, float y, float z, float w);



struct Mat4 {
	union {
		float m[4 * 4];
		Vec4 rows[4];
		struct {
			float m00;
			float m10;
			float m20;
			float m30;

			float m01;
			float m11;
			float m21;
			float m31;
			
			float m02;
			float m12;
			float m22;
			float m32;
			
			float m03;
			float m13;
			float m23;
			float m33;
		};
	};
};

Mat4 m4(float trace);
Mat4 m4(float* m);
Mat4 m4(float m00, float m10, float m20, float m30,
		float m01, float m11, float m21, float m31,
		float m02, float m12, float m22, float m32,
		float m03, float m13, float m23, float m33);


// math

float m_sqrt(float x);
float m_sinf(float x);
float m_cosf(float x);
float m_tanf(float x);



float vec_length(Vec2 v);
float vec_length(Vec3 v);
float vec_length(Vec4 v);

Vec2 vec_add(Vec2 a, Vec2 b);
Vec3 vec_add(Vec3 a, Vec3 b);
Vec4 vec_add(Vec4 a, Vec4 b);

Vec2 vec_sub(Vec2 a, Vec2 b);
Vec3 vec_sub(Vec3 a, Vec3 b);
Vec4 vec_sub(Vec4 a, Vec4 b);

Vec2 vec_mul(Vec2 a, Vec2 b);
Vec3 vec_mul(Vec3 a, Vec3 b);
Vec4 vec_mul(Vec4 a, Vec4 b);

Vec2 vec_div(Vec2 a, Vec2 b);
Vec3 vec_div(Vec3 a, Vec3 b);
Vec4 vec_div(Vec4 a, Vec4 b);

Vec3 vec_cross(Vec3 a, Vec3 b);

float vec_dist(Vec2 a, Vec2 b);
float vec_dist(Vec3 a, Vec3 b);
float vec_dist(Vec4 a, Vec4 b);

float vec_dot(Vec2 a, Vec2 b);
float vec_dot(Vec3 a, Vec3 b);
float vec_dot(Vec4 a, Vec4 b);

Vec2 vec_normalize(Vec2 a);
Vec3 vec_normalize(Vec3 a);
Vec4 vec_normalize(Vec4 a);


Vec4 vec_transform(Mat4 left, Vec4 right);

Vec4 mat_column(Mat4 mat, s32 col);

Mat4 mat_mul(Mat4 left, Mat4 right);
Mat4 mat_transpose(Mat4 mat);
Mat4 mat_inverse(Mat4 mat);

Mat4 mat_translation(Vec3 translation);
Mat4 mat_rotation_axis_angle(Vec3 axis_in, float angle);
Mat4 mat_rotation_euler(Vec3 rotation);
Mat4 mat_scale(Vec3 scale);

Mat4 mat_project_ortho(float left, float right, float bottom, float top, float start, float end);
Mat4 mat_project_perspective(float fovy, float aspect, float start, float end);
