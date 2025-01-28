#pragma once

#include "kb/piston/core/core.h"
#include "kb/piston/core/types.h"

#include <v8.h>

#include <string>

namespace kb::piston
{ // start namespace kb::piston

class js_module
{
public:
    js_module() noexcept = default;
    ~js_module() noexcept = default;

    js_module(
        v8::Global<v8::Context> p_context,
        v8::Global<v8::Module> p_module,
        std::string p_name
    ) noexcept
        : m_context{ std::move(p_context) }, m_module{ std::move(p_module) },
        m_name{ std::move(p_name) }
    { }

    [[nodiscard]] static auto compile_module(
        v8::Isolate* KB_RESTRICT p_isolate,
        std::string_view p_module_source,
        std::string p_module_name = "unnamed_module"
    ) noexcept -> option<js_module>;

private:
    v8::Global<v8::Context> m_context;
    v8::Global<v8::Module> m_module;
    std::string m_name;
};

} // end namespace kb::piston