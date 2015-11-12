#include <gtest/gtest.h>
#include "narf/path.h"

#if defined(__unix__) || defined(__APPLE__)

TEST(PATH, dirName) {
	ASSERT_EQ("/", narf::util::dirName("/"));
	ASSERT_EQ("/", narf::util::dirName("/foo"));
	ASSERT_EQ("/", narf::util::dirName("/foo/"));
	ASSERT_EQ("/foo", narf::util::dirName("/foo/bar"));
}

TEST(PATH, baseName) {
	ASSERT_EQ("/", narf::util::baseName("/"));
	ASSERT_EQ("foo", narf::util::baseName("foo"));
	ASSERT_EQ("foo", narf::util::baseName("/foo/"));
	ASSERT_EQ("bar", narf::util::baseName("/foo/bar"));
}

#endif
