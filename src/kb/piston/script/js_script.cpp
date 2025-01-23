#include "kb/piston/script/js_script.h"

#include "kb/piston/log/logger.h"

namespace kb::piston
{ // start namespace kb::piston

js_script::js_script(js_script&& p_other) noexcept
    : m_context{ std::move(p_other.m_context) }, m_script{ std::move(p_other.m_script) },
    m_name{ p_other.m_name }
{
}

auto js_script::operator=(js_script&& p_other) noexcept -> js_script&
{
    std::swap(m_context, p_other.m_context);
    std::swap(m_script, p_other.m_script);
    std::swap(m_name, p_other.m_name);

    return *this;
}

js_script::js_script(v8::Global<v8::Context> p_context, v8::Global<v8::Script> p_script, std::string p_script_name)
    : m_context{ std::move(p_context) }, m_script{ std::move(p_script) },
    m_name{ std::move(p_script_name) }
{
}

auto js_script::on_init() noexcept -> void
{
}

auto js_script::on_update(
    v8::Isolate* KB_RESTRICT p_isolate,
    const v8::Global<v8::Function>* KB_RESTRICT p_update_func
) const noexcept -> option<error>
{
    // KB_PISTON_INFO("[js_script]: {}.onUpdate", m_name);

    if (!p_isolate)
        return js_script_error_t::lib_internal;

    if (!p_update_func)
        return js_script_error_t::lib_internal;

    v8::HandleScope handle_scope{ p_isolate };
    const v8::Local<v8::Context> context = m_context.Get(p_isolate);

    // Enter the context for running a script
    v8::Context::Scope context_scope{ context };
    {
        const v8::TryCatch try_catch_script_errors{ p_isolate };

        // Retrieve script update function
        const auto js_update_func = p_update_func->Get(p_isolate);

        // Receiver is the `this` object in JS
        v8::Local<v8::Value> recv = v8::Null(p_isolate);

        // Call JS function
        const auto maybe_result = js_update_func->Call(
            js_update_func->GetCreationContext(p_isolate).ToLocalChecked(),
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

            return std::make_optional(error{ js_script_error_t::on_update, returned_error_message });
        }

        if (!result->IsNullOrUndefined())
        {
            v8::String::Utf8Value result_value{ p_isolate, result };
            KB_PISTON_DEBUG(
                "[js_script]: {script_name}.onUpdate() returned {value}",
                "script_name"_a = m_name,
                "value"_a = *result_value
            );
        }
    }

    return std::nullopt;
}

} // end namespace kb::piston