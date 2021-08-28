#pragma once


namespace CharHandler
{
	bool is_strong(RE::Actor* actor);
	bool is_strong_shield(RE::Actor* actor);
}

namespace PlayerHandler
{
	bool eval_condition(RE::TESIdleForm* idle, RE::Character** char_ptr);
}

namespace SpeedHandler
{
	float hooked_ActorState__sub_1410C3D40(char* a1, float speed);
}
