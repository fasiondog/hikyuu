/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-8-4
 *      Author: fasiondog
 *
 */

#include <doctest/doctest.h>
#include <hikyuu/Log.h>
#include <hikyuu/utilities/os.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_os test_hikyuu_os
 * @ingroup test_hikyuu_utilities
 * @{
 */

static void createTestFile(const std::string &filename) {
    FILE *fp = fopen(HKU_PATH(filename).c_str(), "wb");
    int len = 10;
    fwrite(&len, sizeof(int), 1, fp);
    fflush(fp);
    fclose(fp);
}

TEST_CASE("test_getUserDir") {
    auto usr_home = getUserDir();
    CHECK_UNARY_FALSE(usr_home.empty());
    HKU_TRACE(usr_home);
}

TEST_CASE("test_dir_operation") {
    // 创建并删除空目录
    std::string dirname("tmp");
    CHECK_UNARY_FALSE(existFile(dirname));

    CHECK_UNARY(createDir(dirname));
    CHECK_UNARY(existFile(dirname));

    CHECK_UNARY(removeDir(dirname));
    CHECK_UNARY_FALSE(existFile(dirname));

    // 删除非空目录
    CHECK_UNARY(createDir(dirname));
    CHECK_UNARY(createDir(fmt::format("{}/{}", dirname, dirname)));
    CHECK_UNARY(createDir(fmt::format("{}/{}/{}", dirname, dirname, dirname)));
    CHECK_UNARY(existFile(fmt::format("{}/{}", dirname, dirname)));
    CHECK_UNARY(existFile(fmt::format("{}/{}/{}", dirname, dirname, dirname)));

    std::string filename = fmt::format("{}/{}/temp.txt", dirname, dirname);
    createTestFile(filename);

    CHECK_UNARY(existFile(filename));
    CHECK_UNARY(removeDir(dirname));
    CHECK_UNARY_FALSE(existFile(dirname));
}

TEST_CASE("test_dir_operation_unicode") {
    // 创建并删除空目录
    std::string dirname("中文");
    CHECK_UNARY_FALSE(existFile(dirname));

    CHECK_UNARY(createDir(dirname));
    CHECK_UNARY(existFile(dirname));

    CHECK_UNARY(removeDir(dirname));
    CHECK_UNARY_FALSE(existFile(dirname));

    // 删除非空目录
    CHECK_UNARY(createDir(dirname));
    CHECK_UNARY(createDir(fmt::format("{}/{}", dirname, dirname)));
    CHECK_UNARY(createDir(fmt::format("{}/{}/{}", dirname, dirname, dirname)));
    CHECK_UNARY(existFile(fmt::format("{}/{}", dirname, dirname)));
    CHECK_UNARY(existFile(fmt::format("{}/{}/{}", dirname, dirname, dirname)));

    std::string filename = fmt::format("{}/{}/中文temp.txt", dirname, dirname);
    createTestFile(filename);

    CHECK_UNARY(existFile(filename));
    CHECK_UNARY(removeDir(dirname));
    CHECK_UNARY_FALSE(existFile(dirname));
}

TEST_CASE("test_removeFile") {
    std::string filename("中文temp.txt");
    removeFile(filename);
    CHECK_UNARY_FALSE(existFile(filename));

    createTestFile(filename);

    CHECK_UNARY(existFile(filename));
    CHECK_UNARY(removeFile(filename));
    CHECK_UNARY_FALSE(existFile(filename));
}

TEST_CASE("test_copyFile") {
    std::string filename("中文temp.txt");
    removeFile(filename);
    CHECK_UNARY_FALSE(existFile(filename));

    createTestFile(filename);
    CHECK_UNARY(existFile(filename));

    copyFile(filename, "中文temp2.txt");
    CHECK_UNARY(existFile("中文temp2.txt"));
    CHECK_UNARY(removeFile("中文temp2.txt"));

    // 85M 左右的文件，在 v831 上拷贝耗时 6s，直接使用 cp 命令耗时 5s
    // {
    //     SPEND_TIME(copyt_85M_file);
    //     copyFile("test_data/large_user.db", "test_data/large_user.db2");
    // }
}

TEST_CASE("test_renameFile") {
    std::string oldname("中文old.txt");
    std::string newname("中文new.txt");

    // 指定的旧名文件不存在
    CHECK_UNARY_FALSE(existFile(oldname));
    CHECK_UNARY_FALSE(renameFile(oldname, newname, false));
    CHECK_UNARY_FALSE(renameFile(oldname, newname, true));
    CHECK_UNARY_FALSE(existFile(newname));
    CHECK_UNARY_FALSE(existFile(newname));

    // 指定的新名未被占用，非覆盖模式
    createTestFile(oldname);
    CHECK_UNARY_FALSE(existFile(newname));
    CHECK_UNARY(renameFile(oldname, newname, false));
    CHECK_UNARY_FALSE(existFile(oldname));
    CHECK_UNARY(existFile(newname));

    // 指定的新名已被占用，非覆盖模式
    createTestFile(oldname);
    CHECK_UNARY(existFile(newname));
    CHECK_UNARY_FALSE(renameFile(oldname, newname, false));
    CHECK_UNARY(existFile(oldname));
    CHECK_UNARY(existFile(newname));

    // 指定的新名未被占用，覆盖模式
    removeFile(newname);
    createTestFile(oldname);
    CHECK_UNARY_FALSE(existFile(newname));
    CHECK_UNARY(renameFile(oldname, newname, true));
    CHECK_UNARY_FALSE(existFile(oldname));
    CHECK_UNARY(existFile(newname));

    // 指定的新名已被占用，覆盖模式
    createTestFile(oldname);
    CHECK_UNARY(existFile(newname));
    CHECK_UNARY(renameFile(oldname, newname, true));
    CHECK_UNARY_FALSE(existFile(oldname));
    CHECK_UNARY(existFile(newname));
    removeFile(newname);
}

TEST_CASE("test_getDiskFreeSpace") {
#if !HKU_OS_WINDOWS
    CHECK_EQ(getDiskFreeSpace(nullptr), Null<uint64_t>());
#endif
    HKU_INFO("disk free space /: {}", getDiskFreeSpace("/"));
    HKU_INFO("disk free space .: {}", getDiskFreeSpace("."));
}

/** @} */
