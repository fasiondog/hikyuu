#include "build_in.h"
#include "hikyuu/part/PartRegistry.h"

namespace hku {

namespace {

/**
 * Register a letter-named mode alias as a part delegating to its semantic part.
 * The meta is copied from the semantic part, so the params and their defaults live only in
 * the semantic factory and an alias can never drift from it.
 */
void registerModeAliasPart(const string& semantic_full_name, const string& name,
                           const string& full_name) {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has(full_name), void());
    PartMeta meta = reg.getMeta(semantic_full_name);
    meta.name = name;
    meta.full_name = full_name;
    reg.registerPart(meta, [semantic_full_name](const Parameter& sc, const PartParamMap& pp) {
        return PartRegistry::instance().create(semantic_full_name, sc, pp);
    });
}

}  // namespace

void registerPart_PF_ModeA() {
    registerPart_PF_SignalAggregate();
    registerModeAliasPart("core.pf.signalaggregate", "PF_ModeA", "core.pf.modea");
}

void registerPart_PF_ModeB() {
    registerPart_PF_Simple();
    registerModeAliasPart("core.pf.simple", "PF_ModeB", "core.pf.modeb");
}

void registerPart_PF_ModeC() {
    registerPart_PF_WithoutAF();
    registerModeAliasPart("core.pf.withoutaf", "PF_ModeC", "core.pf.modec");
}

void registerPortfolioParts() {
    registerPart_PF_SignalAggregate();
    registerPart_PF_Simple();
    registerPart_PF_WithoutAF();
    registerPart_PF_ModeA();
    registerPart_PF_ModeB();
    registerPart_PF_ModeC();
}

}  // namespace hku
