#pragma once
#define NOMINMAX
#include "SimpleIni.h"
#include "UselessFenixUtils.h"

class Settings
{
	static constexpr auto ini_path_default = "Data/MCM/Config/FenixStaminaOverhaul/settings.ini"sv;
	static constexpr auto ini_path = "Data/MCM/Settings/FenixStaminaOverhaul.ini"sv;

	static bool ReadBool(const CSimpleIniA& ini, const char* section, const char* setting, bool& ans)
	{
		if (ini.GetValue(section, setting)) {
			ans = ini.GetBoolValue(section, setting);
			return true;
		}
		return false;
	}

	static bool ReadFloat(const CSimpleIniA& ini, const char* section, const char* setting, float& ans)
	{
		if (ini.GetValue(section, setting)) {
			ans = static_cast<float>(ini.GetDoubleValue(section, setting));
			return true;
		}
		return false;
	}

	static bool ReadUint32(const CSimpleIniA& ini, const char* section, const char* setting, uint32_t& ans)
	{
		if (ini.GetValue(section, setting)) {
			ans = static_cast<uint32_t>(ini.GetLongValue(section, setting));
			return true;
		}
		return false;
	}

	static void OnConfigClose(RE::TESQuest*) { ReadSettings(); }

	template <const char* header_path, bool _enabled, float _k, float _b>
	class LinearSetting
	{
		static inline bool enabled = _enabled;
		static inline float k = _k;
		static inline float b = _b;

	public:
		static void init(const CSimpleIniA& ini)
		{
			ReadBool(ini, header_path, "bEnabled", enabled);
			ReadFloat(ini, header_path, "fMult", k);
			ReadFloat(ini, header_path, "fBase", b);
		}

		static float get(float skill) { return std::max(0.0f, k * skill + b); }

		static bool is_enabled() { return enabled; }
	};

	static void update_setting(const char* name, float val) {
		auto gameSettings = RE::GameSettingCollection::GetSingleton();
		auto setting = gameSettings->GetSetting(name);
		if (setting)
			setting->data.f = val;
	}

public:
	struct DenyPlayer
	{
		static inline constexpr char header[] = "DenyPlayer";

		enum class Modes : uint32_t
		{
			Allow,
			Scale,
			Deny
		};
		struct Mode
		{
			Modes normal: 2;
			Modes power: 2;
		};
		static inline Mode melee;
		static inline bool bash = true;
		static inline bool block = true;
		static inline bool bow = true;
		static inline bool jump = true;

		static Modes get_melee_mode(RE::BGSAttackData* adata)
		{
			bool power = adata->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack);
			return power ? melee.power : melee.normal;
		}

		static bool read_Modes(const CSimpleIniA& ini, const char* name, Modes& ans)
		{
			uint32_t val;
			if (ReadUint32(ini, header, name, val)) {
				ans = static_cast<Modes>(val);
				return true;
			}

			return false;
		}

		static void init(const CSimpleIniA& ini)
		{
			Modes val;
			if (read_Modes(ini, "uMeleeNormal", val)) {
				DenyPlayer::melee.normal = val;
			}
			if (read_Modes(ini, "uMeleePower", val)) {
				DenyPlayer::melee.power = val;
			}

			ReadBool(ini, header, "bBash", DenyPlayer::bash);
			ReadBool(ini, header, "bBow", DenyPlayer::bow);
			ReadBool(ini, header, "bBlock", DenyPlayer::block);
			ReadBool(ini, header, "bJump", DenyPlayer::jump);
		}
	};
	using DenyModes = DenyPlayer::Modes;

	struct DenyNPC
	{
		static inline constexpr char header[] = "DenyNPC";

		static inline bool melee = true;
		static inline bool bash = true;
		static inline bool block = true;
		static inline bool bow = true;
		static inline bool speed = true;

		static void init(const CSimpleIniA& ini)
		{
			ReadBool(ini, header, "bMelee", DenyNPC::melee);
			ReadBool(ini, header, "bBash", DenyNPC::bash);
			ReadBool(ini, header, "bBow", DenyNPC::bow);
			ReadBool(ini, header, "bBlock", DenyNPC::block);
			ReadBool(ini, header, "bSpeed", DenyNPC::speed);
		}
	};

	struct Costs
	{
		static inline constexpr char header_costs[] = "Costs";
		static inline constexpr char header_skill[] = "CostsSkill";
		static inline constexpr char header_jump[] = "CostsJump";
		static inline constexpr char header_block[] = "CostsBlock";
		static inline constexpr char header_bowshot[] = "CostsBowShot";
		static inline constexpr char header_bowkeep[] = "CostsBowKeep";
		static inline constexpr char header_melee[] = "CostsMelee";
		static inline constexpr char header_bash[] = "CostsBash";
		static inline constexpr char header_swimming[] = "CostsSwimming";

		static class Level
		{
			static inline constexpr char header[] = "CostsLvl";

			static constexpr float lvl0 = 1.0f;

			static inline bool enabled = true;
			static inline float k = 3.0f;
			static inline float f0 = 1.0f;

		public:
			static void init(const CSimpleIniA& ini)
			{
				ReadBool(ini, header, "bEnabled", enabled);
				ReadFloat(ini, header, "fK", k);
				ReadFloat(ini, header, "fF0", f0);
			}

			static float get(int lvl)
			{
				float st0 = 100.0f + 10.0f / k * (lvl0 - 1);
				float st1 = 100.0f + 10.0f / k * (lvl - 1);
				return f0 * st1 / st0;
			}

			static bool is_enabled() { return enabled; }
		} lvlK;

		static class Sprinting
		{
			static inline constexpr char header[] = "CostsSprinting";

			static inline float fSprintStaminaWeightMult = 0.02f;
			static inline float fSprintStaminaWeightBase = 1.0f;
			static inline float fSprintStaminaDrainMult = 7.0f;

		public:
			static void init(const CSimpleIniA& ini)
			{
				ReadFloat(ini, header, "fSprintStaminaWeightMult", fSprintStaminaWeightMult);
				ReadFloat(ini, header, "fSprintStaminaWeightBase", fSprintStaminaWeightBase);
				ReadFloat(ini, header, "fSprintStaminaDrainMult", fSprintStaminaDrainMult);

				update_setting("fSprintStaminaWeightMult", fSprintStaminaWeightMult);
				update_setting("fSprintStaminaWeightBase", fSprintStaminaWeightBase);
				update_setting("fSprintStaminaDrainMult", fSprintStaminaDrainMult);
			}
		} sprinting;

		static LinearSetting<header_skill, true, -1.2f, 1.7f> skillK;
		static LinearSetting<header_jump, true, 0.3f, 10.0f> jump;
		static LinearSetting<header_block, true, 1.0f, -5.0f> block;
		static LinearSetting<header_bowshot, true, 0.6f, 10.0f> bowshot;
		static LinearSetting<header_bowkeep, true, 1.2f, 0.0f> bowkeep;
		static LinearSetting<header_melee, true, 0.3f, 7.0f> melee;
		static LinearSetting<header_bash, true, 1.5f, 5.0f> bash;
		static LinearSetting<header_swimming, true, 0.1f, 1.0f> swimming;

		static inline float powerAttackMult = 1.5;
		static inline float powerBashMult = 2.0;
		static float get_powerActionMult(bool _bash) { return _bash ? powerBashMult : powerAttackMult; }

		static inline bool normalAttackPerks = true;

		static void init(const CSimpleIniA& ini)
		{
			Level::init(ini);
			skillK.init(ini);
			jump.init(ini);
			block.init(ini);
			bowshot.init(ini);
			bowkeep.init(ini);
			melee.init(ini);
			bash.init(ini);
			swimming.init(ini);
			sprinting.init(ini);

			ReadFloat(ini, header_costs, "fPowerAttackMult", powerAttackMult);
			ReadFloat(ini, header_costs, "fPowerBashMult", powerBashMult);
			ReadBool(ini, header_costs, "bNormalAttackPerks", normalAttackPerks);
		}
	};

	class Regen
	{
		static inline constexpr char header[] = "Regen";
		static inline constexpr char header_delay[] = "RegenDelay";

		static inline bool enabled = true;
		static inline float fCombatStaminaRegenRateMult = 0.35f;

	public:
		static class WalkTypes
		{
			static inline constexpr char header[] = "RegenWalkTypes";

			static inline bool enabled = true;
			static inline float stay = 1.3f;
			static inline float walk = 1.0f;
			static inline float run = 0.7f;

		public:
			static void init(const CSimpleIniA& ini)
			{
				ReadBool(ini, header, "bEnabled", enabled);
				ReadFloat(ini, header, "fStay", stay);
				ReadFloat(ini, header, "fWalk", walk);
				ReadFloat(ini, header, "fRun", run);
			}

			static float get(RE::Actor* a)
			{
				if (a->IsPlayerRef() && enabled) {
					// assert !a->actorState1.sprinting
					if (a->actorState1.running) {
						return run;
					} else if (a->actorState1.walking) {
						return walk;
					} else {
						return stay;
					}
				} else {
					return 1.0f;
				}
			}
		} walkK;

		static class Carry
		{
			static inline constexpr char header[] = "RegenCarry";

			static inline bool enabled = true;
			static inline float player_a = -0.5f;
			static inline float player_b = -0.5f;
			static inline float player_c = 1.5f;

		public:
			static void init(const CSimpleIniA& ini)
			{
				ReadBool(ini, header, "bEnabled", enabled);
				ReadFloat(ini, header, "fPlayerA", player_a);
				ReadFloat(ini, header, "fPlayerB", player_b);
				ReadFloat(ini, header, "fPlayerC", player_c);
			}

			static float get(float w, float cwp)
			{
				return enabled ? std::max(0.0f, w * 0.01f * player_a + cwp * player_b + player_c) : 1.0f;
			}
		} carryK;

		static class HP_SP
		{
			static inline constexpr char header[] = "RegenHPSP";

			static inline bool HP_enabled = true;
			static inline bool SP_enabled = true;

			static inline bool HP_down = false;
			static inline bool SP_down = false;

			static float lerpUp(float x)
			{
				constexpr float ay = 0.3f;
				constexpr float bx = 0.2f;
				constexpr float by = 0.7f;
				constexpr float cx = 0.7f;
				constexpr float cy = 0.8f;
				constexpr float dy = 1.4f;

				if (x <= bx) {
					return FenixUtils::lerp(x, 0.0f, ay, bx, by);
				}

				if (x <= cx) {
					return FenixUtils::lerp(x, bx, by, cx, cy);
				}

				return FenixUtils::lerp(x, cx, cy, 1.0f, dy);
			}

			static float lerpDown(float x)
			{
				constexpr float ay = 1.4f;
				constexpr float bx = 0.4f;
				constexpr float by = 1.0f;
				constexpr float cx = 0.8f;
				constexpr float cy = 0.9f;
				constexpr float dy = 0.6f;

				if (x <= bx) {
					return FenixUtils::lerp(x, 0.0f, ay, bx, by);
				}

				if (x <= cx) {
					return FenixUtils::lerp(x, bx, by, cx, cy);
				}

				return FenixUtils::lerp(x, cx, cy, 1.0f, dy);
			}

		public:
			static void init(const CSimpleIniA& ini)
			{
				ReadBool(ini, header, "bEnabledHP", HP_enabled);
				ReadBool(ini, header, "bEnabledSP", SP_enabled);

				ReadBool(ini, header, "bDownHP", HP_down);
				ReadBool(ini, header, "bDownSP", SP_down);
			}

			static float get_HP(float x) { return HP_enabled ? (HP_down ? lerpDown(x) : lerpUp(x)) : 1.0f; }
			static float get_SP(float x) { return SP_enabled ? (SP_down ? lerpDown(x) : lerpUp(x)) : 1.0f; }
		} hpsp;

		static LinearSetting<header_delay, true, 0.05f, 0.2f> delay;

		static void init(const CSimpleIniA& ini)
		{
			ReadBool(ini, header, "bEnabled", enabled);
			ReadFloat(ini, header, "fCombatStaminaRegenRateMult", fCombatStaminaRegenRateMult);

			update_setting("fCombatStaminaRegenRateMult", fCombatStaminaRegenRateMult);

			walkK.init(ini);
			carryK.init(ini);
			hpsp.init(ini);
			delay.init(ini);
		}

		static bool is_enabled() { return enabled; }
	};

	static void ReadSettings_(const char* path)
	{
		CSimpleIniA ini;
		ini.LoadFile(path);

		DenyPlayer::init(ini);
		DenyNPC::init(ini);
		Costs::init(ini);
		Regen::init(ini);
	}

	static void ReadSettings()
	{
		ReadSettings_(ini_path_default.data());
		ReadSettings_(ini_path.data());
	}

	static void RegisterForCloseMCM()
	{
		SKSE::GetPapyrusInterface()->Register([](RE::BSScript::IVirtualMachine* a_vm) -> bool {
			a_vm->RegisterFunction("OnConfigClose", "f314FSO_MCM", OnConfigClose);
			return true;
		});
	}
};
