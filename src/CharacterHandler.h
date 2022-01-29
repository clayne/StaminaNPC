#pragma once


namespace CharHandler
{
	bool is_strong_attack(RE::Actor* actor);
	bool is_strong_bash(RE::Actor* actor);
	float get_attack_cost(char* _a, RE::BGSAttackData* attack = nullptr);
	float get_block_cost(RE::HitData* data);
	void TESObjectWEAP__Fire_140235240_hooked(RE::TESObjectWEAP* weap, RE::TESObjectREFR* source, RE::TESAmmo* overwriteAmmo, RE::EnchantmentItem* ammoEnchantment, RE::AlchemyItem* poison);
	bool is_strong_block(RE::Actor* a);

	bool deny_player_attack_isstrong(char* _a, RE::BGSAttackData* attack);
	bool is_strong_bow(RE::Actor* a, RE::TESObjectWEAP* bow);
	bool is_strong_bow_NPC(RE::Actor* a, RE::TESObjectWEAP* bow);
}

namespace PlayerHandler
{
	void PlayerControls__sub_140705530_hooked(RE::PlayerControls* controls, int32_t a2, int32_t a3, RE::PlayerCharacter* player);
	float deny_bow_Player(char* _a, RE::TESObjectWEAP* weap, bool left, float drawn_time);
	void deny_jump_Player(RE::Actor* player);
	void cost_jump_Player(RE::Actor* player);
}

namespace SpeedHandler
{
	float hooked_ActorState__sub_1410C3D40(char* a1, float speed);
}
