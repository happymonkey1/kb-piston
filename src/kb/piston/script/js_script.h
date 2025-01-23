#pragma once

#include "kb/piston/core/core.h"
#include "kb/piston/core/types.h"
#include "kb/piston/error/js_error.h"
#include "kb/piston/script/js_script_error.h"

#include <v8.h>

#include <filesystem>

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

    [[nodiscard]] auto on_update(
        v8::Isolate* KB_RESTRICT p_isolate,
        const v8::Global<v8::Function>* KB_RESTRICT p_update_func
    ) const noexcept -> option<error>;

private:
    v8::Global<v8::Context> m_context;
    v8::Global<v8::Script> m_script;
    std::string m_name;

    friend class js_engine;
};

} // end namespace kb::piston