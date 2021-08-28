#include "CharacterHandler.h"
#include "Utils.h"
#include "Settings.h"

namespace WeaponsHandler
{
	static RE::TESObjectWEAP* get_weapon(RE::Character* player, bool left)
	{
		auto _weap = player->GetEquippedObject(left);
		if (!_weap)
			return nullptr;

		auto weap = _weap->As<RE::TESObjectWEAP>();
		return weap;
	}

	static RE::TESObjectARMO* get_shield(RE::Character* player)
	{
		auto _shield = player->GetEquippedObject(true);
		if (!_shield)
			return nullptr;

		auto shield = _shield->As<RE::TESObjectARMO>();
		if (!shield)
			return nullptr;
		return shield->GetSlotMask() == RE::BGSBipedObjectForm::BipedObjectSlot::kShield ? shield : nullptr;
	}

	static std::pair<RE::TESObjectWEAP*, bool> check_bow(RE::Character* player)
	{
		auto left_weap = get_weapon(player, true);
		if (!left_weap)
			return { nullptr, false };

		if (left_weap->GetWeaponType() == RE::WEAPON_TYPE::kBow)
			return { left_weap, true };
		else
			return { left_weap, false };
	}
}

namespace CharHandler
{
	static float get_cost_(RE::Actor* actor, bool left)
	{
		float cost;
		auto item = actor->GetEquippedObject(left);
		if (item) {
			cost = WeaponInfo::weap_cost(item);
		} else {
			cost = WeaponInfo::weap_cost(RE::WEAPON_TYPE::kHandToHandMelee);
		}
		return cost;
	}

	const static std::function<float(RE::Actor*)> left_getter = [](RE::Actor* a) {
		return get_cost_(a, true);
	};
	const static std::function<float(RE::Actor*)> right_getter = [](RE::Actor* a) {
		return get_cost_(a, false);
	};

	static bool is_strong(RE::Actor* actor, float cost)
	{
		auto cur_stamina = actor->GetActorValue(RE::ActorValue::kStamina);
		auto all_stamina = actor->GetBaseActorValue(RE::ActorValue::kStamina);
		if (3 * cost > all_stamina) {
			cost = all_stamina / 3;
		}

		return all_stamina <= 0.0f || cur_stamina < 0.0f || cur_stamina >= cost;
	}

	static bool is_strong(RE::Actor* actor, std::function<float(RE::Actor*)> _get_cost)
	{
		return is_strong(actor, _get_cost(actor));
	}

	// with checking incombat
	static bool is_strong_NPC(RE::Actor* actor, std::function<float(RE::Actor*)> _get_cost)
	{
		if (!actor || !actor->IsInCombat())
			return true;

		return is_strong(actor, _get_cost);
	}

	bool is_strong(RE::Actor* actor)
	{
		return is_strong_NPC(actor, right_getter);
	}

	bool is_strong_shield(RE::Actor* actor)
	{
		return is_strong_NPC(actor, [](RE::Actor*) { return WeaponInfo::SHIELD_COST; });
	}
}

namespace PlayerHandler
{
	static const float MOVING_COST = 40.0f;

	static bool is_meleeIDLE(RE::FormID formid) {
		return formid == 0x000BACC3 ||  // LeftHandAttack
		       formid == 0x000870D5 ||  // AttackLeftH2H
		       formid == 0x00013211 ||  // AttackRightRoot
		       formid == 0x00019B22 ||  // PowerAttackLeft
		       formid == 0x00019B23 ||  // PowerAttackBackward
		       formid == 0x00019B24 ||  // PowerAttackRight
		       formid == 0x00019B25 ||  // PowerAttackForward
		       formid == 0x00019B26 ||  // PowerAttackStanding
		       formid == 0x01000984 ||  // LeftPowerAttack
		       formid == 0x0100098F ||  // NonMountedCombatLeftPower
		       formid == 0x000E3F3A ||  // LeftHandPowerAttackBack
		       formid == 0x000E3F3D ||  // H2HRightHandPowerAttack
		       formid == 0x0002E900 ||  // LeftHandPowerAttack
		       formid == 0x00050CA5 ||  // DualAttackRoot
		       formid == 0x0002E2F8 ||  // DualPowerAttackRoot
		       formid == 0x0009753B;    // BowLeftAttack
	}

	static bool is_strong(RE::TESIdleForm* idle, RE::Character* player)
	{
		auto formid = idle->formID;

		if (formid == 0x088302 && *Settings::ablejumpPlayer || formid == 0x1050F3 && *Settings::ablerunPlayer)  // JumpRoot || SprintStart
			return CharHandler::is_strong(player, MOVING_COST);

		if ((formid == 0x013217 || formid == 0x0BACC3 && WeaponsHandler::get_shield(player)) && *Settings::blockbashPlayer)  // BlockingStart || LeftHandAttack
			return CharHandler::is_strong(player, WeaponInfo::SHIELD_COST);

		if (is_meleeIDLE(formid)) {
			auto isBow = WeaponsHandler::check_bow(player);
			if (isBow.second) {
				return !(formid == 0x013211 && *Settings::rangedPlayer || formid == 0x09753B && *Settings::meleePlayer) ||
					CharHandler::is_strong(player, WeaponInfo::weap_cost(isBow.first));
			}

			if (!*Settings::meleePlayer)
				return true;

			return CharHandler::is_strong(player, WeaponInfo::weap_cost(WeaponsHandler::get_weapon(player, false)));
		}

		return true;
	}

	static bool _eval_condition(RE::TESCondition* cond, RE::Character** player_ptr)
	{
		using func_t = decltype(&_eval_condition);
		REL::Relocation<func_t> func{ REL::ID(29078) };
		return func(cond, player_ptr);
	}

	// called from 14035817D
	bool eval_condition(RE::TESIdleForm* idle, RE::Character** player_ptr)
	{
		auto orig_ans = _eval_condition(&idle->conditions, player_ptr);
		auto player = *player_ptr;
		if (player->formID != 0x14)
			return orig_ans;

		return orig_ans && is_strong(idle, player);
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
			speed *= 0.6f - 0.5f * pers;  // 0..0.2 -> 0.6..0.5

		return ActorState__sub_1410C3D40(reinterpret_cast<RE::ActorState*>(a1), speed);
	}
}
