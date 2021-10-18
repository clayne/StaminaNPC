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
			return max(0, 1.0f + (a->GetActorValue(RE::ActorValue::kMass) - 1.0f) * MASS_MULT);
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
			return max(0, (a->GetActorValue(RE::ActorValue::kMass) - 1.0f) * MASS_MULT);
	}

	static float get_curiass_weight(RE::Actor* a)
	{
		auto armo = a->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
		if (armo)
			return armo->GetWeight();
		else
			return max(0, (a->GetActorValue(RE::ActorValue::kMass) - 1.0f) * MASS_MULT);
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
	static bool is_strong(RE::Actor* actor, float cost)
	{
		auto cur_stamina = actor->GetActorValue(RE::ActorValue::kStamina);
		auto all_stamina = actor->GetBaseActorValue(RE::ActorValue::kStamina);
		if (3 * cost > all_stamina) {
			cost = all_stamina / 3;
		}

		return all_stamina <= 0.0f || cur_stamina < 0.0f || cur_stamina >= cost;
	}

	// with checking incombat
	static bool is_strong_NPC(RE::Actor* actor, float cost)
	{
		if (!actor || !actor->IsInCombat())
			return true;

		return is_strong(actor, cost);
	}

	static float getAttackTypeMult(bool isPower, bool isBash)
	{
		return static_cast<float>(isPower ? (isBash ? *Settings::attackTypeMult_powerbash : *Settings::attackTypeMult_powerattack) :
                                            (isBash ? *Settings::attackTypeMult_bash : *Settings::attackTypeMult_normattack));
	}

	float get_attack_cost(RE::Actor* a, bool isPower, bool isBash, float attack_staminaMult)
	{
		float cost, weight;
		auto stamina = a->GetBaseActorValue(RE::ActorValue::kStamina);
		if (isBash) {
			weight = WeaponsHandler::get_shield_weight(a);
		} else {
			weight = WeaponsHandler::get_weapon_weight(a);
		}
		cost = WeaponCosts::get_cost(weight, stamina, *Settings::attackWeightMult, *Settings::attackBase, *Settings::attackStaminaMult);
		return cost * getAttackTypeMult(isPower, isBash) * attack_staminaMult;
	}

	float get_attack_cost(char* _a, bool isPower, bool isBash, float attack_staminaMult)
	{
		return get_attack_cost(reinterpret_cast<RE::Actor*>(_a - 0xB0), isPower, isBash, attack_staminaMult);
	}

	float get_attack_cost(char* _a, RE::BGSAttackData* attack)
	{
		bool isPower = attack->data.flags.underlying() & 0x4;
		bool isBash = attack->data.flags.underlying() & 0x2;
		float attack_staminaMult = attack->data.staminaMult;

		return get_attack_cost(_a, isPower, isBash, attack_staminaMult);
	}

	float get_block_cost(RE::Actor* target, RE::Actor* aggressor)
	{
		float target_weigh = WeaponsHandler::get_shield_weight(target);
		float aggressor_weigh = WeaponsHandler::get_weapon_weight(aggressor);
		float stamina = target->GetActorValue(RE::ActorValue::kStamina);
		return WeaponCosts::get_cost(target_weigh + aggressor_weigh, stamina, *Settings::blockWeightMult, *Settings::blockBase, *Settings::blockStaminaMult);
	}

	float get_block_cost_single(RE::Actor* target)
	{
		float target_weigh = WeaponsHandler::get_shield_weight(target);
		float stamina = target->GetActorValue(RE::ActorValue::kStamina);
		return WeaponCosts::get_cost(target_weigh, stamina, *Settings::blockWeightMult, *Settings::blockBase, *Settings::blockStaminaMult);
	}

	float get_block_cost(RE::HitData* data)
	{
		auto aggressor = data->aggressor.get().get();
		auto target = data->target.get().get();
		return get_block_cost(target, aggressor);
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
		return is_strong_NPC(a, get_attack_cost(a, false, false, 1.0f));
	}

	bool is_strong_block(RE::Actor* a)
	{
		return is_strong_NPC(a, get_block_cost(a));
	}

	bool is_strong_bash(RE::Actor* a)
	{
		return is_strong_NPC(a, get_attack_cost(a, false, true, 1.0f));
	}

	static float get_bow_cost(RE::Actor* a, RE::TESObjectWEAP* bow)
	{
		return WeaponCosts::get_cost(bow->GetWeight(), a->GetBaseActorValue(RE::ActorValue::kStamina),
			*Settings::bowWeightMult, *Settings::bowBase, *Settings::bowStaminaMult);
	}

	bool is_strong_bow(RE::Actor* a, RE::TESObjectWEAP* bow)
	{
		return get_bow_cost(a, bow) <= a->GetActorValue(RE::ActorValue::kStamina);
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

	void Actor__Jump_1405D1F80_hooked(RE::Actor* player) {
		float cost = get_jump_cost(player);
		if (cost <= player->GetActorValue(RE::ActorValue::kStamina)) {
			Utils::damageav(player, RE::ACTOR_VALUE_MODIFIERS::kDamage, RE::ActorValue::kStamina, -cost);
			return Actor__Jump_1405D1F80(player);
		}
	}

	void Actor__Jump_1405D1F80_hooked_onlydeny(RE::Actor* player)
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
		float cost = CharHandler::get_block_cost_single(player);
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

	float get_bow_power_hooked(char* _a, RE::TESObjectWEAP* weap, bool left, float drawn_time)
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
