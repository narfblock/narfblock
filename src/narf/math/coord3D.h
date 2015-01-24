#ifndef NARFBLOCK_MATH_COORD_3D_H
#define NARFBLOCK_MATH_COORD_3D_H

#include <iterator>

#include <math.h>
#include <assert.h>
#include "narf/math/floats.h"
#include "narf/math/orientation.h"

namespace narf {
	template<class T>
	class Vector3;

	template<class T>
	class Point3;

	template<class T>
	T distance(T x, T y, T z, T x2, T y2, T z2);

	template<class T>
	T distance(Point3<T> A, Point3<T> B);

	template<class T>
	class Point3 {
	public:
		T x;
		T y;
		T z;
		Point3() {}
		Point3(T x, T y, T z) : x(x), y(y), z(z) {};
		bool operator==(const Point3<T>& rhs) const {
			return almostEqual(x, rhs.x) && almostEqual(y, rhs.y) && almostEqual(z, rhs.z);
		}
		bool operator!=(const Point3<T>& rhs) const {
			return !(*this == rhs);
		}
		float distanceTo(Point3<T> other) const {
			return distance(x, y, z, other.x, other.y, other.z);
		}
		float distanceTo(T x2, T y2, T z2) const {
			return distance(x, y, z, x2, y2, z2);
		}
		const Point3<T> operator+(const Point3<T> &add) const {
			return Point3<T>(x + add.x, y + add.y, z + add.z);
		}
		Point3<T> &operator -=(const Point3<T> &sub) {
			x -= sub.x;
			y -= sub.y;
			z -= sub.z;
			return *this;
		}
		const Point3<T> operator-(const Point3<T> &sub) const {
			return Point3<T>(x - sub.x, y - sub.y, z - sub.z);
		}
		const T operator[](const int idx) const {
			assert(idx >=0 && idx < 3);
			return (idx == 0) ? x : ((idx == 1) ? y : z);
		}
		operator Orientation<T> () const {
			auto a = Vector3<T>(x, y, z);
			auto b = Vector3<T>(x, y, 0);
			auto c = Vector3<T>(0, 1, 0);
			return Orientation<T>(b.angleTo(a), (x < 0 ? -1 : 1) * c.angleTo(b));
		}
	};

	typedef Point3<float> Point3f;
	typedef Point3<double> Point3d;
	typedef Point3<uint32_t> Point3i;

	template<class T>
	T distanceSquared(T x, T y, T z, T x2, T y2, T z2) {
		T a = (x - x2) * (x - x2);
		T b = (y - y2) * (y - y2);
		T c = (z - z2) * (z - z2);
		return a + b + c;
	}

	template<class T>
	T distance(T x, T y, T z, T x2, T y2, T z2) {
		return static_cast<T>(sqrt(distanceSquared(x, y, z, x2, y2, z2)));
	}

	template<class T>
	T distance(Point3<T> A, Point3<T> B) {
		return A.distanceTo(B);
	}

#define Coord3Type typename
	template<Coord3Type T>
	class Coord3IterProducer {
	public:
		virtual T next(const T& pos) const = 0;
	};

	template<typename T>
	class Coord3Iter {
	private:
		const Coord3IterProducer<T>* producer_;
		T pos_;

	public:
		Coord3Iter(const Coord3IterProducer<T>* producer, T pos) :
			producer_(producer), pos_(pos) {}

		bool operator!=(const Coord3Iter& other) const {
			// TODO: using Point3 operator!= somehow pulls in Angle, which doesn't work for ints
			// so define equality by hand here
			return pos_.x != other.pos_.x ||
				pos_.y != other.pos_.y ||
				pos_.z != other.pos_.z;
		}

		T operator*() const {
			return pos_;
		}

		const Coord3Iter& operator++() {
			pos_ = producer_->next(pos_);
			return *this;
		}
	};


	template<Coord3Type T>
	class ZYXCoordIter : Coord3IterProducer<T> {
	private:
		T start_;
		T end_;

	public:
		ZYXCoordIter(const T& start, const T& end) {
			assert(start.x != end.x);
			assert(start.y != end.y);
			assert(start.z != end.z);

			start_.x = std::min(start.x, end.x);
			start_.y = std::min(start.y, end.y);
			start_.z = std::min(start.z, end.z);

			end_.x = std::max(start.x, end.x);
			end_.y = std::max(start.y, end.y);
			end_.z = std::max(start.z, end.z);
		}

		Coord3Iter<T> begin() const {
			return Coord3Iter<T>(this, start_);
		}

		Coord3Iter<T> end() const {
			return Coord3Iter<T>(this, end_);
		}

		T next(const T& pos) const {
			auto n(pos);
			n.x++;
			if (n.x == end_.x) {
				n.x = start_.x;
				n.y++;
			}

			if (n.y == end_.y) {
				n.y = start_.y;
				n.z++;
			}

			if (n.z == end_.z) {
				// iteration complete
				n = end_;
			}

			return n;
		}
	};
}

#endif
