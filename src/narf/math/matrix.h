#ifndef NARFBLOCK_MATRIX_H
#define NARFBLOCK_MATRIX_H

#include "narf/math/floats.h"
#include "narf/math/ints.h"
#include "narf/math/orientation.h"

namespace narf {

	template<class T>
	class Vector3;

	template<class T>
	class Matrix4x4 {
	public:
		T arr[16];

		Matrix4x4<T>() {}
		Matrix4x4<T>(
			T n0, T n1, T n2, T n3,
			T n4, T n5, T n6, T n7,
			T n8, T n9, T nA, T nB,
			T nC, T nD, T nE, T nF) {
			arr[0] = n0;
			arr[1] = n1;
			arr[2] = n2;
			arr[3] = n3;

			arr[4] = n4;
			arr[5] = n5;
			arr[6] = n6;
			arr[7] = n7;

			arr[8] = n8;
			arr[9] = n9;
			arr[10] = nA;
			arr[11] = nB;

			arr[12] = nC;
			arr[13] = nD;
			arr[14] = nE;
			arr[15] = nF;
		}

		static const Matrix4x4<T> identity() {
			return Matrix4x4<T>(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);
		}

		static const Matrix4x4<T> frustum(T left, T right, T bottom, T top, T zNear, T zFar) {
			Matrix4x4<T> f;
			auto n2 = zNear + zNear;
			f.arr[0] = n2 / (right - left);
			f.arr[1] = 0;
			f.arr[2] = 0;
			f.arr[3] = 0;

			f.arr[4] = 0;
			f.arr[5] = n2 / (top - bottom);
			f.arr[6] = 0;
			f.arr[7] = 0;

			f.arr[8] = (right + left) / (right - left);
			f.arr[9] = (top + bottom) / (top - bottom);
			f.arr[10] = -(zFar + zNear) / (zFar - zNear);
			f.arr[11] = -1;

			f.arr[12] = 0;
			f.arr[13] = 0;
			f.arr[14] = (-n2 * zFar) / (zFar - zNear);
			f.arr[15] = 0;

			return f;
		}

		static const Matrix4x4<T> rotate(T angle, T x, T y, T z) {
			auto u = Vector3<T>(x, y, z).normalize();
			T s = sin(angle);
			T c = cos(angle);
			auto v = u * (1 - c);

			Matrix4x4<T> r;
			r.arr[0] = v.x * u.x + c;
			r.arr[1] = v.x * u.y - (u.z * s);
			r.arr[2] = v.x * u.z + (u.y * s);
			r.arr[3] = 0;

			r.arr[4] = v.y * u.x + (u.z * s);
			r.arr[5] = v.y * u.y + c;
			r.arr[6] = v.y * u.z - (u.x * s);
			r.arr[7] = 0;

			r.arr[8] = v.z * u.x - (u.y * s);
			r.arr[9] = v.z * u.y + (u.x * s);
			r.arr[10] = v.z * u.z + c;
			r.arr[11] = 0;

			r.arr[12] = 0;
			r.arr[13] = 0;
			r.arr[14] = 0;
			r.arr[15] = 1;

			return r;
		}

		static const Matrix4x4<T> translate(T x, T y, T z) {
			return Matrix4x4<T>(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				x, y, z, 1);
		}

		const Matrix4x4<T> operator*(const Matrix4x4<T>& rhs) const {
			Matrix4x4<T> result;
			for (int c = 0; c < 4; c++) {
				for (int r = 0; r < 4; r++) {
					T sum = 0;
					for (int n = 0; n < 4; n++) {
						sum += arr[n * 4 + r] * rhs.arr[c * 4 + n];
					}
					result.arr[c * 4 + r] = sum;
				}
			}
			return result;
		}
	};

	typedef Matrix4x4<float> Matrix4x4f;
	typedef Matrix4x4<float> Matrix4f;

} // namespace narf

#endif // NARFBLOCK_MATRIX_H
