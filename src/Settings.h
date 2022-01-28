#pragma once
#include "PCH.h"

struct Settings
{
	using ISetting = AutoTOML::ISetting;
	using bSetting = AutoTOML::bSetting;
	using fSetting = AutoTOML::fSetting;

	static void load()
	{
		try {
			const auto table = toml::parse_file("Data/SKSE/Plugins/StaminaNPC.toml"s);
			for (const auto& setting : ISetting::get_settings()) {
				setting->load(table);
			}
		} catch (const toml::parse_error& e) {
			std::ostringstream ss;
			ss
				<< "Error parsing file \'" << *e.source().path << "\':\n"
				<< '\t' << e.description() << '\n'
				<< "\t\t(" << e.source().begin << ')';
			logger::error(ss.str());
			throw std::runtime_error("failed to load settings"s);
		}
	}

	static inline bSetting meleeNPC{ "Melee"s, "DenyNPC"s, true };           // Deny NPC attacks with low stamina
	static inline bSetting meleePlayer{ "Melee"s, "DenyPlayer"s, true };     // Deny Player attacks with low stamina
	static inline bSetting meleeCostPlayer{ "Melee"s, "DamageNPC"s, true };  // Damage NPC stamina every melee attack
	static inline bSetting meleeCostNPC{ "Melee"s, "DamagePlayer"s, true };  // Damage Player stamina every melee attack
	static inline fSetting meleeWeightMult{ "Melee"s, "WeightMult"s, 0.9f };
	static inline fSetting meleeStaminaMult{ "Melee"s, "StaminaMult"s, 0.05f };
	static inline fSetting meleeBase{ "Melee"s, "Base"s, 6.8f };

	static inline bSetting bashNPC{ "Bash"s, "DenyNPC"s, true };           // Deny NPC bash with low stamina
	static inline bSetting bashPlayer{ "Bash"s, "DenyPlayer"s, true };     // Deny Player bash with low stamina
	static inline bSetting bashCostPlayer{ "Bash"s, "DamageNPC"s, true };  // Damage NPC stamina every bash
	static inline bSetting bashCostNPC{ "Bash"s, "DamagePlayer"s, true };  // Damage Player stamina every bash
	static inline fSetting bashWeightMult{ "Bash"s, "WeightMult"s, 0.45f };
	static inline fSetting bashStaminaMult{ "Bash"s, "StaminaMult"s, 0.025f };
	static inline fSetting bashBase{ "Bash"s, "Base"s, 3.4f };

	static inline fSetting attackTypeMult_powerattack{ "AttackTypeMults"s, "PowerAttack"s, 1.5f };
	static inline fSetting attackTypeMult_powerbash{ "AttackTypeMults"s, "PowerBash"s, 2.0f };


	static inline bSetting blockNPC{ "Block"s, "NPC"s, true };  // deny npc block
	static inline bSetting blockPlayer{ "Block"s, "Player"s, true };  // deny player block
	static inline bSetting costBlock{ "Block"s, "Damage"s, true };  // damage stamina
	static inline fSetting blockWeightMult{ "Block"s, "WeightMult"s, 1.0f };
	static inline fSetting blockStaminaMult{ "Block"s, "StaminaMult"s, 0.01f };
	static inline fSetting blockBase{ "Block"s, "Base"s, 1.0f };

	static inline bSetting rangedNPC{ "Bow"s, "NPC"s, true };  // deny npc bow attack
	static inline bSetting rangedPlayer{ "Bow"s, "Player"s, true };  // deny player bow attack
	static inline bSetting costRanged{ "Bow"s, "Damage"s, true };  // damage stamina
	static inline fSetting bowWeightMult{ "Bow"s, "WeightMult"s, 1.0f };
	static inline fSetting bowStaminaMult{ "Bow"s, "StaminaMult"s, 0.01f };
	static inline fSetting bowBase{ "Bow"s, "Base"s, 1.0f };

	static inline bSetting jumpPlayer{ "Jumps"s, "Player"s, true };  // deny player jump attack
	static inline bSetting costJump{ "Jumps"s, "Damage"s, true };  // damage stamina
	static inline fSetting jumpWeightMult{ "Jumps"s, "WeightMult"s, 1.0f };
	static inline fSetting jumpStaminaMult{ "Jumps"s, "StaminaMult"s, 0.01f };
	static inline fSetting jumpBase{ "Jumps"s, "Base"s, 1.0f };


	static inline bSetting runspeedNPC{ "Speed"s, "NPC"s, true };  // decrease movement speed
};

