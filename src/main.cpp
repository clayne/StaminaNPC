#include "REL/Relocation.h"

#include "Hooks.h"
#include "CharacterHandler.h"
#include "Settings.h"
#include "spdlog/sinks/basic_file_sink.h"

#define DEBUG_ON

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef DEBUG_ON
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= "StaminaNPC.log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef DEBUG_ON
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	//logger::info("StaminaNPC v1.0.0");

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "StaminaNPC";
	a_info->version = 1;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

bool is_strong(RE::StaticFunctionTag*, RE::Actor* a)
{
	return CharHandler::is_strong(a);
}

bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm)
{
	a_vm->RegisterFunction("isStrong", "f314ImTired_Utility", is_strong);

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus->Register(RegisterFuncs)) {
		return false;
	}

	Settings::load();
	Hooks::apply_hooks(std::uintptr_t(CharHandler::is_strong), std::uintptr_t(CharHandler::is_strong_shield));

	return true;
}
