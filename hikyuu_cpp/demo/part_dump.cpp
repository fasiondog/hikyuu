/*
 * Dump the registered part meta as JSON, the data source of the documentation pipeline.
 *
 * The full introspection walks every prototype, and some prototypes reach into the data layer,
 * so a hikyuu config file is normally required. The JSON goes to a file because the logger
 * writes to the standard output.
 *
 * usage: part_dump <hikyuu_config.ini> <out.json>
 */

#include <cstdio>

#include <hikyuu/hikyuu.h>
#include <hikyuu/part/PartRegistry.h>

using namespace hku;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: part_dump <hikyuu_config.ini> <out.json>\n");
        return 2;
    }

    hikyuu_init(argv[1]);

    std::string json = PartRegistry::instance().toJson();
    FILE* fp = std::fopen(argv[2], "wb");
    if (fp == nullptr) {
        std::fprintf(stderr, "cannot open output file: %s\n", argv[2]);
        return 3;
    }
    std::fwrite(json.data(), 1, json.size(), fp);
    std::fclose(fp);
    return 0;
}
