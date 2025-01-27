#include "kb/piston/runtime/event.h"

#include <v8.h>

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime


auto event::register_with_context(v8::Isolate* p_isolate, v8::Local<v8::Context> p_context) noexcept -> void
{
    const auto event_template = v8::ObjectTemplate::New(p_isolate);

    /*event_template->Set(
        
    )*/

}

auto event::event_raised_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
}

} // end namespace kb::piston::runtime