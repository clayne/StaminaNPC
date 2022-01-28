#pragma once


namespace CharHandler
{
	bool is_strong_attack(RE::Actor* actor);
	bool is_strong_bash(RE::Actor* actor);
	float get_attack_cost(char* _a, RE::BGSAttackData* attack = nullptr);
	float get_block_cost(RE::HitData* data);
	void TESObjectWEAP__Fire_140235240_hooked(RE::TESObjectWEAP* weap, RE::TESObjectREFR* source, RE::TESAmmo* overwriteAmmo, RE::EnchantmentItem* ammoEnchantment, RE::AlchemyItem* poison);
	bool is_strong_bow(RE::Actor* a, RE::TESObjectWEAP* bow);
	bool is_strong_block(RE::Actor* a);

	bool deny_player_attack_isstrong(char* _a, RE::BGSAttackData* attack);
}

namespace PlayerHandler
{
	void Actor__Jump_1405D1F80_hooked(RE::Actor* player);
	void PlayerControls__sub_140705530_hooked(RE::PlayerControls* controls, int32_t a2, int32_t a3, RE::PlayerCharacter* player);
	float get_bow_power_hooked(char* _a, RE::TESObjectWEAP* weap, bool left, float drawn_time);
	void Actor__Jump_1405D1F80_hooked_onlydeny(RE::Actor* player);
}

namespace SpeedHandler
{
	float hooked_ActorState__sub_1410C3D40(char* a1, float speed);
}
