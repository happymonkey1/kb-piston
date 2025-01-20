
#include <v8.h>
#include <libplatform/libplatform.h>

#if __cplusplus <= 201703L
#error "C++20 or later required."
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    const auto platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params{};
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);

    {
        v8::Isolate::Scope isolate_scope{ isolate };

        // Create a stack allocated handle scope
        v8::HandleScope handle_scope{ isolate };

        // Create a new context
        const v8::Local<v8::Context> context = v8::Context::New(isolate);

        // Enter the context for running a script
        v8::Context::Scope context_scope{ context };
        {
            // Create a string containing javascript source code
            const v8::Local<v8::String> source = v8::String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");

            // Compile source code
            const v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

            // Run the script
            const v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

            // Convert the response to a string and print
            v8::String::Utf8Value utf8{ isolate, result };
            printf("%s\n", *utf8);
        }
    }

    // Shutdown V8
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;

    return 0;
}