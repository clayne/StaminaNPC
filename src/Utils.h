#pragma once

#include "PCH.h"

namespace WeaponInfo
{
	const float SHIELD_COST = 30.0f;

	constexpr float weap_weight(RE::WEAPON_TYPE type)
	{
		switch (type) {
		case RE::WEAPON_TYPE::kHandToHandMelee:
			return 1.0f;
		case RE::WEAPON_TYPE::kOneHandSword:
			return 12.0f;
		case RE::WEAPON_TYPE::kOneHandDagger:
			return 3.5f;
		case RE::WEAPON_TYPE::kOneHandAxe:
			return 12.0f;
		case RE::WEAPON_TYPE::kOneHandMace:
			return 14.0f;
		case RE::WEAPON_TYPE::kTwoHandSword:
			return 18.0f;
		case RE::WEAPON_TYPE::kTwoHandAxe:
			return 23.0f;
		case RE::WEAPON_TYPE::kBow:
			return 12.0f;
		case RE::WEAPON_TYPE::kStaff:
			return 16.0f;
		case RE::WEAPON_TYPE::kCrossbow:
			return 15.0f;
		default:
			return 1.0f;
		}
	}

	constexpr float weap_cost(RE::WEAPON_TYPE type)
	{
		switch (type) {
		case RE::WEAPON_TYPE::kOneHandSword:
		case RE::WEAPON_TYPE::kOneHandDagger:
		case RE::WEAPON_TYPE::kOneHandAxe:
		case RE::WEAPON_TYPE::kOneHandMace:
			return weap_weight(type) + 20.0f;
		case RE::WEAPON_TYPE::kTwoHandSword:
		case RE::WEAPON_TYPE::kTwoHandAxe:
			return (weap_weight(type) + 20.0f) * 1.5f;
		case RE::WEAPON_TYPE::kBow:
		case RE::WEAPON_TYPE::kCrossbow:
		case RE::WEAPON_TYPE::kStaff:
		case RE::WEAPON_TYPE::kHandToHandMelee:
			return weap_weight(type) + 20.0f;
		default:
			return 0;
			break;
		}
	}

	float weap_cost(RE::TESForm* item);
}
