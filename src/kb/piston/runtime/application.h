#pragma once

#include "kb/piston/core/core.h"

#include <v8.h>

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime

class application
{
public:
    static auto register_global(
        v8::Isolate* KB_RESTRICT p_isolate,
        v8::Local<v8::Context> p_global_context
    ) noexcept -> void;

private:
    static auto exit_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;
};

} // end namespace kb::piston::runtime