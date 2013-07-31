#include <stdio.h>

#include "narf/version.h"

#include "narf/math/math.h"

int main(int argc, char **argv)
{
	printf("NarfBlock unit tests\n");
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
	
	auto angle = narf::math::Anglef(narf::math::fromDeg(1234.0));
	auto angle2 = narf::math::Anglef(2.687807048071267548443);
	printf("Angle1: %f\n", angle.toDeg());
	printf("Equality: %d\n", angle == angle2 ? 1 : 0);

	 //Should really have some sort of real testing framework
	auto testpoint = narf::math::coord::Point3f(1, 1, 1);
	auto testsphere = static_cast<narf::math::coord::Sphericalf>(testpoint);
	printf("Test Point  : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere : %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);
	testsphere.radius = 1;
	testsphere.azimuth = 3.1415926;
	testsphere.inclination = 3.1415926/2;
	testpoint = testsphere;
	printf("Test Point2 : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere2: %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);
	testpoint.x = 21; testpoint.y = 5.3; testpoint.z = -11.8;
	testsphere.radius =      24.66434673775082979;
	testsphere.inclination = 2.069654722802558995;
	testsphere.azimuth =     0.247218301039533316;
	printf("Test Point3 : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere3: %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);
	testsphere.radius =      24.0;
	testsphere.inclination = 2.069654722802558995;
	testsphere.azimuth =     0.247218301039533316;
	printf("Test Point3 : %f, %f, %f\n", testpoint.x, testpoint.y, testpoint.z);
	printf("Test Sphere4: %f, %f, %f\n", testsphere.radius, testsphere.inclination, testsphere.azimuth);
	printf("Equality    : %d\n", testsphere == testpoint ? 1 : 0);

	return 0;
}
