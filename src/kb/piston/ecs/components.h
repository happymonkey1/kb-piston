#pragma once
#include "kb/piston/script/js_script.h"

namespace kb::piston
{ // start namespace kb::piston

struct script_component
{
    js_script m_script;
};

struct script_update_component
{
    v8::Global<v8::Function> m_on_update_func;
};

struct script_init_component
{
    v8::Global<v8::Function> m_on_init_func;
};

} // end namespace kb::piston