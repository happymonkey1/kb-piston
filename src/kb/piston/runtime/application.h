#pragma once

#include "kb/piston/core/core.h"

#include <v8.h>

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime

struct application_create_info_t
{
    // Name of the application
    std::string p_name = "kb-piston-js";
};

class application
{
public:
    static auto register_with_context(
        v8::Isolate* KB_RESTRICT p_isolate,
        const v8::Local<v8::Context>& p_context,
        const application_create_info_t& p_application_info
    ) noexcept -> void;

private:
    static auto exit_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;
};

} // end namespace kb::piston::runtime