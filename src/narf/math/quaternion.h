#ifndef NARFBLOCK_QUATERNION_H
#define NARFBLOCK_QUATERNION_H

#include <cmath>

#include "narf/bytestream.h"
#include "narf/math/vector.h"
#include "narf/math/matrix.h"
#include "narf/math/orientation.h"

namespace narf {

	template<class T>
	class Matrix4x4;

	template<class T>
	class Orientation;

	template<class T>
		class Quaternion {
			public:

				T w;
				Vector3<T> v;

				Quaternion(T w, T x, T y, T z) : w(w), v(Vector3<T>(x, y, z)){
				}

				Quaternion(T w, Vector3<T> v) : w(w), v(v) {
				}

				Quaternion(Orientation<T> o) {
					// http://www.sedris.org/wg8home/Documents/WG80485.pdf
					T roll_cos = std::cos(o.roll / 2); // Bank
					T pitch_cos = std::cos(o.pitch / 2); // Attitude
					T yaw_cos = std::cos(o.yaw / 2); // Heading
					T roll_sin = std::sin(o.roll / 2);
					T pitch_sin = std::sin(o.pitch / 2);
					T yaw_sin = std::sin(o.yaw / 2);
					w = yaw_cos * roll_cos * pitch_cos + yaw_sin * roll_sin * pitch_sin;
					v.x = -yaw_cos * roll_cos * pitch_sin + yaw_sin * roll_sin * pitch_cos;
					v.y = -yaw_cos * roll_sin * pitch_cos - yaw_sin * roll_cos * pitch_sin;
					v.z = yaw_sin * roll_cos * pitch_cos - yaw_cos * roll_sin * pitch_sin;
				}

				Quaternion(Angle<T> angle, T x, T y, T z) : Quaternion(angle, Vector3<T>(x, y, z)) { }

				Quaternion(Angle<T> angle, Vector3<T> vec) : v(vec) {
					vec.normalizeSelf();
					w = std::cos(angle / 2);
					auto sin_a = std::sin(angle / 2);
					v.x = vec.x * sin_a;
					v.y = vec.y * sin_a;
					v.z = vec.z * sin_a;
					normalizeSelf();
				}

				Quaternion(Matrix4x4<T> mat) {
					T trace = mat.arr[0] + mat.arr[5] + mat.arr[10] + 1.0;
					T s, x, y, z;
					if (trace > 0) {
						s = 0.5 / std::sqrt(trace);
						x = (mat.arr[9] - mat.arr[6]) * s;
						y = (mat.arr[2] - mat.arr[8]) * s;
						z = (mat.arr[4] - mat.arr[1]) * s;
						w = 0.25 / s;
					} else {
						T c0 = mat.arr[0];
						T c1 = mat.arr[5];
						T c2 = mat.arr[10];
						if (c0 > c1 && c0 > c2) {
							s = std::sqrt(1.0 + mat.arr[0] - mat.arr[5] - mat.arr[10]) * 2;
							x = 0.5 / s;
							y = (mat.arr[1] + mat.arr[4]) / s;
							z = (mat.arr[2] + mat.arr[8]) / s;
							w = (mat.arr[6] + mat.arr[9]) / s;
						} else if (c1 > c0 && c1 > c2) {
							s = std::sqrt(1.0 + mat.arr[5] - mat.arr[0] - mat.arr[10]) * 2;
							x = (mat.arr[1] + mat.arr[4]) / s;
							y = 0.5 / s;
							z = (mat.arr[6] + mat.arr[9]) / s;
							w = (mat.arr[2] + mat.arr[8]) / s;
						} else if (c2 > c0 && c2 > c1) {
							s = std::sqrt(1.0 + mat.arr[10] - mat.arr[0] - mat.arr[5]) * 2;
							x = (mat.arr[2] + mat.arr[8]) / s;
							y = (mat.arr[6] + mat.arr[9]) / s;
							z = 0.5 / s;
							w = (mat.arr[1] + mat.arr[4]) / s;
						}
					}
					v = Vector3<T>(x, y, z);
				}

				Quaternion(ByteStreamReader& s) {
					s.readLE(&w);
					s.readLE(&(v.x));
					s.readLE(&(v.y));
					s.readLE(&(v.z));
				}

				void serialize(ByteStreamWriter& s) const {
					s.writeLE(w);
					s.writeLE(v.x);
					s.writeLE(v.y);
					s.writeLE(v.z);
				};

				const Quaternion<T> conjugate() const {
					return Quaternion<T>(w, -v);
				}

				T norm() const {
					return std::sqrt(w * w + v.dot(v));
				}

				void normalizeSelf() {
					T norm = this->norm();
					w /= norm;
					v /= norm;
				}

				const Quaternion<T> normalize() const {
					T norm = this->norm();
					if (norm == 0) {
						return *this;
					} else {
						return Quaternion<T>(w / norm, v / norm);
					}
				}

				const Quaternion<T> inverse() const {
					return conjugate() / (w * w + v.dot(v));
				}

				const Quaternion<T> operator+(const Quaternion<T> &add) const {
					return Quaternion<T>(w + add.w, v + add.v);
				}

				Quaternion<T> &operator +=(const Quaternion<T> &add) {
					w += add.w;
					v += add.v;
					return *this;
				}

				const Quaternion<T> operator-(const Quaternion<T> &sub) const {
					return Quaternion<T>(w - sub.w, v - sub.v);
				}

				Quaternion<T> &operator-=(const Quaternion<T> &sub) {
					w -= sub.w;
					v -= sub.v;
					return *this;
				}

				const Quaternion<T> operator*(const Quaternion<T> &mult) const {
					return Quaternion<T>(w * mult.w - v.dot(mult.v), mult.v * w + v * mult.w + v.cross(mult.v));
				}

				Quaternion<T> &operator*=(const Quaternion<T> &mult) {
					w = w * mult.w - v.dot(mult.v);
					v = mult.v * w + v * mult.w + v.cross(mult.v);
					return *this;
				}

				const Quaternion<T> operator/(const Quaternion<T> &div) const {
					return *this * div.inverse();
				}

				Quaternion<T> &operator/=(const Quaternion<T> &div) {
					*this *= div.inverse();
				}

				const Quaternion<T> operator/(T div) const {
					return Quaternion<T>(w / div, v / div);
				}

				Quaternion<T> &operator/=(T div) {
					w /= div;
					v /= div;
					return *this;
				}

				//const narf::Matrix4x4<T> toMatrix() const {
					//auto u = normalize();
					//return narf::Matrix4x4<T>(
					//1 - 2 * (u.y * u.y + u.z * u.z),     2 * (u.x * u.y - u.z * w),     2 * (u.x * u.z + u.y * w)  , 0,
							//2 * (u.x * u.y + u.z * w)  , 1 - 2 * (u.x * u.x + u.z * w),     2 * (u.y * u.z - u.x * w)  , 0,
							//2 * (u.x * u.z - u.y * w)  ,     2 * (u.y * u.z + u.x * w), 1 - 2 * (u.x * u.x + u.y * u.y), 0,
							//0,                         ,     0                        ,     0                          , 1);
				//};

		};

} // namespace narf

#endif // NARFBLOCK_QUATERNION_H
