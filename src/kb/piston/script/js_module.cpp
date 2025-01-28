#include "kb/piston/script/js_module.h"

#include "kb/piston/log/logger.h"

#include <v8.h>

#include "kb/piston/engine/js_engine.h"

namespace kb::piston
{ // start namespace kb::piston


auto js_module::compile_module(
    v8::Isolate* KB_RESTRICT p_isolate,
    std::string_view p_module_source,
    std::string p_module_name
) noexcept -> option<js_module>
{
    KB_PISTON_INFO("[js_engine]: Compiling script module '{script_name}'", "script_name"_a = p_module_name);
    v8::HandleScope handle_scope{ p_isolate };
    // Global context to store js variables
    const auto global_template = v8::ObjectTemplate::New(p_isolate);

    //const v8::Local<v8::Context> context = get_context();
    const v8::Local<v8::Context> module_context = v8::Context::New(p_isolate);

    // Enter the context for running a script
    //v8::Context::Scope context_scope{ context };

    v8::Local<v8::Module> compiled_module;
    module_context->Enter();
    {
        const v8::TryCatch try_catch_script_errors{ p_isolate };

        const auto utf8_source = v8::String::NewFromUtf8(
            p_isolate,
            p_module_source.data(),
            v8::NewStringType::kNormal
        ).ToLocalChecked();

        const auto script_name_utf8 = v8::String::NewFromUtf8(p_isolate, p_module_name.data()).ToLocalChecked();

        const v8::ScriptOrigin script_origin{
            script_name_utf8,
            0,                          // line offset
            0,                          // column offset
            false,                      // cors
            0,                          // script id
            v8::Local<v8::Value>(),     // source map url
            false,                      // is opaque
            false,                      // wasm
            true,                       // es6
        };

        v8::ScriptCompiler::Source script_source{ utf8_source, script_origin };

        // Compile module
        constexpr auto k_compiler_options = v8::ScriptCompiler::kNoCompileOptions;
        if (!v8::ScriptCompiler::CompileModule(
            p_isolate,
            &script_source,
            k_compiler_options).ToLocal(&compiled_module))
        {
            // NOTE: Unconditional copy here, though performance during initialization error is not a big concern
            v8::String::Utf8Value error_message{ p_isolate, try_catch_script_errors.Exception() };

            // TODO: use error
            KB_PISTON_ERROR(
                "[js_engine]: Failed to compile {script_name} module. Error={error}",
                "script_name"_a = p_module_name,
                "error"_a = *error_message
            );
            return std::nullopt;
        }

        v8::Local<v8::Value> module_result;

        auto run_module = [&]()
            {
                // TODO: evaluate microtasks
                return compiled_module->Evaluate(module_context);
            };

        if (!run_module().ToLocal(&module_result))
        {
            // NOTE: Unconditional copy here, though performance during initialization error is not a big concern
            v8::String::Utf8Value error_message{ p_isolate, try_catch_script_errors.Exception() };

            // TODO: use error
            KB_PISTON_ERROR(
                "[js_engine]: Failed to compile {script_name}. Error={error}",
                "script_name"_a = p_module_name,
                "error"_a = *error_message
            );
            return std::nullopt;
        }
    }
    module_context->Exit();

    return std::make_optional<js_module>(
        v8::Global<v8::Context>{ p_isolate, module_context },
        v8::Global<v8::Module>{ p_isolate, compiled_module },
        std::move(p_module_name)
    );
}

} // end namespace kb::piston