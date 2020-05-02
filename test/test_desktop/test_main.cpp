#include <unity.h>
#include <utils.h>

void testStringToNumber() {
  const char *normalTest = "12345678";
  int8_t errorCode;
  uint32_t number = stringToNumber(normalTest, errorCode);

  TEST_ASSERT_EQUAL_UINT8(0, errorCode);
  TEST_ASSERT_EQUAL_UINT32(12345678, number);

  const char *negativeTest = "-123";
  number = stringToNumber(negativeTest, errorCode);
  TEST_ASSERT_NOT_EQUAL(0, errorCode);

  const char *zeroTest = "0";
  number = stringToNumber(zeroTest, errorCode);
  TEST_ASSERT_EQUAL_UINT8(0, errorCode);
  TEST_ASSERT_EQUAL_UINT32(0, number);

  const char *notNumberTest1 = "x123";
  number = stringToNumber(notNumberTest1, errorCode);
  TEST_ASSERT_NOT_EQUAL(0, errorCode);

  const char *notNumberTest2 = "123 ";
  number = stringToNumber(notNumberTest2, errorCode);
  TEST_ASSERT_NOT_EQUAL(0, errorCode);
}

void testEnsureEndsInSlash() {
  String emptyTest = "";
  ensureEndsInSlash(emptyTest);
  TEST_ASSERT_EQUAL_STRING("/", emptyTest.c_str());

  String slashTest = "/";
  ensureEndsInSlash(slashTest);
  TEST_ASSERT_EQUAL_STRING("/", slashTest.c_str());

  String relativeTest1 = "foo/bar";
  ensureEndsInSlash(relativeTest1);
  TEST_ASSERT_EQUAL_STRING("foo/bar/", relativeTest1.c_str());

  String relativeTest2 = "foo/bar/";
  ensureEndsInSlash(relativeTest2);
  TEST_ASSERT_EQUAL_STRING("foo/bar/", relativeTest2.c_str());

  String absoluteTest1 = "/foo/bar";
  ensureEndsInSlash(absoluteTest1);
  TEST_ASSERT_EQUAL_STRING("/foo/bar/", absoluteTest1.c_str());

  String absoluteTest2 = "/foo/bar/";
  ensureEndsInSlash(absoluteTest2);
  TEST_ASSERT_EQUAL_STRING("/foo/bar/", absoluteTest2.c_str());
}

void testPathJoin() {
  String emptyTest = "";
  String joined = pathJoin(emptyTest, "foo/test.txt");
  TEST_ASSERT_EQUAL_STRING("foo/test.txt", joined.c_str());

  String slashTest = "/";
  joined = pathJoin(slashTest, "foo/test.txt");
  TEST_ASSERT_EQUAL_STRING("/foo/test.txt", joined.c_str());

  String relativeTest1 = "foo/bar";
  joined = pathJoin(relativeTest1, "foo/test.txt");
  TEST_ASSERT_EQUAL_STRING("foo/bar/foo/test.txt", joined.c_str());

  String relativeTest2 = "foo/bar/";
  joined = pathJoin(relativeTest2, "foo/test.txt");
  TEST_ASSERT_EQUAL_STRING("foo/bar/foo/test.txt", joined.c_str());

  String absoluteTest1 = "/foo/bar";
  joined = pathJoin(absoluteTest1, "foo/test.txt");
  TEST_ASSERT_EQUAL_STRING("/foo/bar/foo/test.txt", joined.c_str());

  String absoluteTest2 = "/foo/bar/";
  joined = pathJoin(absoluteTest2, "foo/test.txt");
  TEST_ASSERT_EQUAL_STRING("/foo/bar/foo/test.txt", joined.c_str());
}

void testIntPow() {
  uint16_t result;

  result = int_pow(2, 1);
  TEST_ASSERT_EQUAL_UINT16(2, result);

  result = int_pow(2, 2);
  TEST_ASSERT_EQUAL_UINT16(4, result);

  result = int_pow(2, 12);
  TEST_ASSERT_EQUAL_UINT16(4096, result);

  result = int_pow(2, 0);
  TEST_ASSERT_EQUAL_UINT16(1, result);

  result = int_pow(0, 1);
  TEST_ASSERT_EQUAL_UINT16(0, result);

  result = int_pow(0, 5);
  TEST_ASSERT_EQUAL_UINT16(0, result);

  result = int_pow(500, 2);
  TEST_ASSERT_EQUAL_UINT16(250000, result);
}

void testPathGetDir() {
  TEST_ASSERT_EQUAL_STRING("", pathGetDir("").c_str());
  TEST_ASSERT_EQUAL_STRING("", pathGetDir("foo").c_str());
  TEST_ASSERT_EQUAL_STRING("", pathGetDir("foo/").c_str());
  TEST_ASSERT_EQUAL_STRING("foo", pathGetDir("foo/bar").c_str());
  TEST_ASSERT_EQUAL_STRING("foo", pathGetDir("foo/bar/").c_str());
  TEST_ASSERT_EQUAL_STRING("foo", pathGetDir("foo//bar").c_str());
  TEST_ASSERT_EQUAL_STRING("foo", pathGetDir("foo//bar//").c_str());
  TEST_ASSERT_EQUAL_STRING("foo/bar", pathGetDir("foo/bar/baz").c_str());
  TEST_ASSERT_EQUAL_STRING("foo/bar", pathGetDir("foo/bar/baz/").c_str());
  TEST_ASSERT_EQUAL_STRING("foo//bar", pathGetDir("foo//bar/baz").c_str());
  TEST_ASSERT_EQUAL_STRING("/", pathGetDir("/").c_str());
  TEST_ASSERT_EQUAL_STRING("/", pathGetDir("//").c_str());
  TEST_ASSERT_EQUAL_STRING("/", pathGetDir("/foo").c_str());
  TEST_ASSERT_EQUAL_STRING("/", pathGetDir("/foo/").c_str());
  TEST_ASSERT_EQUAL_STRING("/foo", pathGetDir("/foo/bar").c_str());
  TEST_ASSERT_EQUAL_STRING("/foo", pathGetDir("/foo/bar/").c_str());
  TEST_ASSERT_EQUAL_STRING("/foo/bar", pathGetDir("/foo/bar/baz").c_str());
  TEST_ASSERT_EQUAL_STRING("/foo/bar", pathGetDir("/foo/bar/baz/").c_str());
  TEST_ASSERT_EQUAL_STRING("//foo//bar", pathGetDir("//foo//bar//baz").c_str());
}

int main(int arc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(testStringToNumber);
  RUN_TEST(testEnsureEndsInSlash);
  RUN_TEST(testPathJoin);
  RUN_TEST(testIntPow);
  RUN_TEST(testPathGetDir);
  UNITY_END();

  return 0;
}
