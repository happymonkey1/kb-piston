#pragma once
#include "kb/piston/runtime/application.h"

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime

// Runtime context, which holds configuration and runtime data
struct context_t
{
    application_create_info_t m_application_info;
};

} // end namespace kb::piston::runtime