#pragma once


#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "AutoTOML.hpp"

//#define DEBUG_ON

#ifdef DEBUG_ON
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

namespace logger = SKSE::log;

#define DLLEXPORT __declspec(dllexport)
