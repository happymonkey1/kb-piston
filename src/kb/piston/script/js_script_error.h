#pragma once
#include "kb/piston/core/types.h"

namespace kb::piston
{ // start namespace kb::piston

enum class js_script_error_t : u8
{
    lib_internal,

    on_update,
};

} // end namespace kb::piston