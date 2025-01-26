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
