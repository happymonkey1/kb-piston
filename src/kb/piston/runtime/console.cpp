#include "kb/piston/runtime/console.h"

#include "kb/piston/log/logger.h"

#include <sstream>

#include "kb/piston/core/core.h"

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime

namespace details
{ // start namespace kb::piston::runtime::details

constexpr const char* k_log_level_names[static_cast<size_t>(log_level_t::RESERVED_DO_NOT_USE)] = {
    "trace",
    "debug",
    "info",
    "warn",
    "error",
};

constexpr auto log_level_to_c_str(log_level_t p_log_level) noexcept -> const char*
{
    return k_log_level_names[static_cast<size_t>(p_log_level)];
}

} // end namespace kb::piston::runtime::details

v8::Isolate* console::s_isolate = nullptr;

auto console::register_global(v8::Isolate* p_isolate, v8::Local<v8::Context> p_global_context) noexcept -> void
{
    KB_PISTON_INFO("[console]: Registering Console APIs");

    const auto console_template = v8::ObjectTemplate::New(p_isolate);
    // Set functions on the console template object
    console_template->Set(
        v8::String::NewFromUtf8Literal(p_isolate, "log"),
        v8::FunctionTemplate::New(p_isolate, log_info)
    );

    console_template->Set(
        v8::String::NewFromUtf8Literal(p_isolate, "debug"),
        v8::FunctionTemplate::New(p_isolate, log_debug)
    );
    console_template->Set(
        v8::String::NewFromUtf8Literal(p_isolate, "info"),
        v8::FunctionTemplate::New(p_isolate, log_info)
    );
    console_template->Set(
        v8::String::NewFromUtf8Literal(p_isolate, "warn"),
        v8::FunctionTemplate::New(p_isolate, log_warn)
    );
    console_template->Set(
        v8::String::NewFromUtf8Literal(p_isolate, "error"),
        v8::FunctionTemplate::New(p_isolate, log_error)
    );

    p_global_context->Global()->Set(
        p_global_context,
        v8::String::NewFromUtf8Literal(p_isolate, "console"),
        console_template->NewInstance(p_global_context).ToLocalChecked()
    );

    set_isolate(p_isolate);
}

auto console::log_debug(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
    log_impl(log_level_t::debug, p_args);
}

auto console::log_info(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
    log_impl(log_level_t::info, p_args);
}

auto console::log_warn(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
    log_impl(log_level_t::warn, p_args);
}

auto console::log_error(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
    log_impl(log_level_t::error, p_args);
}

auto console::log_impl(log_level_t p_level, const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
    KB_PISTON_ASSERT(s_isolate, "[console]: Isolate pointer can not be null!");

    const char* log_level_name = details::log_level_to_c_str(p_level);
    constexpr const char* log_format = "[JS Console] [{}]: {}";
    auto args_str = js_args_to_string(p_args);

    switch (p_level)
    {
    case log_level_t::trace:
        KB_PISTON_TRACE(log_format, log_level_name, args_str);
        break;
    case log_level_t::debug:
        KB_PISTON_DEBUG(log_format, log_level_name, args_str);
        break;
    case log_level_t::info:
        KB_PISTON_INFO(log_format, log_level_name, args_str);
        break;
    case log_level_t::warn:
        KB_PISTON_WARN(log_format, log_level_name, args_str);
        break;
    case log_level_t::error:
        KB_PISTON_ERROR(log_format, log_level_name, args_str);
        break;
    }

    // Wtf does this do?
    p_args.GetReturnValue().Set(p_args.Holder());
}

auto console::js_args_to_string(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> std::string
{
    std::stringstream ss;
    for (i32 i = 0; i < p_args.Length(); ++i)
    {
        if (i > 0)
            ss << ' ';

        v8::String::Utf8Value arg_str{ s_isolate, p_args[i] };
        ss << *arg_str;
    }

    return ss.str();
}

} // end namespace kb::piston::runtime
