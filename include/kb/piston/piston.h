#pragma once

#include "kb/piston/core/types.h"
#include "kb/piston/engine/js_engine.h"
#include "kb/piston/log/logger.h"

namespace kb::piston
{ // start namespace kb::piston

auto init() noexcept -> void
{
#ifndef KB_PISTON_EXTERNAL_LOGGER
    // TODO: this should probably be handled by internal api exposed through kb::piston::init() when `KB_PISTON_EXTERNAL_LOGGER` is not defined.
    fmtlog::startPollingThread(1000000);
#endif
}

} // end namespace kb::piston