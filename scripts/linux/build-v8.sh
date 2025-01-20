#!/usr/bin/env bash
cd ./vendor/v8/

lastcommit_filename="LASTCHANGE.committime"

# Bugfix
if [ ! -f ./build/util/$lastcommit_filename]; then
    echo "LASTCOMMIT.committime" does not exist, creating...
    epoch_time=$(date +%s)
    echo $epoch_time >> ./build/util/$lastcommit_filename
fi

# Manual V8 build reference: https://v8.dev/docs/build-gn#manual
v8_build_dir="out/x64.static-release"
# TODO: should probably read from file so we can use common args between Windows/Linux scripts...
v8_gn_args="treat_warnings_as_errors = false\nis_component_build = false\nis_debug = false\ntarget_cpu = "x64"\nv8_enable_sandbox = true\nv8_enable_backtrace = true\nv8_enable_disassembler = true\nv8_enable_object_print = true\nv8_enable_verify_heap = true\ndcheck_always_on = false\nv8_static_library = true\nv8_monolithic = true\nv8_use_external_startup_data = false\nuse_custom_libcxx = false\n"
v8_gn_args_filename="args.gn"

# Create static release build target
if [ ! -d $v8_build_dir/$v8_gn_args_filename ]; then
    echo "V8 Static Library build target does not exist, creating..."
    mkdir -p $v8_build_dir
    echo $v8_gn_args >> "$v8_build_dir/$v8_gn_args_filename"
fi

echo "Creating Ninja build scripts..."
bash -c "gn gen $v8_build_dir"

# Compile
ninja -C $v8_build_dir
