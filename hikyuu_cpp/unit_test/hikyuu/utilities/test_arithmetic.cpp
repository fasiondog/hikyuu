#include "doctest/doctest.h"
#include <hikyuu/utilities/arithmetic.h>
#include <hikyuu/Log.h>

using namespace hku;

TEST_CASE("test_string_to_upper") {
    std::string x("abcd");
    to_upper(x);
    CHECK(x == "ABCD");

    std::string y("中abcdD");
    to_upper(y);
    CHECK(y == "中ABCDD");
}

TEST_CASE("test_string_to_lower") {
    std::string x("ABcD");
    to_lower(x);
    CHECK(x == "abcd");

    std::string y("中abCdD");
    to_lower(y);
    CHECK(y == "中abcdd");
}

// TEST_CASE("test_byteToHexStr") {
//     const char *x = "abcd";
//     std::string hex = byteToHexStr(x, 4);
//     CHECK_EQ(hex, "61626364");

//     std::string y(x);
//     hex = byteToHexStr(y);
//     CHECK_EQ(hex, "61626364");

//     CHECK_EQ("", byteToHexStr(""));
// }

// TEST_CASE("test_byteToHexStrForPrint") {
//     const char *x = "abcd";
//     std::string hex = byteToHexStrForPrint(x, 4);
//     CHECK_EQ(hex, "0x61 0x62 0x63 0x64");

//     CHECK_EQ("", byteToHexStrForPrint(""));
// }

TEST_CASE("test_split_by_char") {
    std::string x("");
    auto splits = split(x, '.');
    CHECK_EQ(splits.size(), 1);
    CHECK_EQ(splits[0], x);

    x = "100.1.";
    splits = split(x, '.');
    CHECK_EQ(splits.size(), 2);
    CHECK_EQ(splits[0], "100");
    CHECK_EQ(splits[1], "1");
}

TEST_CASE("test_split_by_string") {
    std::string x("");

    // 分割字符串为空
    auto splits = split(x, "");
    CHECK_EQ(splits.size(), 1);
    CHECK_EQ(splits[0], x);

    x = "123";
    splits = split(x, "");
    CHECK_EQ(splits.size(), 1);
    CHECK_EQ(splits[0], x);

    // 分割字符串长度为1
    x = "100.1.";
    splits = split(x, ".");
    CHECK_EQ(splits.size(), 2);
    CHECK_EQ(splits[0], "100");
    CHECK_EQ(splits[1], "1");

    // 分割字符串长度为2
    x = "100.1.234.1.56";
    splits = split(x, ".1");
    CHECK_EQ(splits.size(), 3);
    CHECK_EQ(splits[0], "100");
    CHECK_EQ(splits[1], ".234");
    CHECK_EQ(splits[2], ".56");
}
