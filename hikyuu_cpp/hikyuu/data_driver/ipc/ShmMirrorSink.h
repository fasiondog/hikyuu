/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"

namespace hku {
namespace ipc {

/**
 * The plugin side implementation interface of the realtime mirror writing (implemented and
 * registered by the shmserver plugin)
 * @details Stock::realtimeUpdate drives this interface through the thin forwarding of the core
 * library (shmMirrorRealtimeUpdate), so that the core library does not need to know the existence
 * of the shared memory publisher. The staging within the publish window / the double writing of the
 * old segment / the replay after taking over are all internal responsibilities of the implementer
 * of this interface, and the forwarder of the core library does not do any staging (see design
 * §5.1).
 * @note This interface is a callback contract on the core library side (dependency inversion), not
 * a plugin control entry, so it is placed in data_driver/ipc/ instead of plugin/interface/ (the
 *       latter is exclusively the plugin control interfaces that inherit PluginBase).
 * @ingroup DataDriver
 */
class HKU_API ShmMirrorSink {
public:
    virtual ~ShmMirrorSink() = default;
    virtual void onRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                  const KRecord& record) = 0;
};

/**
 * Register / unregister the mirror implementation (pass nullptr to unregister); it is called by the
 * plugin at the single point of start / stop
 * @note The pid of the publishing process is recorded on registration, used to exclude the writing
 *       of the forked child process (maintaining the single-writer premise)
 */
HKU_API void registerShmMirrorSink(ShmMirrorSink* sink) noexcept;

/**
 * The existing call entry of Stock::realtimeUpdate; when nothing is registered it returns after a
 * single atomic read only
 * @details The core library implementation only does the atomic fast path judgment + the pid gating
 *          of the forked child process + the virtual call forwarding
 */
HKU_API void shmMirrorRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                     const KRecord& record);

}  // namespace ipc
}  // namespace hku
