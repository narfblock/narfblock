#include <stdio.h>

#include <gtest/gtest.h>

#include "narf/version.h"

#include "narf/utf.h"
#include "narf/math/math.h"
#include "narf/net.h"

#include "narf/stdioconsole.h"

using namespace narf;

void testPlanePoint(const narf::Plane<float> &plane, const narf::Point3f &point) {
	auto distance = plane.distanceTo(point);
	auto closest = plane.nearestPoint(point);
	printf("(%f,%f,%f) distance: %+f closest point: (%f,%f,%f) on plane: %s\n", point.x, point.y, point.z, distance, closest.x, closest.y, closest.z, plane.containsPoint(point) ? "y" : "n");
}

void testRayAtDistance(const narf::Ray<float> &ray, float distance) {
	auto p = ray.pointAtDistance(distance);
	printf("At distance %f: (%f, %f, %f)\n", distance, p.x, p.y, p.z);
}

static void oldTests();

int main(int argc, char **argv)
{
	printf("NarfBlock unit tests\n");
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
	oldTests();
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


// TODO: convert these to gtest
static void oldTests() {
	auto angle = narf::Anglef(narf::fromDeg(1234.0f));
	auto angle2 = narf::Anglef(2.687807048071267548443f);
	printf("Angle1: %f\n", angle.toDeg());
	printf("Equality: %d\n", angle == angle2 ? 1 : 0);

	 //Should really have some sort of real testing framework
	auto testplane = narf::Plane<float>(narf::Point3f(1, 0, 0), narf::Vector3f(1, 0, 0));
	printf("Test plane: (%f, %f, %f), %f\n", testplane.normal().x, testplane.normal().y, testplane.normal().z, testplane.distanceToOrigin());
	testPlanePoint(testplane, narf::Point3f(0, 0, 0));
	testPlanePoint(testplane, narf::Point3f(1, 0, 0));
	testPlanePoint(testplane, narf::Point3f(2, 0, 0));
	testPlanePoint(testplane, narf::Point3f(0, 1, 0));

	testplane = narf::Plane<float>(narf::Point3f(2, 0, 0), narf::Vector3f(1, 1, 0));
	auto interpoint = testplane.intersect(narf::Point3<float>(0, 0, 0), narf::Point3<float>(2, 2, 2));
	printf("Test plane 1: Intercept (%f, %f, %f)\n", interpoint.x, interpoint.y, interpoint.z);
	testplane = narf::Plane<float>(narf::Point3f(-2, 0, 0), narf::Vector3f(1, 1, 0));
	interpoint = testplane.intersect(narf::Point3<float>(0, 0, 0), narf::Vector3f(2, 2, 2).normalize());
	printf("Test plane 2: Intercept (%f, %f, %f)\n", interpoint.x, interpoint.y, interpoint.z);
	testplane = narf::Plane<float>(narf::Point3f(2, 0, 0), narf::Vector3f(1, 0, 0));
	interpoint = testplane.intersect(narf::Point3<float>(0, 0, 0), narf::Point3<float>(3, 0, 0));
	printf("Test plane 3: Intercept (%f, %f, %f)\n", interpoint.x, interpoint.y, interpoint.z);
	testplane = narf::Plane<float>(narf::Point3f(-1, 0, 0), narf::Vector3f(1, 0, 0));
	interpoint = testplane.intersect(narf::Point3<float>(-0.5f, 0.5f, 0.5f), narf::Vector3<float>(-0.5f, 0.1f, -0.1f));
	printf("Test plane 4: Intercept (%f, %f, %f)\n", interpoint.x, interpoint.y, interpoint.z);

	auto testray = narf::Ray<float>(narf::Point3f(0, 0, 0), narf::Vector3f(1, 1, 1));
	printf("Test ray: point (%f, %f, %f) direction (%f, %f, %f)\n", testray.initialPoint().x, testray.initialPoint().y, testray.initialPoint().z, testray.direction().x, testray.direction().y, testray.direction().z);
	testRayAtDistance(testray, 0);
	testRayAtDistance(testray, 0.5);
	testRayAtDistance(testray, 1);
	testRayAtDistance(testray, 5);
	narf::Orientation<float> orient = narf::Vector3f(1, 0, 1);
	printf("Vector3 (1, 0, 1)-> Orientation (%f, %f)\n", orient.yaw.toDeg(), orient.pitch.toDeg());

	printf("ZYXCoordIter test 0,0,0->2,2,2\n");
	narf::ZYXCoordIter<narf::Point3<uint32_t>> iter({0, 0, 0}, {2, 2, 2});
	for (const auto& c : iter) {
		printf("%u,%u,%u\n", c.x, c.y, c.z);
	}
}


TEST(ilog2Test, Positive) {
	EXPECT_EQ(0, narf::ilog2(1));
	EXPECT_EQ(1, narf::ilog2(2));
	EXPECT_EQ(1, narf::ilog2(3));
	EXPECT_EQ(2, narf::ilog2(4));
	EXPECT_EQ(2, narf::ilog2(5));
	EXPECT_EQ(2, narf::ilog2(6));
	EXPECT_EQ(2, narf::ilog2(7));
	EXPECT_EQ(3, narf::ilog2(8));
	EXPECT_EQ(3, narf::ilog2(15));
	EXPECT_EQ(4, narf::ilog2(16));
}
