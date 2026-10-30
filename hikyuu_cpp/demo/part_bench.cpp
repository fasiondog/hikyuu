/*
 * Micro benchmark for the lazy introspection overhead of PartRegistry.
 *
 * Phase 1 runs while the pending-introspection counter is > 0 (the steady state until every part
 * has been visited once): every getMeta/create takes the introspect mutex and returns without
 * doing work. Once the counter reaches 0 the same calls only pay one atomic acquire load, i.e.
 * strictly less than the numbers printed here. The full introspection is NOT triggered on
 * purpose: prototypes of the context-bound indicators (ADVANCE ...) crash without loaded data,
 * which is exactly why the introspection is lazy.
 *
 * usage: part_bench [loops]
 */

#include <chrono>
#include <cstdio>
#include <cstdlib>

#include "hikyuu/indicator/crt/MA.h"
#include "hikyuu/part/PartRegistration.h"
#include "hikyuu/part/PartRegistry.h"

using namespace hku;
namespace ch = std::chrono;

template <class Fn>
static double bench(const char* label, size_t loops, Fn&& fn) {
    fn();  // warm up
    auto start = ch::steady_clock::now();
    for (size_t i = 0; i < loops; i++) {
        fn();
    }
    double ns = ch::duration_cast<ch::nanoseconds>(ch::steady_clock::now() - start).count() / loops;
    fmt::print("{:<46} {:>10.1f} ns/op\n", label, ns);
    return ns;
}

int main(int argc, char* argv[]) {
    const size_t loops = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1000000;
    auto& reg = PartRegistry::instance();

    fmt::print("registry size: {}, pending counter > 0 in this phase\n", reg.getNameList().size());
    bench("has() (pure map lookup baseline)", loops, [&] { (void)reg.has("core.ind.ma"); });
    bench("getMeta() of an introspected part", loops, [&] { (void)reg.getMeta("core.ind.ma"); });

    const size_t create_loops = loops / 10;
    Parameter sc;
    sc.set<int64_t>("n", 10);
    bench("create() of MA(n=10)", create_loops, [&] {
        auto part = reg.create("core.ind.ma", sc);
        (void)part;
    });

    return 0;
}
