#pragma once

#include <v8.h>

#include "kb/piston/core/types.h"
#include "kb/piston/log/logger.h"
#include "kb/piston/script/js_script.h"
#include "kb/piston/ecs/components.h"

#include <entt/entt.hpp>

#include <vector>


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

    auto register_script(const std::filesystem::path& p_path) noexcept -> bool;
    auto register_script(
        script_handle_t p_script_handle,
        js_script p_js_script
    ) noexcept -> bool;

    auto on_init() const noexcept -> void;
    auto on_update(time_step_t p_time_step) const noexcept -> void;

    static auto get_context() noexcept -> v8::Local<v8::Context> { return s_global_context.Get(s_isolate); }
    static auto get_isolate() noexcept -> v8::Isolate* { return s_isolate; }

    auto handle_exception(const v8::TryCatch& p_try_catch) noexcept -> bool;
    auto handle_exception(v8::Local<v8::Value> p_error, v8::Local<v8::Message> p_message) noexcept -> bool;

    template <typename ComponentT>
    auto get_component(script_handle_t p_script_handle) const noexcept -> const ComponentT&
    {
        KB_PISTON_ASSERT(
            m_script_registry.all_of<ComponentT>(p_script_handle),
            "[js_engine]: Can not retrieve component that does not exist on entity!"
        );
        return m_script_registry.get<ComponentT>(p_script_handle);
    }

    template <typename ComponentT>
    auto get_component(script_handle_t p_script_handle) noexcept -> ComponentT&
    {
        KB_PISTON_ASSERT(
            m_script_registry.all_of<ComponentT>(p_script_handle),
            "[js_engine]: Can not retrieve component that does not exist on entity!"
        );
        return m_script_registry.get<ComponentT>(p_script_handle);
    }

private:
    [[nodiscard]] auto compile_script(
        std::string_view p_script_source,
        std::string p_script_name = "unnamed_script"
    ) const noexcept -> option<js_script>;

    template <typename ComponentT, int N>
    [[nodiscard]] auto register_js_script(
        const script_handle_t p_script_handle,
        const js_script& p_js_script,
        const char (&p_function_name)[N],
        const v8::Local<v8::Object>& p_js_globals,
        v8::Local<v8::Context>& p_script_context
    ) noexcept -> bool;

private:
    std::unique_ptr<v8::Platform> m_platform;
    static v8::Isolate* s_isolate;
    v8::Global<v8::Object> m_self_instance{};
    static v8::Global<v8::Context> s_global_context;
    v8::Isolate::CreateParams m_create_params{};

    entt::registry m_script_registry{};
};

template <typename ComponentT, int N>
auto js_engine::register_js_script(
    const script_handle_t p_script_handle,
    const js_script& p_js_script,
    const char (&p_function_name)[N],
    const v8::Local<v8::Object>& p_js_globals,
    v8::Local<v8::Context>& p_script_context
) noexcept -> bool
{
    auto maybe_func = p_js_globals->Get(
        p_script_context,
        v8::String::NewFromUtf8Literal(s_isolate, p_function_name)
    );

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

    auto* isolate = get_isolate();
    if (func->IsNullOrUndefined() || !func->IsFunction())
    {
        const auto func_type = func->TypeOf(isolate);
        v8::String::Utf8Value func_type_str{ isolate, func_type };

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

    m_script_registry.emplace<ComponentT>(
        p_script_handle,
        v8::Global<v8::Function>{ s_isolate, func.As<v8::Function>() }
    );

    return true;
}

} // end namespace kb::piston