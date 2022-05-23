#include "REL/Relocation.h"

#include "Hooks.h"
#include "CharacterHandler.h"
#include "Settings.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef DEBUG_ON
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= "FenixStaminaOverhaul.log"sv;
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
		logger::critical("Unsupported runtime version {}", ver.string());
		return false;
	}

	return true;
}

namespace Papyrus
{
	float get_level_k(RE::StaticFunctionTag*, RE::Actor* a) { return Costs::Level::get_level_k(a); }
	float get_skill_k_one(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return Costs::Skill::get_skill_k(a, RE::ActorValue::kOneHanded);
	}
	float get_skill_k_two(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return Costs::Skill::get_skill_k(a, RE::ActorValue::kTwoHanded);
	}
	float get_skill_k_blo(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return Costs::Skill::get_skill_k(a, RE::ActorValue::kBlock);
	}
	float get_skill_k_arh(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return Costs::Skill::get_skill_k(a, RE::ActorValue::kArchery);
	}
	void ReadSettings(RE::StaticFunctionTag*) { return Settings::ReadSettings(); }
	float get_blocking_thing_weight(RE::StaticFunctionTag*, RE::Actor* a) {
		return Costs::Equipement::get_blocking_thing_weight(a);
	}
	float get_block_cost_base(RE::StaticFunctionTag*, RE::Actor* a, float w)
	{
		return Costs::Block::get_block_cost_base(a, w);
	}
	float get_curiass_weight(RE::StaticFunctionTag*, RE::Actor* a) { return Costs::Equipement::get_curiass_weight(a); }
	float get_jump_cost_base(RE::StaticFunctionTag*, RE::Actor* a) { return Costs::Jump::get_jump_cost_base(a); }
	float get_bow_cost_base(RE::StaticFunctionTag*, RE::Actor* a) { return Costs::Bow::get_bow_cost_base(a); }
	float get_bow_weight(RE::StaticFunctionTag*, RE::Actor* a) { return Costs::Equipement::get_bow_weight(a); }
	float get_bow_cost_keep_base(RE::StaticFunctionTag*, RE::Actor* a) { return Costs::Bow::get_bow_cost_keep_base(a); }
	float get_cost_meleeBash_base_(RE::StaticFunctionTag*, RE::Actor* a, bool bash, bool power, bool left)
	{
		return Costs::MeleeBash::get_cost_meleeBash_base_(a, bash, power, left);
	}
	float get_skill_k_melee(RE::StaticFunctionTag*, RE::Actor* a, bool bash) {
		return Costs::Skill::get_skill_k(a, Costs::MeleeBash::get_action_skill_(a, false, bash));
	}
	float get_attacking_thing_weight(RE::StaticFunctionTag*, RE::Actor* a, bool left, bool bash) {
		return Costs::Equipement::get_attacking_thing_weight(a, left, bash);
	}
	float get_w(RE::StaticFunctionTag*, RE::Actor* a) { return Regen::get_w(a); }
	float get_cwp(RE::StaticFunctionTag*, RE::Actor* a) { return Regen::get_cwp(a); }
	float get_carry_k(RE::StaticFunctionTag*, RE::Actor* a) { return Regen::get_carry_k(a); }
	float get_HP_k(RE::StaticFunctionTag*, RE::Actor* a) { return Regen::get_HP_k(a); }
	float get_SP_k(RE::StaticFunctionTag*, RE::Actor* a) { return Regen::get_SP_k(a); }
	float get_SP_pers(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return FenixUtils::getAV_pers(a, RE::ActorValue::kStamina);
	}
	float get_HP_pers(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return FenixUtils::getAV_pers(a, RE::ActorValue::kHealth);
	}
	float get_swimming_cost_base(RE::StaticFunctionTag*, RE::Actor* a)
	{
		return Costs::Swimming::get_swimming_cost_base(a);
	}
	
	void Register()
	{
		SKSE::GetPapyrusInterface()->Register([](RE::BSScript::IVirtualMachine* a_vm) -> bool {
			a_vm->RegisterFunction("get_swimming_cost_base", "f314FSO_MCM", get_swimming_cost_base);
			a_vm->RegisterFunction("get_HP_pers", "f314FSO_MCM", get_HP_pers);
			a_vm->RegisterFunction("get_SP_pers", "f314FSO_MCM", get_SP_pers);
			a_vm->RegisterFunction("get_SP_k", "f314FSO_MCM", get_SP_k);
			a_vm->RegisterFunction("get_HP_k", "f314FSO_MCM", get_HP_k);
			a_vm->RegisterFunction("get_level_k", "f314FSO_MCM", get_level_k);
			a_vm->RegisterFunction("get_skill_k_one", "f314FSO_MCM", get_skill_k_one);
			a_vm->RegisterFunction("get_skill_k_two", "f314FSO_MCM", get_skill_k_two);
			a_vm->RegisterFunction("get_skill_k_blo", "f314FSO_MCM", get_skill_k_blo);
			a_vm->RegisterFunction("get_skill_k_arh", "f314FSO_MCM", get_skill_k_arh);
			a_vm->RegisterFunction("ReadSettings", "f314FSO_MCM", ReadSettings);
			a_vm->RegisterFunction("get_curiass_weight", "f314FSO_MCM", get_curiass_weight);
			a_vm->RegisterFunction("get_jump_cost_base", "f314FSO_MCM", get_jump_cost_base);
			a_vm->RegisterFunction("get_block_cost_base", "f314FSO_MCM", get_block_cost_base);
			a_vm->RegisterFunction("get_blocking_thing_weight", "f314FSO_MCM", get_blocking_thing_weight);
			a_vm->RegisterFunction("get_bow_weight", "f314FSO_MCM", get_bow_weight);
			a_vm->RegisterFunction("get_bow_cost_base", "f314FSO_MCM", get_bow_cost_base);
			a_vm->RegisterFunction("get_bow_cost_keep_base", "f314FSO_MCM", get_bow_cost_keep_base);
			a_vm->RegisterFunction("get_cost_meleeBash_base_", "f314FSO_MCM", get_cost_meleeBash_base_);
			a_vm->RegisterFunction("get_skill_k_melee", "f314FSO_MCM", get_skill_k_melee);
			a_vm->RegisterFunction("get_attacking_thing_weight", "f314FSO_MCM", get_attacking_thing_weight);
			a_vm->RegisterFunction("get_w", "f314FSO_MCM", get_w);
			a_vm->RegisterFunction("get_cwp", "f314FSO_MCM", get_cwp);
			a_vm->RegisterFunction("get_carry_k", "f314FSO_MCM", get_carry_k);
			return true;
		});
	}
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Hooks::apply_hooks();

		Settings::RegisterForCloseMCM();
		Settings::ReadSettings();
		Papyrus::Register();
		break;
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
	if (!g_messaging) {
		logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
		return false;
	}

	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(1 << 10);

	g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

	return true;
}
