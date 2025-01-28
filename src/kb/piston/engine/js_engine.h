#pragma once

#include <v8.h>

#include "kb/piston/core/types.h"
#include "kb/piston/log/logger.h"
#include "kb/piston/script/js_script.h"
#include "kb/piston/ecs/components.h"
#include "kb/piston/event/orchestrator.h"
#include "kb/piston/event/meta.h"

#include <entt/entt.hpp>

#include <vector>

#include "kb/piston/runtime/application.h"
#include "kb/piston/runtime/context.h"
#include "kb/piston/util/file_util.h"


#ifndef KB_ENGINE

#else
// Includes specific to KablunkEngine
#endif


namespace kb::piston
{ // start namespace kb::piston

class js_engine
{
public:
    // TODO: should be kb::handle
    using script_handle_t = entt::entity;

public:
    js_engine();
    ~js_engine() noexcept;

    template <event::meta::PistonEventT... Events>
    auto register_script(const std::filesystem::path& p_path) noexcept -> bool;

    template <event::meta::PistonEventT... Events>
    auto register_script(
        script_handle_t p_script_handle,
        js_script p_js_script
    ) noexcept -> bool;

    /**
     * \brief Call `onInit` JS functions that have been registered
     */
    auto on_init() const noexcept -> void;

    /**
     * \brief Call `onUpdate` JS functions that have been registered
     * \param p_time_step Delta time from the last `onUpdate` call 
     */
    auto on_update(time_step_t p_time_step) const noexcept -> void;

    /**
     * \brief Handler for registered events which dispatches to registered JS callbacks
     * \tparam EventT 
     * \param p_event 
     */
    template <event::meta::PistonEventT EventT>
    auto on_event(EventT* KB_RESTRICT p_event) const noexcept -> void;

    /**
     * \brief Register runtime APIs (console.log, etc) within a context
     * \param p_isolate Isolate instance
     * \param p_context Context to register runtimes within
     * \param p_runtime_context Piston runtime data
     */
    static auto register_runtime_apis(
        v8::Isolate* KB_RESTRICT p_isolate,
        const v8::Local<v8::Context>& p_context,
        const runtime::context_t& p_runtime_context
    ) noexcept -> void;

    auto handle_exception(const v8::TryCatch& p_try_catch) noexcept -> bool;
    auto handle_exception(v8::Local<v8::Value> p_error, v8::Local<v8::Message> p_message) noexcept -> bool;

    template <typename ComponentT>
    [[nodiscard]] auto get_component(script_handle_t p_script_handle) const noexcept -> const ComponentT&
    {
        KB_PISTON_ASSERT(
            m_script_registry.all_of<ComponentT>(p_script_handle),
            "[js_engine]: Can not retrieve component that does not exist on entity!"
        );
        return m_script_registry.get<ComponentT>(p_script_handle);
    }

    template <typename ComponentT>
    [[nodiscard]] auto get_component(script_handle_t p_script_handle) noexcept -> ComponentT&
    {
        KB_PISTON_ASSERT(
            m_script_registry.all_of<ComponentT>(p_script_handle),
            "[js_engine]: Can not retrieve component that does not exist on entity!"
        );
        return m_script_registry.get<ComponentT>(p_script_handle);
    }

private:
    /**
     * \brief Register a JS function in the internal ecs registry
     * \tparam ComponentT 
     * \tparam N 
     * \param p_script_handle Handle to the entity within the internal registry
     * \param p_js_script JS script to register the function with
     * \param p_function_name Function name to look for
     * \param p_js_globals V8 Global data
     * \param p_script_context Local script context
     * \return Boolean indicating success
     */
    template <typename ComponentT, int N>
    [[nodiscard]] auto register_js_script(
        const script_handle_t p_script_handle,
        const js_script& p_js_script,
        const char (&p_function_name)[N],
        const v8::Local<v8::Object>& p_js_globals,
        v8::Local<v8::Context>& p_script_context
    ) noexcept -> bool;

private:
    // V8 Platform
    std::unique_ptr<v8::Platform> m_platform;
    // Isolated V8 instance
    v8::Isolate* m_isolate;
    // Global context for the piston engine
    v8::Global<v8::Context> m_global_context;
    // Piston runtime data
    std::unique_ptr<runtime::context_t> m_runtime_context{ nullptr };
    // JS object instance for the piston engine
    v8::Global<v8::Object> m_self_instance{};
    // JS object template for the piston engine
    v8::Global<v8::ObjectTemplate> m_global_template;
    // Isolated V8 instance parameters (such as allocator)
    v8::Isolate::CreateParams m_create_params{};
    // Piston ecs registry
    entt::registry m_script_registry{};
};

template <event::meta::PistonEventT... Events>
auto js_engine::register_script(const std::filesystem::path& p_path) noexcept -> bool
{
    const auto script_source = util::read_file_into_buffer(p_path);
    auto script_name = p_path.filename().stem().string();

    auto script = js_script::compile_script(
        m_isolate,
        *m_runtime_context,
        std::string_view{ script_source },
        std::move(script_name)
    );

    if (!script)
    {
        KB_PISTON_ERROR("[js_engine]: Failed to compile script!");
        return false;
    }

    const auto script_handle = m_script_registry.create();
    return register_script<Events...>(script_handle, std::move(*script));
}

template <event::meta::PistonEventT... Events>
auto js_engine::register_script(script_handle_t p_script_handle, js_script p_js_script) noexcept -> bool
{
    v8::HandleScope handle_scope{ m_isolate };

    // Retrieve globals
    auto script_context = p_js_script.m_context.Get(m_isolate);
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
    // TODO: handle return value
    register_js_script<script_init_component>(
        p_script_handle,
        p_js_script,
        js_script::k_on_init_js_name,
        globals,
        script_context
    );

    // Get events
    if constexpr (sizeof...(Events) != 0)
    {
        // Compile time iteration of events.
        // Tries to register event functions based off piston event definition.
        using events_tuple_t = std::tuple<Events...>;
        meta::for_each<events_tuple_t>([&]<typename T>()
            {
                register_js_script<script_event_callback_component<T>>(
                    p_script_handle,
                    p_js_script,
                    T::get_js_event_function_name(),
                    globals,
                    script_context
                );
            }
        );
    }

    m_script_registry.emplace<script_component>(p_script_handle, std::move(p_js_script));

    KB_PISTON_INFO("[js_engine]: Registered script '{script}'", "script"_a = p_js_script.get_name());

    return true;
}

template <event::meta::PistonEventT EventT>
auto js_engine::on_event(EventT* KB_RESTRICT p_event) const noexcept -> void
{
    const auto view = m_script_registry.view<script_component, script_event_callback_component<EventT>>();
    for (const auto entity : view)
    {
        const auto& script = get_component<script_component>(entity).m_script;
        const auto& event_callback_func = get_component<script_event_callback_component<EventT>>(entity).m_on_event_callback;

        script.template on_event<EventT>(m_isolate, &event_callback_func, p_event);
    }
}

template <typename ComponentT, int N>
auto js_engine::register_js_script(
    const script_handle_t p_script_handle,
    const js_script& p_js_script,
    const char (&p_function_name)[N],
    const v8::Local<v8::Object>& p_js_globals,
    v8::Local<v8::Context>& p_script_context
) noexcept -> bool
{
    // Retrieve the js function from the context
    auto maybe_func = p_js_globals->Get(
        p_script_context,
        v8::String::NewFromUtf8Literal(m_isolate, p_function_name)
    );

    // Try loading
    v8::Local<v8::Value> func;
    if (!maybe_func.ToLocal(&func))
    {
        KB_PISTON_ERROR(
            "[js_engine]: Failed to find {func_name} in '{script_name}'",
            "func_name"_a = p_function_name,
            "script_name"_a = p_js_script.get_name()
        );

        return false; // TODO: return error
    }

    // Ensure function was found
    if (func->IsNullOrUndefined() || !func->IsFunction())
    {
        const auto func_type = func->TypeOf(m_isolate);
        v8::String::Utf8Value func_type_str{ m_isolate, func_type };

        if (func->IsNullOrUndefined())
        {
            KB_PISTON_ERROR(
                "[js_engine]: Failed to register {func_name} in '{script_name}', it is a '{type}'!",
                "func_name"_a = p_function_name,
                "script_name"_a = p_js_script.get_name(),
                "type"_a = *func_type_str
            );
        }
        else
        {
            KB_PISTON_ERROR(
                "[js_engine]: Failed to register {func_name} in '{script_name}', it is a '{type}' instead of a function!",
                "func_name"_a = p_function_name,
                "script_name"_a = p_js_script.get_name(),
                "type"_a = *func_type_str
            );
        }

        return false; // TODO: return error
    }

    // Register the function in the internal ecs registry
    m_script_registry.emplace<ComponentT>(
        p_script_handle,
        v8::Global<v8::Function>{ m_isolate, func.As<v8::Function>() }
    );

    return true;
}

} // end namespace kb::piston