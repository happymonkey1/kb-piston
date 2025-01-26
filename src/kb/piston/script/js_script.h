#pragma once

#include "kb/piston/core/core.h"
#include "kb/piston/core/types.h"
#include "kb/piston/error/js_error.h"
#include "kb/piston/script/js_script_error.h"

#include <v8.h>

#include <filesystem>

#include "kb/piston/log/logger.h"

namespace kb::piston
{ // start namespace kb::piston

class js_script
{
public:
    inline static constexpr size_t k_error_capacity = 1024;
    using error = js_error<js_script_error_t, u8_fixed_string<k_error_capacity>>;

    inline static constexpr char k_on_init_js_name[] = "onInit";
    inline static constexpr char k_on_update_js_name[] = "onUpdate";
    inline static constexpr char k_on_render_js_name[] = "onRender";
    inline static constexpr char k_on_render_2d_js_name[] = "onRender2d";
    inline static constexpr char k_on_shutdown_js_name[] = "onShutdown";

public:
    js_script() noexcept = default;

    // Deleted copy constructor
    js_script(const js_script&) noexcept = delete;
    // Deleted copy assign operator
    auto operator=(const js_script&) noexcept -> js_script& = delete;
    // Default move constructor
    js_script(js_script&& p_other) noexcept;
    // Default move assign operator
    auto operator=(js_script&& p_other) noexcept -> js_script&;

    js_script(v8::Global<v8::Context> p_context, v8::Global<v8::Script> p_script, std::string p_script_name);

    ~js_script() noexcept = default;

    auto get_name() const noexcept -> const std::string& { return m_name; }

    auto on_init() noexcept -> void;

    [[nodiscard]] auto on_init(
        v8::Isolate* KB_RESTRICT p_isolate,
        const v8::Global<v8::Function>* KB_RESTRICT p_update_func
    ) const noexcept -> option<error>
    {
        return call_function(p_isolate, p_update_func, k_on_init_js_name);
    }

    [[nodiscard]] auto on_update(
        v8::Isolate* KB_RESTRICT p_isolate,
        const v8::Global<v8::Function>* KB_RESTRICT p_update_func
    ) const noexcept -> option<error>
    {
        return call_function(p_isolate, p_update_func, k_on_update_js_name);
    }

private:
    template <int N>
    [[nodiscard]] auto call_function(
        v8::Isolate* KB_RESTRICT p_isolate,
        const v8::Global<v8::Function>* KB_RESTRICT p_update_func,
        const char(&p_func_name)[N]
    ) const noexcept -> option<error>;

private:
    v8::Global<v8::Context> m_context;
    v8::Global<v8::Script> m_script;
    std::string m_name;

    friend class js_engine;
};

template <int N>
auto js_script::call_function(
    v8::Isolate* KB_RESTRICT p_isolate,
    const v8::Global<v8::Function>* KB_RESTRICT p_func,
    const char(& p_func_name)[N]
) const noexcept -> option<error>
{
    // KB_PISTON_INFO("[js_script]: {}.onUpdate", m_name);

    if (!p_isolate)
        return js_script_error_t::lib_internal;

    if (!p_func)
        return js_script_error_t::lib_internal;

    v8::HandleScope handle_scope{ p_isolate };
    const v8::Local<v8::Context> context = m_context.Get(p_isolate);

    // Enter the context for running a script
    v8::Context::Scope context_scope{ context };
    {
        const v8::TryCatch try_catch_script_errors{ p_isolate };

        // Retrieve script update function
        const auto js_func = p_func->Get(p_isolate);

        // Receiver is the `this` object in JS
        v8::Local<v8::Value> recv = v8::Null(p_isolate);

        // Call JS function
        const auto maybe_result = js_func->Call(
            js_func->GetCreationContext(p_isolate).ToLocalChecked(),
            recv,
            0,
            nullptr
        );

        v8::Local<v8::Value> result;
        if (!maybe_result.ToLocal(&result))
        {
            // Write exception into fixed string buffer
            const auto error_message = try_catch_script_errors.Exception().As<v8::String>();
            u8_fixed_string<k_error_capacity> returned_error_message{};
            error_message->WriteUtf8V2(
                p_isolate,
                reinterpret_cast<char*>(returned_error_message.data()),
                returned_error_message.size()
            );

            return std::make_optional(error{
                js_script_error_t::illegal_state_exception,
                returned_error_message
            });
        }

        if (!result->IsNullOrUndefined())
        {
            v8::String::Utf8Value result_value{ p_isolate, result };
            KB_PISTON_DEBUG(
                "[js_script]: {script_name}.{func_name}() returned {value}",
                "script_name"_a = m_name,
                "func_name"_a = p_func_name,
                "value"_a = *result_value
            );
        }
    }

    return std::nullopt;
}

} // end namespace kb::piston