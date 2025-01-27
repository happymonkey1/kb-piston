#include "kb/piston/engine/js_engine.h"

#include "kb/piston/util/file_util.h"

#include <libplatform/libplatform.h>

#include "kb/piston/log/logger.h"
#include "kb/piston/runtime/application.h"
#include "kb/piston/runtime/console.h"

namespace kb::piston
{ // start namespace kb::piston

v8::Global<v8::Context> js_engine::s_global_context{};
v8::Isolate* js_engine::s_isolate{};
v8::Global<v8::ObjectTemplate> js_engine::s_global_template{};

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
        const auto context = v8::Context::New(s_isolate);
        s_global_context.Reset(s_isolate, context);

        // context->Enter();

        // Template object to create new JS global object
        // Used to access runtime API, as well as global data (such as delta time)
        auto global_template_object = v8::ObjectTemplate::New(s_isolate);

        // Register runtime APIs
        // NOTE: moved to script creation context
        // TODO: we should be able to globally register once, instead of each script context no?
        // register_runtime_apis(context);

        // Set static handle to the global template object
        s_global_template.Reset(s_isolate, global_template_object);
        global_template_object->SetInternalFieldCount(1);
        // Create self object
        const auto self_object = global_template_object->NewInstance(context).ToLocalChecked();
        self_object->SetInternalField(0, v8::External::New(s_isolate, this));
        m_self_instance = v8::Global<v8::Object>{
            s_isolate,
            self_object
        };
    }
}

js_engine::~js_engine() noexcept
{
    // Clear script system
    m_script_registry.clear<>();

    // s_global_context.Get(s_isolate)->Exit();
    s_global_context.Reset();
    m_self_instance.Reset();

    s_isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();

    delete m_create_params.array_buffer_allocator;
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

    KB_PISTON_INFO("[js_engine]: Finished script onInit() calls");
}

auto js_engine::on_update(time_step_t p_time_step) const noexcept -> void
{
    // TODO: set time step in JS context

    const auto view = m_script_registry.view<script_component, script_update_component>();
    for (const auto entity : view)
    {
        const auto& script = get_component<script_component>(entity).m_script;
        const auto& script_update_comp = get_component<script_update_component>(entity);

        const auto error = script.on_update(s_isolate, &script_update_comp.m_on_update_func);
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

auto js_engine::register_runtime_apis(const v8::Local<v8::Context>& p_context) noexcept -> void
{
    KB_PISTON_ASSERT(s_isolate, "[js_engine]: Isolate can not be null while registering runtime APIs!");

    runtime::console::register_with_context(s_isolate, p_context);
    runtime::application::register_with_context(s_isolate, p_context);

    KB_PISTON_INFO("[js_engine]: Finished registering runtime APIs.");
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

} // end namespace kb::piston