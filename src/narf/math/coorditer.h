#ifndef NARFBLOCK_MATH_COORDITER_H
#define NARFBLOCK_MATH_COORDITER_H

#include <iterator>

#include <math.h>
#include <assert.h>

namespace narf {

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
