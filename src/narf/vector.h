#ifndef NARFBLOCK_VECTOR_H
#define NARFBLOCK_VECTOR_H

#include <math.h>

namespace narf {

class Vector2f {
public:

	float x, y;

	Vector2f() : x(0.0f), y(0.0f) { }
	Vector2f(float x, float y) : x(x), y(y) { }

	float length() const
	{
		return sqrt(x * x + y * y);
	}

	const Vector2f normalize() const
	{
		float length = this->length();
		if (length == 0) {
			return *this;
		} else {
			return Vector2f(x / length, y / length);
		}
	}

	const Vector2f operator+(const Vector2f &add) const
	{
		return Vector2f(x + add.x, y + add.y);
	}

	Vector2f &operator +=(const Vector2f &add)
	{
		x += add.x;
		y += add.y;
		return *this;
	}

	const Vector2f operator-(const Vector2f &sub) const
	{
		return Vector2f(x - sub.x, y - sub.y);
	}

	Vector2f &operator-=(const Vector2f &sub)
	{
		x -= sub.x;
		y -= sub.y;
		return *this;
	}

	const Vector2f operator*(float v) const
	{
		return Vector2f(x * v, y * v);
	}

};

class Vector3f {
public:

	float x, y, z;

	Vector3f() : x(0.0f), y(0.0f), z(0.0f) { }
	Vector3f(float x, float y, float z) : x(x), y(y), z(z) { }

	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	const Vector3f normalize() const
	{
		float length = this->length();
		if (length == 0) {
			return *this;
		} else {
			return Vector3f(x / length, y / length, z / length);
		}
	}

	const Vector3f operator+(const Vector3f &add) const
	{
		return Vector3f(x + add.x, y + add.y, z + add.z);
	}

	Vector3f &operator +=(const Vector3f &add)
	{
		x += add.x;
		y += add.y;
		z += add.z;
		return *this;
	}

	const Vector3f operator-(const Vector3f &sub) const
	{
		return Vector3f(x - sub.x, y - sub.y, z - sub.z);
	}

	Vector3f &operator-=(const Vector3f &sub)
	{
		x -= sub.x;
		y -= sub.y;
		z -= sub.z;
		return *this;
	}

	const Vector3f operator*(float v) const
	{
		return Vector3f(x * v, y * v, z * v);
	}

};

} // namespace narf

#endif // NARFBLOCK_VECTOR_H
