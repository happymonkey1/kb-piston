#pragma once

#include <v8.h>

#include "kb/piston/core/types.h"
#include "kb/piston/script/js_script.h"

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
    js_engine();
    ~js_engine() noexcept;

    auto register_script(const std::filesystem::path& p_path) noexcept -> bool;
    auto register_script(js_script p_js_script) noexcept -> bool;

    auto on_update(time_step_t p_time_step) const -> void;

    auto get_context() const noexcept -> v8::Local<v8::Context> { return m_global_context.Get(m_isolate); }
    auto get_context() noexcept -> v8::Local<v8::Context> { return m_global_context.Get(m_isolate); }

    auto handle_exception(const v8::TryCatch& p_try_catch) noexcept -> bool;
    auto handle_exception(v8::Local<v8::Value> p_error, v8::Local<v8::Message> p_message) noexcept -> bool;

private:
    auto compile_script(
        std::string_view p_script_source,
        std::string p_script_name = "unnamed_script"
    ) const noexcept -> option<js_script>;

private:
    std::unique_ptr<v8::Platform> m_platform;
    v8::Isolate* m_isolate = nullptr;
    v8::Global<v8::Object> m_self_instance{};
    v8::Global<v8::Context> m_global_context{};
    v8::Isolate::CreateParams m_create_params{};

    struct script_system
    {
        inline static constexpr size_t k_reserve_size = 1024ull;

        std::vector<js_script> m_scripts;
        // Registered JS functions
        std::vector<v8::Global<v8::Function>> m_update_functions{};

        size_t m_registered_script_count = 0;

        auto reserve_for_scripts() noexcept -> void;
    } m_script_system;
};

} // end namespace kb::piston