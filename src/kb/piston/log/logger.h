#pragma once

#ifndef KB_PISTON_EXTERNAL_LOGGER

#   define FMTLOG_HEADER_ONLY
#   include <fmtlog.h>
#   include <fmt/ranges.h>

using namespace fmt::literals;

#   define KB_PISTON_TRACE(format, ...) logd(format, __VA_ARGS__)
#   define KB_PISTON_DEBUG(format, ...) logd(format, __VA_ARGS__)
#   define KB_PISTON_INFO(format, ...) logi(format, __VA_ARGS__)
#   define KB_PISTON_WARN(format, ...) logw(format, __VA_ARGS__)
#   define KB_PISTON_ERROR(format, ...) loge(format, __VA_ARGS__)

#else

#ifndef KB_PISTON_TRACE
#   define KB_PISTON_TRACE(format, ...)
#endif
#ifndef KB_PISTON_DEBUG
#   define KB_PISTON_DEBUG(format, ...)
#endif
#ifndef KB_PISTON_INFO(format, ...)
#   define KB_PISTON_INFO
#endif
#ifndef KB_PISTON_WARN(format, ...)
#   define KB_PISTON_WARN
#endif
#ifndef KB_PISTON_ERROR(format, ...)
#   define KB_PISTON_ERROR
#endif

#endif


