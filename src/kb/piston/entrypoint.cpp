#include "kb/piston/piston.h"

#include <csignal>

#include "kb/piston/event/event.h"
#include "kb/piston/event/orchestrator.h"
#include "kb/piston/meta/meta.h"

// TODO: should be library internal
namespace piston_internal
{ // start namespace ::details

auto signal_handler(kb::piston::i32 p_signal) noexcept -> void
{
    fmtlog::stopPollingThread();
    fmtlog::poll();

    std::signal(p_signal, SIG_DFL);
    std::raise(p_signal);
}

auto register_signal_handlers() noexcept -> void
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);
}

} // end namespace ::details


int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
#if 0
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
#endif

    piston_internal::register_signal_handlers();

    // TODO: this should probably be handled by internal api exposed through kb::piston::init() when `KB_PISTON_EXTERNAL_LOGGER` is not defined.
    fmtlog::startPollingThread(1000000);

    // construct test events
    {
        enum class event_type_t
        {
            exit = 0,
            foo,
        };

        PISTON_EVENT_BEGIN(foo_event, event_type_t::foo)
        PISTON_EVENT_END()

        PISTON_EVENT_BEGIN(exit_event, event_type_t::exit)
        PISTON_EVENT_END()

        auto on_exit_event_handler = [](exit_event* KB_RESTRICT p_event) -> bool
            {
                KB_PISTON_INFO("exit event handled!");
                return true;
            };

        auto on_foo_event_handler = [](foo_event* KB_RESTRICT p_event) -> bool
            {
                KB_PISTON_INFO("foo event handled!");
                return true;
            };

        using events_t = std::tuple<exit_event, foo_event>;

        const auto event_orchestrator = kb::piston::event::make_orchestrator<events_t>(
            on_exit_event_handler,
            on_foo_event_handler
        );

        auto test_event = exit_event{};
        event_orchestrator.dispatch(test_event);
    }

    KB_PISTON_INFO("Starting Piston JS Engine");
    kb::piston::js_engine engine{};
    if (!engine.register_script("examples/basic-testing/entity.js"))
        return -1;

    bool running = true;
    std::chrono::time_point<std::chrono::steady_clock> end;
    while (running)
    {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::seconds>(now - end);

        engine.on_update(static_cast<kb::piston::f32>(delta.count()));

        end = std::chrono::high_resolution_clock::now();
    }

    return 0;
}
