#include "kb/piston/script/js_script.h"

#include "kb/piston/engine/js_engine.h"
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

auto js_script::compile_script(
    std::string_view p_script_source,
    std::string p_script_name
) noexcept -> option<js_script>
{
    KB_PISTON_INFO("[js_engine]: Compiling script '{script_name}'", "script_name"_a = p_script_name);
    auto* isolate = js_engine::get_isolate();
    v8::HandleScope handle_scope{ isolate };

    const v8::Local<v8::Context> script_context = v8::Context::New(
        isolate,
        nullptr,
        js_engine::get_global_template()
    );
    // const auto script_context = js_engine::get_context();

    // This feels dirty, but we are registering the runtime APIs with each new script context
    // TODO: There should be a better way, potentially using object templates that I have not figured out yet...
    js_engine::register_runtime_apis(script_context);

#if 0
    // Debugging
    if constexpr (false)
    {
        const auto global_object = js_engine::get_global_template()->NewInstance(script_context).ToLocalChecked();

        const auto global_object_str = v8::JSON::Stringify(script_context, global_object).ToLocalChecked();

        v8::String::Utf8Value str{ isolate, global_object_str };
        KB_PISTON_INFO("[js_script] [DEBUG]: {obj}", "obj"_a = *str);

        const auto console_obj = global_object->Get(script_context, v8::String::NewFromUtf8Literal(isolate, "console")).ToLocalChecked();

        v8::Local<v8::Value> log_func;
        const auto log_func_maybe = console_obj.As<v8::Object>()->Get(script_context, v8::String::NewFromUtf8Literal(isolate, "log"));
        KB_PISTON_ASSERT(log_func_maybe.ToLocal(&log_func), "wtf log is not here!");


        v8::Local<v8::Value> argv[1] = {
            v8::Local<v8::String>{ v8::String::NewFromUtf8Literal(isolate, "manual log invoke!") }
        };

        log_func.As<v8::Function>()->Call(isolate, script_context, v8::Null(isolate), 1, argv);
    }
#endif

    // Enter the context for running a script
    v8::Context::Scope context_scope{ script_context };
    v8::Local<v8::Script> script{};
    {
        const v8::TryCatch try_catch_script_errors{ isolate };

        const auto utf8_source = v8::String::NewFromUtf8(
            isolate,
            p_script_source.data(),
            v8::NewStringType::kNormal
        ).ToLocalChecked();

        const auto compile_result = v8::Script::Compile(
            script_context,
            utf8_source
        );

        if (!compile_result.ToLocal(&script))
        {
            // NOTE: Unconditional copy here, though performance during initialization error is not a big concern
            v8::String::Utf8Value error_message{ isolate, try_catch_script_errors.Exception() };

            // TODO: use error
            KB_PISTON_ERROR(
                "[js_engine]: Failed to compile {script_name}. Error={error}",
                "script_name"_a = p_script_name,
                "error"_a = *error_message
            );

            return std::nullopt;
        }

        v8::Local<v8::Value> script_result;
        if (!script->Run(script_context).ToLocal(&script_result))
        {
            // NOTE: Unconditional copy here, though performance during initialization error is not a big concern
            v8::String::Utf8Value error_message{ isolate, try_catch_script_errors.Exception() };

            // TODO: use error
            KB_PISTON_ERROR(
                "[js_engine]: Failed to compile {script_name}. Error={error}",
                "script_name"_a = p_script_name,
                "error"_a = *error_message
            );

            return std::nullopt;
        }
    }

    return std::make_optional<js_script>(
        v8::Global<v8::Context>{ isolate, script_context },
        v8::Global<v8::Script>{ isolate, script },
        std::move(p_script_name)
    );
}

auto js_script::on_init() noexcept -> void
{
}
} // end namespace kb::piston