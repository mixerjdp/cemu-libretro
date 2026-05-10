#include "LibretroLogging.h"
#include "libretro/libretro.h"
#include <cstdlib>
#include <string>

extern retro_log_printf_t s_log_cb;

static LibretroLoggingCallbacks* s_libretro_logging = nullptr;

static uint64 libretro_get_normal_logging_flags()
{
	uint64 flags = 0;
	flags |= cemuLog_getFlag(LogType::Force);
	flags |= cemuLog_getFlag(LogType::APIErrors);
	flags |= cemuLog_getFlag(LogType::UnsupportedAPI);
	return flags;
}

static uint64 libretro_get_verbose_logging_flags()
{
	uint64 flags = libretro_get_normal_logging_flags();
	flags |= cemuLog_getFlag(LogType::CoreinitLogging);
	flags |= cemuLog_getFlag(LogType::GX2);
	flags |= cemuLog_getFlag(LogType::SoundAPI);
	flags |= cemuLog_getFlag(LogType::InputAPI);
	flags |= cemuLog_getFlag(LogType::TextureCache);
	flags |= cemuLog_getFlag(LogType::Patches);
	return flags;
}

static bool libretro_verbose_logging_from_env()
{
	const char* value = std::getenv("CEMU_LIBRETRO_VERBOSE_LOGGING");
	if (!value || value[0] == '\0')
		value = std::getenv("CEMU_LIBRETRO_DEBUG_VERBOSE");
	return value && value[0] != '\0' && value[0] != '0';
}

void LibretroLoggingCallbacks::Log(std::string_view filter, std::string_view message)
{
	if (!s_log_cb)
		return;

	// Map Cemu log types to RetroArch log levels
	retro_log_level level = RETRO_LOG_INFO;
	
	// Check for error/warning keywords
	if (message.find("error") != std::string_view::npos || 
	    message.find("Error") != std::string_view::npos ||
	    message.find("ERROR") != std::string_view::npos ||
	    message.find("failed") != std::string_view::npos ||
	    message.find("Failed") != std::string_view::npos)
	{
		level = RETRO_LOG_ERROR;
	}
	else if (message.find("warning") != std::string_view::npos || 
	         message.find("Warning") != std::string_view::npos ||
	         message.find("WARN") != std::string_view::npos)
	{
		level = RETRO_LOG_WARN;
	}

	// Format: [Cemu:filter] message
	if (!filter.empty())
		s_log_cb(level, "[Cemu:%.*s] %.*s\n", (int)filter.size(), filter.data(), (int)message.size(), message.data());
	else
		s_log_cb(level, "[Cemu] %.*s\n", (int)message.size(), message.data());
}

void LibretroLoggingCallbacks::Log(std::string_view filter, std::wstring_view message)
{
	if (!s_log_cb)
		return;

	// Convert wstring to string for libretro logging
	std::string msg_str;
	msg_str.reserve(message.size());
	for (wchar_t c : message)
	{
		if (c < 128)
			msg_str.push_back((char)c);
		else
			msg_str.push_back('?'); // Replace non-ASCII with ?
	}

	Log(filter, msg_str);
}

void libretro_init_logging()
{
	if (!s_libretro_logging)
	{
		s_libretro_logging = new LibretroLoggingCallbacks();
		cemuLog_setCallbacks(s_libretro_logging);
		libretro_set_verbose_logging(libretro_verbose_logging_from_env());
	}
}

void libretro_set_verbose_logging(bool enabled)
{
	cemuLog_setActiveLoggingFlags(enabled ? libretro_get_verbose_logging_flags() : libretro_get_normal_logging_flags());
}

void libretro_shutdown_logging()
{
	if (s_libretro_logging)
	{
		cemuLog_clearCallbacks();
		delete s_libretro_logging;
		s_libretro_logging = nullptr;
	}
}
