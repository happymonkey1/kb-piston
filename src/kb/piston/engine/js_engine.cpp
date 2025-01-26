#include "kb/piston/engine/js_engine.h"

#include "kb/piston/util/file_util.h"

#include <libplatform/libplatform.h>

#include "kb/piston/log/logger.h"
#include "kb/piston/runtime/console.h"

namespace kb::piston
{ // start namespace kb::piston

v8::Global<v8::Context> js_engine::s_global_context{};
v8::Isolate* js_engine::s_isolate{};

js_engine::js_engine()
{
    // Initialize V8
    v8::V8::InitializeICU();
    m_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(m_platform.get());
    v8::V8::Initialize();

    // v8::V8::SetFlagsFromString("--harmony-shipping");

    m_create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    s_isolate = v8::Isolate::New(m_create_params);

    {
        v8::HandleScope handle_scope{ s_isolate };

        // Create a new context
        auto context = v8::Context::New(s_isolate);
        s_global_context.Reset(s_isolate, context);

        // Template object to create new JS objects
        auto template_object = v8::ObjectTemplate::New(s_isolate);
        template_object->SetInternalFieldCount(1);
        // Create self object
        auto self_object = template_object->NewInstance(context).ToLocalChecked();
        self_object->SetInternalField(0, v8::External::New(s_isolate, this));
        m_self_instance = v8::Global<v8::Object>{
            s_isolate,
            self_object
        };

        // Register runtime APIs
        runtime::console::register_global(s_isolate, context);
    }
}

js_engine::~js_engine() noexcept
{
    s_global_context.Reset();
    m_self_instance.Reset();

    s_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();

    // Clear script system
    m_script_registry.clear<>();

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

    const auto script_handle = m_script_registry.create();
    return register_script(script_handle, std::move(*script));
}

auto js_engine::register_script(
    const script_handle_t p_script_handle,
    js_script p_js_script
) noexcept -> bool
{
    v8::HandleScope handle_scope{ s_isolate };

    // Retrieve globals
    auto script_context = p_js_script.m_context.Get(s_isolate);
    const auto globals = script_context->Global();

    v8::Context::Scope context_scope{ script_context };

    // Get onUpdate function
    register_js_script<script_update_component>(
        p_script_handle,
        p_js_script,
        js_script::k_on_update_js_name,
        globals,
        script_context
    );

    // Get onInit function
    register_js_script<script_init_component>(
        p_script_handle,
        p_js_script,
        js_script::k_on_init_js_name,
        globals,
        script_context
    );

    m_script_registry.emplace<script_component>(p_script_handle, std::move(p_js_script));

    KB_PISTON_INFO("[js_engine]: Registered script '{script}'", "script"_a = p_js_script.get_name());

    return true;
}

auto js_engine::on_init() const noexcept -> void
{
    const auto view = m_script_registry.view<script_component, script_init_component>();
    for (const auto entity : view)
    {
        const auto& script = get_component<script_component>(entity).m_script;
        const auto& script_init_comp = get_component<script_init_component>(entity);

        const auto error = script.on_init(s_isolate, &script_init_comp.m_on_init_func);
        if (error)
        {
            KB_PISTON_ERROR(
                "[js_engine]: {script_name}.onInit failed. Error={error}",
                "script_name"_a = script.get_name(),
                "error"_a = reinterpret_cast<const char*>(error->m_value.data())
            );
        }
    }
}

auto js_engine::on_update(time_step_t p_time_step) const noexcept -> void
{
    // TODO: set time step in JS context

    const auto view = m_script_registry.view<script_component, script_update_component>();
    for (const auto entity : view)
    {
        const auto& script = get_component<script_component>(entity).m_script;
        const auto& script_update_comp = get_component<script_update_component>(entity);

        const auto error = script.on_init(s_isolate, &script_update_comp.m_on_update_func);
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
    v8::HandleScope handle_scope{ s_isolate };
    // Global context to store js variables
    const auto global_template = v8::ObjectTemplate::New(s_isolate);
    const v8::Local<v8::Context> context = get_context();

    // Enter the context for running a script
    //v8::Context::Scope context_scope{ context };
    v8::Local<v8::Script> script{};
    context->Enter();
    {
        const v8::TryCatch try_catch_script_errors{ s_isolate };

        const auto utf8_source = v8::String::NewFromUtf8(
            s_isolate,
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
            v8::String::Utf8Value error_message{ s_isolate, try_catch_script_errors.Exception() };

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
            v8::String::Utf8Value error_message{ s_isolate, try_catch_script_errors.Exception() };

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
        v8::Global<v8::Context>{ s_isolate, context },
        v8::Global<v8::Script>{ s_isolate, script },
        std::move(p_script_name)
    });
}

} // end namespace kb::piston