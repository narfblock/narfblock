#include <stdio.h>

#include <gtest/gtest.h>
#include "narf/math/vector.h"
#include "narf/math/quaternion.h"
#include "narf/math/floats.h"

TEST(Quaternion, General) {
	narf::Quaternion<float> q1(1, 0, 0, 2);
	ASSERT_FLOAT_EQ(q1.w, 1.0f);
	ASSERT_FLOAT_EQ(q1.v.x, 0.0f);
	ASSERT_FLOAT_EQ(q1.v.y, 0.0f);
	ASSERT_FLOAT_EQ(q1.v.z, 2.0f);

	narf::Quaternion<float> q2(3, -1, 4, 3);

	narf::Quaternion<float> q3 = (q1 * q2).inverse();
	ASSERT_FLOAT_EQ(q3.w, -0.017142856f);
	ASSERT_FLOAT_EQ(q3.v.x, 0.051428571f);
	ASSERT_FLOAT_EQ(q3.v.y, -0.011428571f);
	ASSERT_FLOAT_EQ(q3.v.z, -0.051428571f);
	//printf("%f %f %f %f\n", q3.w, q3.v.v.x, q3.v.v.y, q3.v.v.z);
}

TEST(Quaternion, Conjugate) {
	narf::Quaternion<float> q1(1, 1.5f, -2.2f, 2);
	auto q1c = q1.conjugate();
	ASSERT_FLOAT_EQ(q1c.w, 1.0f);
	ASSERT_FLOAT_EQ(q1c.v.x, -1.5f);
	ASSERT_FLOAT_EQ(q1c.v.y, 2.2f);
	ASSERT_FLOAT_EQ(q1c.v.z, -2.0f);
}

TEST(Quaternion, Norm) {
	narf::Quaternion<float> q1(1, 1.5f, 2.2f, 3);
	auto q1n = q1.norm();
	ASSERT_FLOAT_EQ(q1n, 4.1340055f);
}

TEST(Quaternion, Inverse) {
	narf::Quaternion<float> q1(3, -1, 4, 3);
	auto q1i = q1.inverse();
	ASSERT_FLOAT_EQ(q1i.w, 0.0857143f);
	ASSERT_FLOAT_EQ(q1i.v.x, 0.028571429f);
	ASSERT_FLOAT_EQ(q1i.v.y, -0.11428571f);
	ASSERT_FLOAT_EQ(q1i.v.z, -0.0857143f);
}

TEST(Quaternion, Normalize) {
	narf::Quaternion<float> q1(3, -1, 4, 3);
	ASSERT_FLOAT_EQ(q1.norm(), 5.91608f);
	auto q2 = q1.normalize();
	ASSERT_FLOAT_EQ(q2.norm(), 1.0f);
	ASSERT_FLOAT_EQ(q2.w, 0.50709254f);
	ASSERT_FLOAT_EQ(q2.v.x, -0.16903085f);
	ASSERT_FLOAT_EQ(q2.v.y, 0.67612338f);
	ASSERT_FLOAT_EQ(q2.v.z, 0.50709254f);
}

TEST(Quaternion, Orientation) {
	narf::Quaternion<float> q1(3, -1, 4, 3);
	q1.normalizeSelf();
	narf::Orientation<float> o = q1;
	narf::Quaternion<float> q2 = o;
	ASSERT_NEAR(q1.w, q2.w, 0.00001f);
	ASSERT_NEAR(q1.v.x, q2.v.x, 0.00001f);
	ASSERT_NEAR(q1.v.y, q2.v.y, 0.00001f);
	ASSERT_NEAR(q1.v.z, q2.v.z, 0.00001f);
}

TEST(Quaternion, Orientation2) {
	narf::Quaternion<float> q(narf::Angle<float>(0), 1, 0, 0); // No rotation, so should be facing straight ahead
	narf::Orientation<float> o = q;
	ASSERT_FLOAT_EQ(o.roll, 0);
	ASSERT_FLOAT_EQ(o.pitch, 0);
	ASSERT_FLOAT_EQ(o.yaw, 0);
	// Axis-angle rotations go counter-clockwise, so rotating negative around x pitches it up
	q = narf::Quaternion<float>(narf::Angle<float>(narf::fromDeg(-45.0f)), 1, 0, 0); // -45° rotation around x-axis (pitch)
	o = q;
	ASSERT_FLOAT_EQ(o.roll, 0);
	ASSERT_FLOAT_EQ(o.pitch, (float)M_PI / 4);
	ASSERT_FLOAT_EQ(o.yaw, 0);
	q = narf::Quaternion<float>(narf::Angle<float>(narf::fromDeg(-90.0f)), 1, 0, 0); // -90° rotation around x-axis (pitch)
	o = q;
	ASSERT_FLOAT_EQ(o.roll, 0);
	ASSERT_FLOAT_EQ(o.pitch, (float)M_PI / 2);
	ASSERT_FLOAT_EQ(o.yaw, 0);
	q = narf::Quaternion<float>(narf::Angle<float>(narf::fromDeg(-45.0f)), 0, 0, 1); // -45° rotation around the z-axis (yaw)
	o = q;
	ASSERT_FLOAT_EQ(o.roll, 0);
	ASSERT_FLOAT_EQ(o.pitch, 0);
	ASSERT_FLOAT_EQ(o.yaw, -(float)M_PI / 4);
}

TEST(Quaternion, Matrix) {
	narf::Quaternion<float> q1(3, -1, 4, 3);
	q1.normalizeSelf();
	//printf("%s\n", q1.to_string().c_str());
	narf::Matrix4x4<float> o = q1;
	//printf("%s\n", o.to_string().c_str());
	narf::Quaternion<float> q2 = o;
	q2.normalizeSelf();
	//printf("%s\n", q2.to_string().c_str());
	ASSERT_NEAR(q1.w, q2.w, 0.00001f);
	ASSERT_NEAR(q1.v.x, q2.v.x, 0.00001f);
	ASSERT_NEAR(q1.v.y, q2.v.y, 0.00001f);
	ASSERT_NEAR(q1.v.z, q2.v.z, 0.00001f);

}
