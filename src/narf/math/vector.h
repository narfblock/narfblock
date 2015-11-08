#ifndef NARFBLOCK_VECTOR_H
#define NARFBLOCK_VECTOR_H

#include "narf/bytestream.h"

#include "narf/math/floats.h"
#include "narf/math/ints.h"
#include "narf/math/orientation.h"

namespace narf {

	template<class T>
	class Orientation;

	template<class T>
	class Vector2 {
	public:
		T x, y;

		Vector2() : x(0.0f), y(0.0f) { }
		Vector2(T x, T y) : x(x), y(y) { }

		T length() const {
			return sqrt(x * x + y * y);
		}

		const Vector2<T> normalize() const {
			T length = this->length();
			if (length == 0) {
				return *this;
			} else {
				return Vector2<T>(x / length, y / length);
			}
		}

		const Vector2<T> operator+(const Vector2<T> &add) const {
			return Vector2<T>(x + add.x, y + add.y);
		}

		Vector2<T> &operator +=(const Vector2<T> &add) {
			x += add.x;
			y += add.y;
			return *this;
		}

		const Vector2<T> operator-(const Vector2<T> &sub) const {
			return Vector2<T>(x - sub.x, y - sub.y);
		}

		Vector2<T> &operator-=(const Vector2<T> &sub) {
			x -= sub.x;
			y -= sub.y;
			return *this;
		}

		const Vector2<T> operator*(T v) const {
			return Vector2<T>(x * v, y * v);
		}

		const Vector2<T> operator/(T v) const {
			return Vector2<T>(x / v, y / v);
		}

		T dot(Vector2<T> vec) const {
			return x * vec.x + y * vec.y;
		}

		T angleTo(Vector2<T> vec) const {
			return std::acos(dot(vec) / (length() * vec.length()));
		}
	};

	template<class T>
	class Vector3 {
	public:
		T x, y, z;

		Vector3() : x(0), y(0), z(0) { }
		Vector3(T x, T y, T z) : x(x), y(y), z(z) { }

		Vector3(ByteStream& s) {
			s.read(&x, ByteStream::Endian::LITTLE);
			s.read(&y, ByteStream::Endian::LITTLE);
			s.read(&z, ByteStream::Endian::LITTLE);
		}

		void serialize(ByteStream& s) const {
			s.write(x, ByteStream::Endian::LITTLE);
			s.write(y, ByteStream::Endian::LITTLE);
			s.write(z, ByteStream::Endian::LITTLE);
		};

		bool operator==(const Vector3<T>& rhs) const {
			return almostEqual(x, rhs.x) && almostEqual(y, rhs.y) && almostEqual(z, rhs.z);
		}

		bool operator!=(const Vector3<T>& rhs) const {
			return !(*this == rhs);
		}

		bool operator<=(const Vector3<T>& rhs) const {
			return x <= rhs.x && y <= rhs.y && z <= rhs.z;
		}

		T lengthSquared() const {
			return x * x + y * y + z * z;
		}

		T length() const {
			return std::sqrt(lengthSquared());
		}

		operator Orientation<T>() const {
			auto a = Vector3<T>(x, y, z);
			auto b = Vector3<T>(x, y, 0);
			auto c = Vector3<T>(0, 1, 0);
			return Orientation<T>(b.angleTo(a), (x < 0 ? -1 : 1) * c.angleTo(b));
		}

		const Vector3<T> normalize() const {
			T length = this->length();
			if (length == 0) {
				return *this;
			} else {
				return Vector3<T>(x / length, y / length, z / length);
			}
		}

		void normalizeSelf() {
			T length = this->length();
			if (length != 0) {
				x /= length;
				y /= length;
				z /= length;
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

		const Vector3<T> operator-() const {
			return Vector3<T>(-x, -y, -z);
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

		const Vector3<T> operator/(T v) const {
			return Vector3<T>(x / v, y / v, z / v);
		}

		Vector3<T> &operator/=(T v) {
			x /= v;
			y /= v;
			z /= v;
			return *this;
		}

		Vector3<T> operator*(Vector3<T> vec) const {
			return Vector3<T>(x * vec.x, y * vec.y, z * vec.z);
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

		T angleTo(Vector3<T> vec) const {
			return std::acos(dot(vec) / (length() * vec.length()));
		}

		// point to point distance squared
		T distanceSquaredTo(const Vector3<T>& p) const {
			T a = p.x - x;
			T b = p.y - y;
			T c = p.z - z;
			return a * a + b * b + c * c;
		}

		// point to point distance
		T distanceTo(const Vector3<T>& p) const {
			return std::sqrt(distanceSquaredTo(p));
		}

		T minComponent() const {
			return std::min(x, std::min(y, z));
		}

		T maxComponent() const {
			return std::max(x, std::max(y, z));
		}

		const Vector3<T> abs() const {
			return Vector3<T>(fabsf(x), fabsf(y), fabsf(z));
		}

	};

	template<class T>
	static inline Vector3<T> min(const Vector3<T>& v1, const Vector3<T>& v2) {
		return Vector3<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z));
	}

	template<class T>
	static inline Vector3<T> max(const Vector3<T>& v1, const Vector3<T>& v2) {
		return Vector3<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z));
	}

	typedef Vector2<float> Vector2f;
	typedef Vector3<float> Vector3f;

#define Point2 Vector2
#define Point3 Vector3
	typedef Point2<float> Point2f;
	typedef Point3<float> Point3f;

} // namespace narf


namespace std {
	template<typename T>
	struct hash<narf::Vector3<T> > {
		typedef narf::Vector3<T> argument_type;
		typedef size_t result_type;
		result_type operator()(const argument_type& p) const {
			const auto h1(hash<T>()(p.x));
			const auto h2(hash<T>()(p.y));
			const auto h3(hash<T>()(p.z));
			// TODO: maybe use a better method to mix these hashes
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
} // namespace std

#endif // NARFBLOCK_VECTOR_H
