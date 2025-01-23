#include "kb/piston/engine/js_engine.h"

#include "kb/piston/util/file_util.h"

#include <libplatform/libplatform.h>

#include "kb/piston/log/logger.h"
#include "kb/piston/runtime/console.h"

namespace kb::piston
{ // start namespace kb::piston

js_engine::js_engine()
{
    // Initialize V8
    v8::V8::InitializeICU();
    m_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(m_platform.get());
    v8::V8::Initialize();

    // v8::V8::SetFlagsFromString("--harmony-shipping");

    m_create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    m_isolate = v8::Isolate::New(m_create_params);

    {
        v8::HandleScope handle_scope{ m_isolate };

        // Create a new context
        auto context = v8::Context::New(m_isolate);
        m_global_context.Reset(m_isolate, context);

        // Template object to create new JS objects
        auto template_object = v8::ObjectTemplate::New(m_isolate);
        template_object->SetInternalFieldCount(1);
        // Create self object
        auto self_object = template_object->NewInstance(context).ToLocalChecked();
        self_object->SetInternalField(0, v8::External::New(m_isolate, this));
        m_self_instance = v8::Global<v8::Object>{
            m_isolate,
            self_object
        };

        // Register runtime APIs
        runtime::console::register_global(m_isolate, context);
    }

    m_script_system.reserve_for_scripts();
}

js_engine::~js_engine() noexcept
{
    m_global_context.Reset();
    m_self_instance.Reset();

    m_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();

    // Clear script system
    m_script_system.m_scripts.clear();
    m_script_system.m_update_functions.clear();
    m_script_system.m_registered_script_count = 0;

    delete m_create_params.array_buffer_allocator;
}

auto js_engine::register_script(const std::filesystem::path& p_path) noexcept -> bool
{
    const auto script_source = util::read_file_into_buffer(p_path);
    auto script_name = p_path.filename().stem().string();

    auto script = compile_script(script_source, std::move(script_name));
    if (!script)
    {
        KB_PISTON_ERROR("[js_engine]: Failed to compile script!");
        return false;
    }

    return register_script(std::move(*script));
}

auto js_engine::register_script(js_script p_js_script) noexcept -> bool
{
    v8::HandleScope handle_scope{ m_isolate };

    // Retrieve globals
    auto script_context = p_js_script.m_context.Get(m_isolate);
    const auto globals = script_context->Global();

    v8::Context::Scope context_scope{ script_context };

    // Get onUpdate function
    {
        auto maybe_update_func = globals->Get(
            script_context,
            v8::String::NewFromUtf8Literal(m_isolate, js_script::k_on_update_js_name)
        );

        v8::Local<v8::Value> update_func;
        if (!maybe_update_func.ToLocal(&update_func))
        {
            KB_PISTON_ERROR(
                "[js_engine]: Failed to find {func_name} in '{script_name}'",
                "func_name"_a = js_script::k_on_update_js_name,
                "script_name"_a = p_js_script.get_name()
            );

            return false; // TODO: return error
        }

        if (!update_func->IsFunction())
        {
            KB_PISTON_ERROR(
                "[js_engine]: Found onUpdate, but it is a 'TODO' instead of a function!"
            );

            return false; // TODO: return error
        }

        m_script_system.m_update_functions.emplace_back(m_isolate, update_func.As<v8::Function>());
    }

    KB_PISTON_INFO("[js_engine]: Registered script '{script}'", "script"_a = p_js_script.get_name());

    m_script_system.m_scripts.push_back(std::move(p_js_script));
    m_script_system.m_registered_script_count++;

    return true;
}

auto js_engine::on_update(time_step_t p_time_step) const -> void
{
    // TODO: set time step in JS context

    for (size_t i = 0; i < m_script_system.m_registered_script_count; ++i)
    {
        const auto& script = m_script_system.m_scripts[i];

        const auto& update_func = m_script_system.m_update_functions[i];
        const auto error = script.on_update(m_isolate, &update_func);

        // TODO: use error
        if (error)
        {
            KB_PISTON_ERROR(
                "[js_engine]: {script_name}.onUpdate failed. Error={error}",
                "script_name"_a = script.get_name(),
                "error"_a = reinterpret_cast<const char*>(error->m_value.data())
            );
        }
    }
}

auto js_engine::handle_exception(const v8::TryCatch& p_try_catch) noexcept -> bool
{
    return handle_exception(p_try_catch.Exception(), p_try_catch.Message());
}

auto js_engine::handle_exception(v8::Local<v8::Value> p_error, v8::Local<v8::Message> p_message) noexcept -> bool
{
    KB_PISTON_ASSERT(false, "Not implemented!");
    return false; // TODO: fix
}

auto js_engine::compile_script(
    std::string_view p_script_source,
    std::string p_script_name
) const noexcept -> option<js_script>
{
    KB_PISTON_INFO("[js_engine]: Compiling script '{script_name}'", "script_name"_a = p_script_name);
    v8::HandleScope handle_scope{ m_isolate };
    // Global context to store js variables
    const auto global_template = v8::ObjectTemplate::New(m_isolate);
    const v8::Local<v8::Context> context = get_context();

    // Enter the context for running a script
    //v8::Context::Scope context_scope{ context };
    v8::Local<v8::Script> script{};
    context->Enter();
    {
        const v8::TryCatch try_catch_script_errors{ m_isolate };

        const auto utf8_source = v8::String::NewFromUtf8(
            m_isolate,
            p_script_source.data(),
            v8::NewStringType::kNormal
        ).ToLocalChecked();

#if 1
        const auto compile_result = v8::Script::Compile(
            context,
            utf8_source
        );
#else
        constexpr auto k_compiler_options = v8::ScriptCompiler::kNoCompileOptions;
        const auto compile_result = v8::ScriptCompiler::CompileModule(
            context,
            utf8_source,
            k_compiler_options
        );
#endif

        if (!compile_result.ToLocal(&script))
        {
            // NOTE: Unconditional copy here, though performance during initialization error is not a big concern
            v8::String::Utf8Value error_message{ m_isolate, try_catch_script_errors.Exception() };

            // TODO: use error
            KB_PISTON_ERROR(
                "[js_engine]: Failed to compile {script_name}. Error={error}",
                "script_name"_a = p_script_name,
                "error"_a = *error_message
            );
            return std::nullopt;
        }

        v8::Local<v8::Value> script_result;
        if (!script->Run(context).ToLocal(&script_result))
        {
            // NOTE: Unconditional copy here, though performance during initialization error is not a big concern
            v8::String::Utf8Value error_message{ m_isolate, try_catch_script_errors.Exception() };

            // TODO: use error
            KB_PISTON_ERROR(
                "[js_engine]: Failed to compile {script_name}. Error={error}",
                "script_name"_a = p_script_name,
                "error"_a = *error_message
            );
            return std::nullopt;
        }
    }
    context->Exit();

    return std::make_optional(js_script{
        v8::Global<v8::Context>{ m_isolate, context },
        v8::Global<v8::Script>{ m_isolate, script },
        std::move(p_script_name)
    });
}

auto js_engine::script_system::reserve_for_scripts() noexcept -> void
{
    m_scripts.reserve(k_reserve_size);
    m_update_functions.reserve(k_reserve_size);
}

} // end namespace kb::piston