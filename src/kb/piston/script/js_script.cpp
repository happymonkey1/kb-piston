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
} // end namespace kb::piston