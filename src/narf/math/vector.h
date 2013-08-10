#ifndef NARFBLOCK_VECTOR_H
#define NARFBLOCK_VECTOR_H

#include <math.h>

namespace narf {
namespace math {

	template<class T>
		class Vector2 {
			public:

				T x, y;

				Vector2() : x(0.0f), y(0.0f) { }
				Vector2(T x, T y) : x(x), y(y) { }

				T length() const
				{
					return sqrt(x * x + y * y);
				}

				const Vector2<T> normalize() const
				{
					T length = this->length();
					if (length == 0) {
						return *this;
					} else {
						return Vector2<T>(x / length, y / length);
					}
				}

				const Vector2<T> operator+(const Vector2<T> &add) const
				{
					return Vector2<T>(x + add.x, y + add.y);
				}

				Vector2<T> &operator +=(const Vector2<T> &add)
				{
					x += add.x;
					y += add.y;
					return *this;
				}

				const Vector2<T> operator-(const Vector2<T> &sub) const
				{
					return Vector2<T>(x - sub.x, y - sub.y);
				}

				Vector2<T> &operator-=(const Vector2<T> &sub)
				{
					x -= sub.x;
					y -= sub.y;
					return *this;
				}

				const Vector2<T> operator*(T v) const
				{
					return Vector2<T>(x * v, y * v);
				}

				T dot(Vector2<T> vec) const {
					return x * vec.x + y * vec.y;
				}

		};

	template<class T>
		class Vector3 {
			public:

				T x, y, z;

				Vector3() : x(0), y(0), z(0) { }
				Vector3(T x, T y, T z) : x(x), y(y), z(z) { }

				T length() const {
					return sqrtf(x * x + y * y + z * z);
				}

				const Vector3<T> normalize() const {
					T length = this->length();
					if (length == 0) {
						return *this;
					} else {
						return Vector3<T>(x / length, y / length, z / length);
					}
				}

				const Vector3<T> operator+(const Vector3<T> &add) const {
					return Vector3<T>(x + add.x, y + add.y, z + add.z);
				}

				Vector3<T> &operator +=(const Vector3<T> &add) {
					x += add.x;
					y += add.y;
					z += add.z;
					return *this;
				}

				const Vector3<T> operator-(const Vector3<T> &sub) const {
					return Vector3<T>(x - sub.x, y - sub.y, z - sub.z);
				}

				Vector3<T> &operator-=(const Vector3<T> &sub) {
					x -= sub.x;
					y -= sub.y;
					z -= sub.z;
					return *this;
				}

				const Vector3<T> operator*(T v) const {
					return Vector3<T>(x * v, y * v, z * v);
				}

				T dot(Vector3<T> vec) const {
					return x * vec.x + y * vec.y + z * vec.z;
				}

				const Vector3<T> cross(Vector3<T> vec) const {
					return Vector3<T>(
							y * vec.z - z * vec.y,
							z * vec.x - x * vec.z,
							x * vec.y - y * vec.x);
				}
		};

	typedef Vector2<float> Vector2f;
	typedef Vector3<float> Vector3f;

} // namespace math
} // namespace narf

#endif // NARFBLOCK_VECTOR_H
