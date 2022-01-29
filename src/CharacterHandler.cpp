#include "CharacterHandler.h"
#include "Utils.h"
#include "Settings.h"

namespace WeaponCosts
{
	float get_cost(float weight, float stamina, float k, float b, float c)
	{
		return weight * k + b + stamina * c;
	}

	float get_cost(float weight, float stamina, double k, double b, double c)
	{
		return get_cost(weight, stamina, static_cast<float>(k), static_cast<float>(b), static_cast<float>(c));
	}
}

namespace WeaponsHandler
{
	static const float MASS_MULT = 11.0f;

	static RE::TESObjectWEAP* get_weapon(RE::Actor* a, bool left)
	{
		auto _weap = a->GetEquippedObject(left);
		if (!_weap)
			return nullptr;

		auto weap = _weap->As<RE::TESObjectWEAP>();
		return weap;
	}

	static float get_weapon_weight(RE::Actor* a)
	{
		auto weap = get_weapon(a, false);
		if (weap)
			return weap->GetWeight();
		else
			return std::max(0.0f, 1.0f + (a->GetActorValue(RE::ActorValue::kMass) - 1.0f) * MASS_MULT);
	}

	static RE::TESObjectARMO* get_shield(RE::Actor* a)
	{
		auto _shield = a->GetEquippedObject(true);
		if (!_shield)
			return nullptr;

		auto shield = _shield->As<RE::TESObjectARMO>();
		if (!shield)
			return nullptr;
		return shield->GetSlotMask() == RE::BGSBipedObjectForm::BipedObjectSlot::kShield ? shield : nullptr;
	}

	static float get_shield_weight(RE::Actor* a)
	{
		auto shield = get_shield(a);
		if (shield)
			return shield->GetWeight();

		auto weap = get_weapon(a, false);
		if (weap)
			return weap->GetWeight();
		else
			return std::max(0.0f, (a->GetActorValue(RE::ActorValue::kMass) - 1.0f) * MASS_MULT);
	}

	static float get_curiass_weight(RE::Actor* a)
	{
		auto armo = a->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
		if (armo)
			return armo->GetWeight();
		else
			return std::max(0.0f, (a->GetActorValue(RE::ActorValue::kMass) - 1.0f) * MASS_MULT);
	}

	//static std::pair<RE::TESObjectWEAP*, bool> check_bow(RE::Character* player)
	//{
	//	auto left_weap = get_weapon(player, true);
	//	if (!left_weap)
	//		return { nullptr, false };
	//
	//	if (left_weap->GetWeaponType() == RE::WEAPON_TYPE::kBow)
	//		return { left_weap, true };
	//	else
	//		return { left_weap, false };
	//}
}

namespace CharHandler
{
	inline constexpr REL::ID UnarmedWeap(static_cast<std::uint64_t>(514923));

	RE::TESObjectWEAP* get_UnarmedWeap()
	{
		REL::Relocation<RE::NiPointer<RE::TESObjectWEAP>*> singleton{ UnarmedWeap };
		return singleton->get();
	}

	static RE::TESObjectWEAP* Actor__GetWeapon_140625EB0(RE::Actor* a, bool left)
	{
		using func_t = decltype(&Actor__GetWeapon_140625EB0);
		REL::Relocation<func_t> func{ REL::ID(37621) };
		return func(a, left);
	}

	static void ApplyPerkEntryPoint_14032ECE0(int id, RE::Actor* a, void* a3, float* k)
	{
		using func_t = decltype(&ApplyPerkEntryPoint_14032ECE0);
		REL::Relocation<func_t> func{ REL::ID(23073) };
		return func(id, a, a3, k);
	}

	static bool is_strong(RE::Actor* actor, float cost)
	{
		auto cur_stamina = actor->GetActorValue(RE::ActorValue::kStamina);
		auto all_stamina = actor->GetBaseActorValue(RE::ActorValue::kStamina);
		if (3 * cost > all_stamina) {
			cost = all_stamina / 3;
		}

		return all_stamina <= 0.0f || cur_stamina < 0.0f || cur_stamina >= cost;
	}

	static float ActorValueOwner__get_attack_cost_1403BEC90(char* _a, RE::BGSAttackData* attack)
	{
		using func_t = decltype(&ActorValueOwner__get_attack_cost_1403BEC90);
		REL::Relocation<func_t> func{ REL::ID(25863) };
		return func(_a, attack);
	}

	static float get_attack_cost_newversion(RE::Actor* a, bool isPower, bool isBash)
	{
		float cost;
		auto stamina = a->GetBaseActorValue(RE::ActorValue::kStamina);
		if (isBash) {
			float weight = WeaponsHandler::get_shield_weight(a);
			cost = WeaponCosts::get_cost(weight, stamina, *Settings::bashWeightMult, *Settings::bashBase, *Settings::bashStaminaMult);
		} else {
			float weight = WeaponsHandler::get_weapon_weight(a);
			cost = WeaponCosts::get_cost(weight, stamina, *Settings::meleeWeightMult, *Settings::meleeBase, *Settings::meleeStaminaMult);
		}

		if (isPower)
			cost *= static_cast<float>(isBash ? *Settings::attackTypeMult_powerbash : *Settings::attackTypeMult_powerattack);

		return cost;
	}

	static void ApplyPerkEntryPoint(RE::Actor* a, float* k) {
		auto weap = Actor__GetWeapon_140625EB0(a, false);
		if (!weap)
			weap = get_UnarmedWeap();

		ApplyPerkEntryPoint_14032ECE0(27, a, weap, k);
	}

	static float get_attack_cost_newversion(RE::Actor* a, RE::BGSAttackData* attack)
	{
		bool isPower = attack->data.flags.underlying() & 0x4;
		bool isBash = attack->data.flags.underlying() & 0x2;

		float cost = get_attack_cost_newversion(a, isPower, isBash);
		ApplyPerkEntryPoint(a, &cost);

		// 1.0  almost always
		// 0.5  for power dual (I need count it)
		// 0.01 for sphere (norm)
		// 3.0  for one werewolf attack (ok then)
		return cost * attack->data.staminaMult;
	}

	static bool deny_player_attack_isstrong_origin(char* _a, RE::BGSAttackData* attack)
	{
		auto a = reinterpret_cast<RE::Actor*>(_a - 0xB0);
		return ActorValueOwner__get_attack_cost_1403BEC90(_a, attack) <= 0.0f ||
			a->GetActorValue(RE::ActorValue::kStamina) > 0.0f;
	}

	bool deny_player_attack_isstrong(char* _a, RE::BGSAttackData* attack)
	{
		bool isBash = attack->data.flags.underlying() & 0x2;
		bool new_version = isBash && *Settings::bashPlayer || !isBash && *Settings::meleePlayer;
		auto a = reinterpret_cast<RE::Actor*>(_a - 0xB0);

		return new_version ? is_strong(a, get_attack_cost_newversion(a, attack)) :
                             deny_player_attack_isstrong_origin(_a, attack);
	}

	// with checking incombat
	static bool is_strong_NPC(RE::Actor* actor, float cost)
	{
		if (!actor || !actor->IsInCombat())
			return true;

		return is_strong(actor, cost);
	}

	// fires every attask. Should run ApplyPerkEntryPoint_14032ECE0
	float get_attack_cost(char* _a, RE::BGSAttackData* attack)
	{
		bool isBash = attack->data.flags.underlying() & 0x2;

		auto a = reinterpret_cast<RE::Actor*>(_a - 0xB0);
		bool isPlayer = a->IsPlayer();
		bool new_version = false;

		new_version = isBash && isPlayer && *Settings::bashCostPlayer;
		new_version = !isBash && isPlayer && *Settings::meleeCostPlayer;
		new_version = isBash && !isPlayer && *Settings::bashCostNPC;
		new_version = !isBash && !isPlayer && *Settings::meleeCostNPC;

		return new_version ? get_attack_cost_newversion(a, attack) :
                             ActorValueOwner__get_attack_cost_1403BEC90(_a, attack);
	}

	float get_block_cost(RE::Actor* target, RE::Actor* aggressor)
	{
		float target_weigh = WeaponsHandler::get_shield_weight(target);
		float aggressor_weigh = WeaponsHandler::get_weapon_weight(aggressor);
		float stamina = target->GetActorValue(RE::ActorValue::kStamina);
		return WeaponCosts::get_cost(target_weigh + aggressor_weigh, stamina, *Settings::blockWeightMult, *Settings::blockBase, *Settings::blockStaminaMult);
	}

	float get_block_cost_Player(RE::Actor* target)
	{
		float target_weigh = WeaponsHandler::get_shield_weight(target);
		float stamina = target->GetActorValue(RE::ActorValue::kStamina);
		return WeaponCosts::get_cost(target_weigh, stamina, *Settings::blockWeightMult, *Settings::blockBase, *Settings::blockStaminaMult);
	}

	static float get_block_cost_1403BED80(RE::HitData* data)
	{
		using func_t = decltype(&get_block_cost_1403BED80);
		REL::Relocation<func_t> func{ REL::ID(25864) };
		return func(data);
	}

	float get_block_cost(RE::HitData* data)
	{
		auto aggressor = data->aggressor.get().get();
		auto target = data->target.get().get();

		bool isPlayer = aggressor->IsPlayer();
		bool new_version = isPlayer && *Settings::blockCostPlayer || !isPlayer && *Settings::blockCostNPC;

		return new_version ? get_block_cost(target, aggressor) : get_block_cost_1403BED80(data);
	}

	float get_block_cost(RE::Actor* target)
	{
		auto aggressor = target->currentCombatTarget.get().get();
		if (!aggressor)
			return 0.0f;
		return get_block_cost(target, aggressor);
	}

	bool is_strong_attack(RE::Actor* a)
	{
		return is_strong_NPC(a, get_attack_cost_newversion(a, false, false));
	}

	bool is_strong_block(RE::Actor* a)
	{
		return is_strong_NPC(a, get_block_cost(a));
	}

	bool is_strong_bash(RE::Actor* a)
	{
		return is_strong_NPC(a, get_attack_cost_newversion(a, false, true));
	}

	static float get_bow_cost(RE::Actor* a, RE::TESObjectWEAP* bow)
	{
		return WeaponCosts::get_cost(bow->GetWeight(), a->GetBaseActorValue(RE::ActorValue::kStamina),
			*Settings::rangedWeightMult, *Settings::rangedBase, *Settings::rangedStaminaMult);
	}

	bool is_strong_bow_NPC(RE::Actor* a, RE::TESObjectWEAP* bow)
	{
		return is_strong_NPC(a, get_bow_cost(a, bow));
	}

	bool is_strong_bow(RE::Actor* a, RE::TESObjectWEAP* bow)
	{
		return is_strong(a, get_bow_cost(a, bow));
	}

	static void TESObjectWEAP__Fire_140235240(RE::TESObjectWEAP* a1, RE::TESObjectREFR* source, RE::TESAmmo* overwriteAmmo, RE::EnchantmentItem* ammoEnchantment, RE::AlchemyItem* poison)
	{
		using func_t = decltype(&TESObjectWEAP__Fire_140235240);
		REL::Relocation<func_t> func{ REL::ID(17693) };
		return func(a1, source, overwriteAmmo, ammoEnchantment, poison);
	}

	void TESObjectWEAP__Fire_140235240_hooked(RE::TESObjectWEAP* weap, RE::TESObjectREFR* source, RE::TESAmmo* overwriteAmmo, RE::EnchantmentItem* ammoEnchantment, RE::AlchemyItem* poison)
	{
		TESObjectWEAP__Fire_140235240(weap, source, overwriteAmmo, ammoEnchantment, poison);
		auto a = source->As<RE::Character>();
		if (!a || !weap || weap->weaponData.animationType != RE::WEAPON_TYPE::kBow)
			return;

		bool isPlayer = a->IsPlayer();
		if (isPlayer && *Settings::rangedCostPlayer || !isPlayer && *Settings::rangedCostNPC)
			Utils::damageav(a, RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, -get_bow_cost(a, weap));
	}
}

namespace PlayerHandler
{
	static const float MOVING_COST = 40.0f;

	static float get_jump_cost(RE::Actor* a)
	{
		return WeaponCosts::get_cost(WeaponsHandler::get_curiass_weight(a), a->GetBaseActorValue(RE::ActorValue::kStamina),
			*Settings::jumpWeightMult, *Settings::jumpBase, *Settings::jumpStaminaMult);
	}

	static void Actor__Jump_1405D1F80(RE::Actor* player)
	{
		using func_t = decltype(&Actor__Jump_1405D1F80);
		REL::Relocation<func_t> func{ REL::ID(36271) };
		return func(player);
	}

	void cost_jump_Player(RE::Actor* player)
	{
		Utils::damageav(player, RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, -get_jump_cost(player));
	}

	void deny_jump_Player(RE::Actor* player)
	{
		float cost = get_jump_cost(player);
		if (cost <= player->GetActorValue(RE::ActorValue::kStamina)) {
			return Actor__Jump_1405D1F80(player);
		}
	}

	static void PlayerControls__sub_140705530(RE::PlayerControls* controls, int32_t a2, int32_t a3)
	{
		using func_t = decltype(&PlayerControls__sub_140705530);
		REL::Relocation<func_t> func{ REL::ID(41271) };
		return func(controls, a2, a3);
	}

	void PlayerControls__sub_140705530_hooked(RE::PlayerControls* controls, int32_t a2, int32_t a3, RE::PlayerCharacter* player)
	{
		float cost = CharHandler::get_block_cost_Player(player);
		if (cost <= player->GetActorValue(RE::ActorValue::kStamina)) {
			return PlayerControls__sub_140705530(controls, a2, a3);
		}
	}

	static float ActorValueOwner__get_weapon_speed(char* _a, RE::TESObjectWEAP* weap, bool left) {
		using func_t = decltype(&ActorValueOwner__get_weapon_speed);
		REL::Relocation<func_t> func{ REL::ID(25851) };
		return func(_a, weap, left);
	}

	static float get_bow_power(float time, float speed)
	{
		using func_t = decltype(&get_bow_power);
		REL::Relocation<func_t> func{ REL::ID(25868) };
		return func(time, speed);
	}

	float deny_bow_Player(char* _a, RE::TESObjectWEAP* weap, bool left, float drawn_time)
	{
		auto weapon_speed = ActorValueOwner__get_weapon_speed(_a, weap, left);
		auto power = get_bow_power(drawn_time, weapon_speed);
		
		if (!weap)
			return power;

		const float MIN_POWER = 0.35f;
		auto a = reinterpret_cast<RE::Character*>(_a - 0xB0);
		return CharHandler::is_strong_bow(a, weap) ? power : MIN_POWER;
	}
}

namespace SpeedHandler
{
	static float ActorState__sub_1410C3D40(RE::ActorState* a1, float speed)
	{
		using func_t = decltype(&ActorState__sub_1410C3D40);
		REL::Relocation<func_t> func{ REL::ID(88499) };
		return func(a1, speed);
	}

	float hooked_ActorState__sub_1410C3D40(char* a1, float speed)
	{
		auto actor = reinterpret_cast<RE::Actor*>(a1 - 0xB8);

		auto pers = actor->GetBaseActorValue(RE::ActorValue::kStamina);
		pers = pers > 0.00001f ? actor->GetActorValue(RE::ActorValue::kStamina) / pers : 1.0f;
		
		if (pers < 0.0f)
			pers = 0.0f;

		if (pers <= 0.2f)
			speed *= 0.5f;

		return ActorState__sub_1410C3D40(reinterpret_cast<RE::ActorState*>(a1), speed);
	}
}
