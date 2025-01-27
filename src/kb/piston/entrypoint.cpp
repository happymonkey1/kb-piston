#include "kb/piston/piston.h"

#include <csignal>
#include <random>

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

namespace test
{ // start namespace ::test

enum class event_type_t
{
    exit,
    foo,
    random_event,
};

PISTON_EVENT_BEGIN(foo_event, event_type_t::foo, "onFooEvent")
PISTON_EVENT_END()

PISTON_EVENT_BEGIN(exit_event, event_type_t::exit, "onExitEvent")
PISTON_EVENT_END()

PISTON_EVENT_BEGIN(random_event, event_type_t::random_event, "onRandomEvent")
PISTON_EVENT_END()

auto test_events_standalone() noexcept -> void
{
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

} // end namespace ::test

} // end namespace ::details

// construct events
enum class event_type_t
{
    random_event,
};

PISTON_EVENT_BEGIN(random_event, event_type_t::random_event, "onRandomEvent")
PISTON_EVENT_END()

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    piston_internal::register_signal_handlers();

    // TODO: this should probably be handled by internal api exposed through kb::piston::init() when `KB_PISTON_EXTERNAL_LOGGER` is not defined.
    fmtlog::startPollingThread(1000000);

    KB_PISTON_INFO("Starting Piston JS Engine");
    kb::piston::js_engine engine{};

    using events_t = std::tuple<random_event>;

    const auto event_orchestrator = kb::piston::event::make_orchestrator<events_t>(
        // Bind the JS engine on event handler
        [&](random_event* KB_RESTRICT p_event) -> bool { engine.on_event(p_event); return true; }
    );

    std::vector<const char*> scripts_to_load{};
    scripts_to_load.emplace_back("examples/basic-testing/entity.js");
    scripts_to_load.emplace_back("examples/basic-testing/serialization.js");
    scripts_to_load.emplace_back("examples/basic-testing/events.js");

    for (const char* script_name : scripts_to_load)
    {
        if (!engine.register_script<random_event>(script_name))
        {
            KB_PISTON_ERROR("Fatal script loading error!");
            return -1;
        }
    }

    engine.on_init();

    // Create a random number generator
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 10000);

    bool running = true;
    std::chrono::time_point<std::chrono::steady_clock> end;
    // Start update loop
    while (running)
    {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::seconds>(now - end);

        // Trigger random event
        const auto random_value = dist(rng);
        if (random_value == 99)
        {
            random_event event{};
            event_orchestrator.dispatch(event);
        }

        engine.on_update(static_cast<kb::piston::f32>(delta.count()));

        end = std::chrono::high_resolution_clock::now();
    }

    return 0;
}
