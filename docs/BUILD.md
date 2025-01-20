# Dependencies

## Required

- [Git](https://git-scm.com/downloads)
- [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
  - Google development and build tools
  - [Windows](https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md#Setting-up-Windows)
- [Ninja](https://ninja-build.org/)
- [CMake](https://cmake.org/)

## Transitively Included

- [V8 Javascript Engine](https://github.com/v8/v8)
  - Building V8 from source included via internal scripts

### Linux

Ensure there is an alias to `gm.py`:

```
alias gm=/path/to/v8/tools/dev/gm.py
```

# Retrieve source code

To retrieve the source code:
`git clone https://github.com/happymonkey1/kb-piston`

# Build from source code (Linux)

**NOTE:** These instructions use linux related commands/scripts. While there are no dedicated windows build instructions, please substitute the corresponding windows command, arguments, or scripts (`./scripts/windows/`) when running commands below.

## Manual Workflow

When building from source for the first time, we need to set up dependency folder(s):

```
cd PATH/TO/kb-piston
mkdir ./vendor/v8
```

Retrieve v8 dependency source code:

```
cd PATH/TO/kb-piston
./scripts/linux/fetch-v8.sh
```

Build `v8` using the integrated script

```
cd PATH/TO/kb-piston
./scripts/linux/build-v8.sh
```

# FAQ

- `gm` build command fails with error: `ERROR at //build/timestamp.gni:31:19: Script returned non-zero exit code.`
  - Navigate to `vendor/v8/v8/build/util`.
  - Add `LASTCHANGE.committime` with current epoch second ([Epoch Converter](https://www.epochconverter.com/))
  - [Reference](https://groups.google.com/a/chromium.org/g/chromium-discuss/c/9-clGIeN7bI)
- Build fails with error: `FileNotFoundError: [Errno 2] No such file or directory: 'C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\cdb.exe'`
  - Install [Windows 10 Standalone SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
  - [Reference](https://stackoverflow.com/a/42699793)

# References

- V8
  - [Building V8 from source](https://v8.dev/docs/build)
  - [Checking out V8 source code](https://v8.dev/docs/source-code)
  - [Building V8 with GN](https://v8.dev/docs/build-gn)
    - [Compiling V8 as static lib - Google Groups](https://groups.google.com/g/v8-users/c/L_Qc60uOZlk)
    - [GN Gen Error 'No Such file or directory' - Google Groups](https://groups.google.com/a/chromium.org/g/chromium-discuss/c/9-clGIeN7bI)
    - [Build error 'cdb.exe not found' - StackOverflow](https://stackoverflow.com/a/42699793)
