#include <stdio.h>

#include "narf/version.h"

#include "narf/math/math.h"

void testPlanePoint(const narf::math::Plane<float> &plane, const narf::math::coord::Point3f &point) {
	auto distance = plane.distanceTo(point);
	auto closest = plane.nearestPoint(point);
	printf("(%f,%f,%f) distance: %+f closest point: (%f,%f,%f) on plane: %s\n", point.x, point.y, point.z, distance, closest.x, closest.y, closest.z, plane.containsPoint(point) ? "y" : "n");
}

void testRayAtDistance(const narf::math::Ray<float> &ray, float distance) {
	auto p = ray.pointAtDistance(distance);
	printf("At distance %f: (%f, %f, %f)\n", distance, p.x, p.y, p.z);
}

int main(int argc, char **argv)
{
	printf("NarfBlock unit tests\n");
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
	
	auto angle = narf::math::Anglef(narf::math::fromDeg(1234.0f));
	auto angle2 = narf::math::Anglef(2.687807048071267548443f);
	printf("Angle1: %f\n", angle.toDeg());
	printf("Equality: %d\n", angle == angle2 ? 1 : 0);

	 //Should really have some sort of real testing framework
	auto testpoint = narf::math::coord::Point3f(1, 1, 1);
	auto testsphere = static_cast<narf::math::coord::Sphericalf>(testpoint);
	printf("Test Point  : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere : %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);
	testsphere.radius = 1;
	testsphere.azimuth = 3.1415926f;
	testsphere.inclination = 3.1415926f/2;
	testpoint = testsphere;
	printf("Test Point2 : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere2: %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);
	testpoint.x = 21; testpoint.y = 5.3f; testpoint.z = -11.8f;
	testsphere.radius =      24.66434673775082979f;
	testsphere.inclination = 2.069654722802558995f;
	testsphere.azimuth =     0.247218301039533316f;
	printf("Test Point3 : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere3: %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);
	testsphere.radius =      24.0f;
	testsphere.inclination = 2.069654722802558995f;
	testsphere.azimuth =     0.247218301039533316f;
	printf("Test Point3 : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere4: %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);

	auto testplane = narf::math::Plane<float>::fromPointNormal(narf::math::coord::Point3f(1, 0, 0), narf::math::Vector3f(1, 0, 0));
	printf("Test plane: (%f, %f, %f), %f\n", testplane.normal().x, testplane.normal().y, testplane.normal().z, testplane.distanceToOrigin());
	testPlanePoint(testplane, narf::math::coord::Point3f(0, 0, 0));
	testPlanePoint(testplane, narf::math::coord::Point3f(1, 0, 0));
	testPlanePoint(testplane, narf::math::coord::Point3f(2, 0, 0));
	testPlanePoint(testplane, narf::math::coord::Point3f(0, 1, 0));

	auto testray = narf::math::Ray<float>(narf::math::coord::Point3f(0, 0, 0), narf::math::Vector3f(1, 1, 1));
	printf("Test ray: point (%f, %f, %f) direction (%f, %f, %f)\n", testray.initialPoint().x, testray.initialPoint().y, testray.initialPoint().z, testray.direction().x, testray.direction().y, testray.direction().z);
	testRayAtDistance(testray, 0);
	testRayAtDistance(testray, 0.5);
	testRayAtDistance(testray, 1);
	testRayAtDistance(testray, 5);
	return 0;
}