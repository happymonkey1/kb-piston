#pragma once
#include <v8-array-buffer.h>

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime

class event
{
public:
    static auto register_with_context(
        v8::Isolate* p_isolate,
        v8::Local<v8::Context> p_context
    ) noexcept -> void;

private:
    static auto event_raised_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void;
};

} // end namespace kb::piston::runtime
