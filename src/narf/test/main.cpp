#include <stdio.h>

#include "narf/version.h"

#include "narf/math/math.h"
#include "narf/net/net.h"

#include "narf/stdioconsole.h"

void testPlanePoint(const narf::Plane<float> &plane, const narf::Point3f &point) {
	auto distance = plane.distanceTo(point);
	auto closest = plane.nearestPoint(point);
	printf("(%f,%f,%f) distance: %+f closest point: (%f,%f,%f) on plane: %s\n", point.x, point.y, point.z, distance, closest.x, closest.y, closest.z, plane.containsPoint(point) ? "y" : "n");
}

void testRayAtDistance(const narf::Ray<float> &ray, float distance) {
	auto p = ray.pointAtDistance(distance);
	printf("At distance %f: (%f, %f, %f)\n", distance, p.x, p.y, p.z);
}

int main(int argc, char **argv)
{
	printf("NarfBlock unit tests\n");
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

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
	printf("Test ray: point (%f, %f, %f) direction (%f, %f)\n", testray.initialPoint().x, testray.initialPoint().y, testray.initialPoint().z, testray.direction().yaw.toDeg(), testray.direction().pitch.toDeg());
	testRayAtDistance(testray, 0);
	testRayAtDistance(testray, 0.5);
	testRayAtDistance(testray, 1);
	testRayAtDistance(testray, 5);
	narf::Orientation<float> orient = narf::Vector3f(1, 0, 1);
	printf("Vector3 (1, 0, 1)-> Orientation (%f, %f)\n", orient.yaw.toDeg(), orient.pitch.toDeg());

	const struct {
		const char* input;
		const char* host;
		uint16_t port;
	} testAddrs[] = {
		{"0.0.0.0",                     "0.0.0.0",      0   },
		{"0.0.0.0:80",                  "0.0.0.0",      80  },
		{"0.0.0.0::80",                 nullptr             },
		{"::",                          nullptr             }, // plain IPv6 without port - invalid for now, but should be unambiguous with IPv4 + port
		{"::1",                         nullptr             },
		{"1:2::3",                      nullptr             },
		{"[::]",                        "::",           0   },
		{"[1:2::3]",                    "1:2::3",       0   },
		{"[1:2::3]:80",                 "1:2::3",       80  },
		{"[1:2::3]::80",                nullptr             },
		{"1:[2:3]::4",                  nullptr             },
		{"1.2.3.4:[80]",                nullptr             },
		{"host",                        "host",         0   },
		{"host:80",                     "host",         80  },
		{"host::80",                    nullptr             },
		{"example.com",                 "example.com",  0   },
		{"example.com:80",              "example.com",  80  },
		{"example.com::80",             nullptr             },
	};

	for (size_t i = 0; i < sizeof(testAddrs) / sizeof(*testAddrs); i++) {
		std::string addr(testAddrs[i].input), host;
		uint16_t port;
		printf("splitHostPort: %-15s -> ", addr.c_str());
		bool valid = narf::net::splitHostPort(addr, host, port);

		if ((!valid && testAddrs[i].host == nullptr) ||
			(testAddrs[i].host != nullptr && host == std::string(testAddrs[i].host) && port == testAddrs[i].port)) {
			printf("PASS - ");
		} else {
			printf("FAIL - ");
		}

		if (valid) {
			printf("host \"%s\" port %u\n", host.c_str(), port);
		} else {
			printf("invalid\n");
		}

	}

	printf("ZYXCoordIter test 0,0,0->2,2,2\n");
	narf::ZYXCoordIter<narf::Point3<uint32_t>> iter({0, 0, 0}, {2, 2, 2});
	for (const auto& c : iter) {
		printf("%u,%u,%u\n", c.x, c.y, c.z);
	}

#define TEST_ILOG2(n) printf("ilog2(%u) = %u (%s)\n", n, narf::ilog2(n), narf::ilog2(n) == (int32_t)(log((double)n) / log(2.0)) ? "OK" : "FAIL")

	TEST_ILOG2(0);
	TEST_ILOG2(1);
	TEST_ILOG2(2);
	TEST_ILOG2(3);
	TEST_ILOG2(4);
	TEST_ILOG2(5);
	TEST_ILOG2(6);
	TEST_ILOG2(15);
	TEST_ILOG2(16);
	return 0;
}
