#pragma once
#include "PCH.h"

struct Settings
{
	using ISetting = AutoTOML::ISetting;
	using bSetting = AutoTOML::bSetting;

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

	static inline bSetting melee{ "NPC"s, "Melee"s, true };
	static inline bSetting ranged{ "NPC"s, "Ranged"s, true };
	static inline bSetting blockbash{ "NPC"s, "Block"s, true };
	static inline bSetting runspeed{ "NPC"s, "RunSpeed"s, true };

	static inline bSetting meleePlayer{ "Player"s, "Melee"s, true };
	static inline bSetting rangedPlayer{ "Player"s, "Ranged"s, true };
	static inline bSetting blockbashPlayer{ "Player"s, "Block"s, true };
	static inline bSetting ablerunPlayer{ "Player"s, "AbleRun"s, true };
	static inline bSetting ablejumpPlayer{ "Player"s, "AbleJump"s, true };
};

