#ifndef NARFBLOCK_VECTOR_H
#define NARFBLOCK_VECTOR_H

#include <math.h>

namespace narf {

class Vector2f {
public:

	Vector2f(float x, float y) : x_(x), y_(y) { }

	float x() const { return x_; }
	float y() const { return y_; }

	float length() const
	{
		return sqrt(x_ * x_ + y_ * y_);
	}

	const Vector2f normalize() const
	{
		float length = this->length();
		if (length == 0) {
			return *this;
		} else {
			return Vector2f(x_ / length, y_ / length);
		}
	}

	const Vector2f operator+(const Vector2f &add) const
	{
		return Vector2f(x_ + add.x_, y_ + add.y_);
	}

	Vector2f &operator +=(const Vector2f &add)
	{
		x_ += add.x_;
		y_ += add.y_;
		return *this;
	}

	const Vector2f operator-(const Vector2f &sub) const
	{
		return Vector2f(x_ - sub.x_, y_ - sub.y_);
	}

	Vector2f &operator-=(const Vector2f &sub)
	{
		x_ -= sub.x_;
		y_ -= sub.y_;
		return *this;
	}

private:

	float x_, y_;
};

class Vector3f {
public:

	Vector3f(float x, float y, float z) : x_(x), y_(y), z_(z) { }

	float length()
	{
		return sqrtf(x_ * x_ + y_ * y_ + z_ * z_);
	}

private:

	float x_, y_, z_;
};

} // namespace narf

#endif // NARFBLOCK_VECTOR_H
