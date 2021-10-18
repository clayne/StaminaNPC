#include "Utils.h"

namespace Utils
{
	void damageav(RE::Actor* a, RE::ACTOR_VALUE_MODIFIERS::ACTOR_VALUE_MODIFIER i1, RE::ActorValue i2, float val)
	{
		if (val >= 0.0f)
			return;
		using func_t = decltype(&damageav);
		REL::Relocation<func_t> func{ REL::ID(37523) };
		return func(a, i1, i2, val);
	}

	RE::PlayerCharacter* getPlayer()
	{
		using func_t = decltype(&getPlayer);
		REL::Relocation<func_t> func{ REL::ID(54836) };
		return func();
	}
}
