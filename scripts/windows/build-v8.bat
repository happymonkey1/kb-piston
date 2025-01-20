@echo off
cd .\vendor\v8\

REM Bugfix
if not exist ".\build\\util\\LASTCHANGE.committime" (
    REM echo "LASTCOMMIT.committime" does not exist, creating..."
    REM Windows does not have a nice `date +%s` utility...
    echo 1737338584 > ".\\build\\util\\LASTCHANGE.committime"
)

REM Manual V8 build reference: https://v8.dev/docs/build-gn#manual
REM Create static release build target
if not exist "out\x64.static-release\args.gn" (
    echo "V8 Static Library build target does not exist, creating..."
    mkdir ".\out\x64.static-release\"
    REM TODO: should probably read from file so we can use common args between Windows/Linux scripts...
    (
        echo treat_warnings_as_errors = false
        echo is_component_build = false
        echo is_debug = false
        echo target_cpu = "x64"
        echo v8_enable_sandbox = true
        echo v8_enable_backtrace = true
        echo v8_enable_disassembler = true
        echo v8_enable_object_print = true
        echo v8_enable_verify_heap = true
        echo dcheck_always_on = false
        echo v8_static_library = true
        echo v8_monolithic = true
        echo v8_use_external_startup_data = false
        echo use_custom_libcxx = false
    ) >> ".\out\x64.static-release\args.gn"
)

echo "Creating Ninja build scripts..."
cmd /c "gn gen out\x64.static-release"

REM Compile
ninja -C out\x64.static-release

cd ../../