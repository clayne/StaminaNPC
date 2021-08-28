#include "Utils.h"

namespace WeaponInfo
{
	RE::WEAPON_TYPE get_weapon_type(RE::TESForm* object)
	{
		if (!object)
			return RE::WEAPON_TYPE::kHandToHandMelee;
		return static_cast<RE::TESObjectWEAP*>(object)->weaponData.animationType.get();
	}

	float weap_cost(RE::TESForm* item)
	{
		return weap_cost(get_weapon_type(item));
	}
}
