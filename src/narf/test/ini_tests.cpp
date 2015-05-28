#include <stdio.h>

#include <gtest/gtest.h>
#include "narf/ini.h"


TEST(INI, BasicRead) {
	std::string inData = " foo  = \"1\"     \n  [bar] ; baz \n\tqux = quux\n\nquz = true\n[mar]\n\n\nqux = 0.5\nquz = 0.75\n";
	narf::INI::File ini;
	ini.load(inData);
	ASSERT_EQ(ini.getInt32("foo"), 1);
	ASSERT_EQ(ini.getString("bar.qux"), "quux");
	ASSERT_EQ(ini.getBool("bar.quz"), true);
	ASSERT_EQ(ini.getFloat("mar.qux"), 0.5);
	ASSERT_EQ(ini.getDouble("mar.quz"), 0.75);
}

TEST(INI, BasicSave) {
	std::string inData = " foo  = \"1\"     \n  [bar] ; baz \n\tqux = quux\n\nquz = true\n[mar]\n\n\nqux = 0.5\nquz = 0.75\n";
	std::string expectedOutData = " foo  = 2     \n  [bar] ; baz \n\tqux = quux\n\nquz = true\n[mar]\n\n\nqux = 0.5\nquz = 0.75\n";
	narf::INI::File ini;
	ini.load(inData);
	ini.setInt32("foo", 2);
	std::string outData = ini.save();
	ASSERT_EQ(outData, expectedOutData);
}

TEST(INI, Int32) {
	narf::INI::File ini;
	ini.load("foo = 501");
	ASSERT_EQ(ini.getInt32("foo"), 501);
	ini.setString("foo", "106232");
	ASSERT_EQ(ini.getInt32("foo"), 106232);
	ini.setInt32("foo", 123456);
	ASSERT_EQ(ini.getInt32("foo"), 123456);
}

TEST(INI, String) {
	narf::INI::File ini;
	ini.load("foo = beep");
	ASSERT_EQ(ini.getString("foo"), "beep");
	ini.setInt32("foo", 123);
	ASSERT_EQ(ini.getString("foo"), "123");
	ini.setFloat("foo", 0.5);
	ASSERT_EQ(ini.getString("foo"), "0.5");
	ini.setDouble("foo", 0.75);
	ASSERT_EQ(ini.getString("foo"), "0.75");
	ini.setBool("foo", true);
	ASSERT_EQ(ini.getString("foo"), "true");
}

TEST(INI, Comments) {
	auto ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("; comment\nfoo = beep"));
	ASSERT_EQ(ini->getString("foo"), "beep");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[bar] ; bez\nfoo = beep"));
	ASSERT_EQ(ini->getString("bar.foo"), "beep");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[baz]\n; bez\nfoo = beep"));
	ASSERT_EQ(ini->getString("baz.foo"), "beep");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[qux]\nfoo = \"beep \"  ; Bez"));
	ASSERT_EQ(ini->getString("qux.foo"), "beep ");
	delete ini; ini = new narf::INI::File();
	ASSERT_FALSE(ini->load("[qux]\nfoo = \"beep \"  bex ; Bez"));
	ASSERT_EQ(ini->getString("qux.foo"), "beep ");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[qux]\nfoo = beep ; Bez"));
	ASSERT_EQ(ini->getString("qux.foo"), "beep");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[qux]\nfoo = beep honk    ; Bez"));
	ASSERT_EQ(ini->getString("qux.foo"), "beep honk");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[qux]\nfoo = \"beep;;honk\""));
	ASSERT_EQ(ini->getString("qux.foo"), "beep;;honk");
	delete ini; ini = new narf::INI::File();
	ASSERT_TRUE(ini->load("[qux]\nfoo = beep\\;\\;honk"));
	ASSERT_EQ(ini->getString("qux.foo"), "beep;;honk");

}
