#include "kb/piston/runtime/application.h"

#include "kb/piston/core/types.h"
#include "kb/piston/engine/js_engine.h"

namespace kb::piston::runtime
{ // start namespace kb::piston::runtime


auto application::register_with_context(
    v8::Isolate* KB_RESTRICT p_isolate,
    const v8::Local<v8::Context>& p_context
) noexcept -> void
{
    const auto application_template = v8::ObjectTemplate::New(p_isolate);
    application_template->Set(
        v8::String::NewFromUtf8Literal(p_isolate, "exit"),
        v8::FunctionTemplate::New(p_isolate, exit_handler)
    );

    p_context->Global()->Set(
        p_context,
        v8::String::NewFromUtf8Literal(p_isolate, "Application"),
        application_template->NewInstance(p_context).ToLocalChecked()
    );
}

auto application::exit_handler(const v8::FunctionCallbackInfo<v8::Value>& p_args) noexcept -> void
{
    i32 exit_code = 0;

    {
        auto* isolate = piston::js_engine::get_isolate();
        v8::HandleScope handle_scope{ isolate };
        const auto context = piston::js_engine::get_context();

        if (p_args.Length() >= 1)
        {
            const auto& arg = p_args[0];
            if (arg->IsNumber())
            {
                const auto value = arg->ToNumber(context).FromMaybe(v8::Number::New(isolate, 0));
                exit_code = static_cast<i32>(value->Value());
            }
        }
    }

    std::exit(exit_code);
}

} // end namespace kb::piston::runtime