#pragma once
#include <v8.h>

#include "kb/piston/core/types.h"

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime

enum class log_level_t : u8
{
    trace = 0,
    debug,
    info,
    warn,
    error,

    RESERVED_DO_NOT_USE,
};

class console
{
public:
    static auto register_global(
        v8::Isolate* p_isolate,
        v8::Local<v8::Context> p_global_context
    ) noexcept -> void;

private:
    static auto log_debug_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;
    static auto log_info_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;
    static auto log_warn_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;
    static auto log_error_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;

    static auto log_impl_handler(
        log_level_t p_level,
        const v8::FunctionCallbackInfo<v8::Value>& p_args
    ) noexcept -> void;

    static auto set_isolate(v8::Isolate* p_isolate) noexcept { s_isolate = p_isolate; }

    static auto js_args_to_string(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> std::string;

    static v8::Isolate* s_isolate;

    friend class js_engine;
};

} // end namespace kb::piston::runtime
