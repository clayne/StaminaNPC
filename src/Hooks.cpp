#include "Hooks.h"
#include "Settings.h"
#include "CharacterHandler.h"
#include <UselessFenixUtils.h>

namespace Hooks
{
	void apply_get_CombatRangedAttackChance(std::uintptr_t is_strong)
	{
		const int ID = 49772, BRANCH_TYPE = 5;
		constexpr REL::ID funcOffset = REL::ID(ID);
		auto funcAddr = funcOffset.address();

		struct Code : Xbyak::CodeGenerator
		{
			Code(uintptr_t is_strong_addr, uintptr_t ret_addr)
			{
				Xbyak::Label still_strong;

				push(rcx);
				push(rdx);
				sub(rsp, 40);
				mov(rax, is_strong_addr);
				call(rax);
				add(rsp, 40);
				pop(rdx);
				pop(rcx);

				test(al, al);
				jnz(still_strong);
				xorps(xmm0, xmm0);
				ret();

				L(still_strong);
				push(rbx);       // recover
				sub(rsp, 0x40);  // recover
				mov(rax, ret_addr);
				jmp(rax);
			}
		} xbyakCode{ is_strong, funcAddr + BRANCH_TYPE };
		add_trampoline<BRANCH_TYPE, ID>(&xbyakCode);
	}

	void apply_get_CombatBlockChance(std::uintptr_t is_strong)
	{
		const int ID = 49751, BRANCH_TYPE = 6;
		constexpr REL::ID funcOffset = REL::ID(ID);
		auto funcAddr = funcOffset.address();

		struct Code : Xbyak::CodeGenerator
		{
			Code(uintptr_t is_strong_addr, uintptr_t ret_addr)
			{
				Xbyak::Label still_strong;

				push(rcx);
				push(rdx);
				sub(rsp, 40);
				mov(rax, is_strong_addr);
				call(rax);
				add(rsp, 40);
				pop(rdx);
				pop(rcx);

				test(al, al);
				jnz(still_strong);
				xorps(xmm0, xmm0);
				ret();

				L(still_strong);
				push(rbx);       // recover
				sub(rsp, 0x30);  // recover
				mov(rax, ret_addr);
				jmp(rax);
			}
		} xbyakCode{ is_strong, funcAddr + BRANCH_TYPE };
		add_trampoline<BRANCH_TYPE, ID>(&xbyakCode);
	}

	void apply_get_CombatBlockAttackChance(std::uintptr_t is_strong)
	{
		const int ID = 49750, BRANCH_TYPE = 5;
		constexpr REL::ID funcOffset(ID);
		auto funcAddr = funcOffset.address();
		constexpr REL::ID subOffset(49711);
		auto subAddr = subOffset.address();

		struct Code : Xbyak::CodeGenerator
		{
			Code(uintptr_t is_strong_addr, uintptr_t ret_addr, uintptr_t sub_addr)
			{
				Xbyak::Label still_strong;

				push(rcx);
				push(rdx);
				sub(rsp, 48);
				mov(rax, is_strong_addr);
				call(rax);
				add(rsp, 48);
				pop(rdx);
				pop(rcx);

				test(al, al);
				jnz(still_strong);
				xorps(xmm0, xmm0);
				add(rsp, 0x28);  // recover
				ret();

				L(still_strong);
				mov(rax, ret_addr);
				push(rax);
				mov(rax, sub_addr);
				jmp(rax);
			}
		} xbyakCode{ is_strong, funcAddr + 0x4 + BRANCH_TYPE, subAddr };
		add_trampoline<BRANCH_TYPE, ID, 0x4>(&xbyakCode);
	}

	float player_bow_hooked(char* _a, RE::TESObjectWEAP* weap, bool left, float drawn_time)
	{
		auto weapon_speed =
			_generic_foo_<25851, float(char* _a, RE::TESObjectWEAP* weap, bool left)>::eval(_a, weap, left);
		auto power = _generic_foo_<25868, float(float time, float speed)>::eval(drawn_time, weapon_speed);

		if (!weap)
			return power;

		const float MIN_POWER = 0.35f;
		auto a = reinterpret_cast<RE::Character*>(_a - 0xB0);
		return Denying::Player::is_strong_Player_bow(a) ? power : MIN_POWER;
	}

	void apply_deny_bow_Player()
	{
		// SkyrimSE.exe+74B769(+0x5F9) -- SkyrimSE.exe+74B779(+0x609)
		const int ID = 42928, BRANCH_TYPE = 5;
		constexpr REL::ID funcOffset = REL::ID(ID);
		auto retAddr = std::uintptr_t(funcOffset.address() + 0x609);

		struct Code : Xbyak::CodeGenerator
		{
			Code(uintptr_t jump_addr, uintptr_t retaddr)
			{
				movaps(xmm3, xmm6);
				mov(rax, jump_addr);
				call(rax);
				mov(rax, retaddr);
				jmp(rax);
			}
		} xbyakCode{ std::uintptr_t(player_bow_hooked), retAddr };
		add_trampoline<BRANCH_TYPE, ID, 0x5F9>(&xbyakCode);
	}

	void apply_meleeBash_cost()
	{
		Costs::MeleeBash::OnAttackHook::Hook();

		// SkyrimSE.exe+627A59
		const int ID = 37650;
		constexpr REL::ID funcOffset = REL::ID(ID);
		auto retAddr = std::uintptr_t(funcOffset.address() + 0x132);
		auto retaddr_nulldata = std::uintptr_t(funcOffset.address() + 0x206);

		struct Code : Xbyak::CodeGenerator
		{
			Code(uintptr_t func_nulldata, uintptr_t retaddr, uintptr_t retaddr_nulldata)
			{
				Xbyak::Label nulldata;

				cmp(qword[r14], r12);
				je(nulldata);
				mov(rax, retaddr);
				jmp(rax);

				L(nulldata);
				sub(rsp, 0x20);
				mov(rcx, rbx);
				mov(rax, func_nulldata);
				call(rax);
				add(rsp, 0x20);
				mov(rax, retaddr_nulldata);
				jmp(rax);
			}
		} xbyakCode{ std::uintptr_t(Costs::MeleeBash::on_attack_nulldata), retAddr, retaddr_nulldata };
		add_trampoline<6, ID, 0x129>(&xbyakCode);
	}

	void apply_hooks()
	{
		[[maybe_unused]] auto& trampoline = SKSE::GetTrampoline();

		// NPC, deny melee & bash
		GetThisAttackChanceHook::Hook();

		// NPC, deny bow
		apply_get_CombatRangedAttackChance(std::uintptr_t(Denying::NPC::is_strong_NPC_bow));

		// NPC, deny block
		apply_get_CombatBlockChance(std::uintptr_t(Denying::NPC::is_strong_NPC_block));
		apply_get_CombatBlockAttackChance(std::uintptr_t(Denying::NPC::is_strong_NPC_block));

		// NPC, get slower
		ActorState__get_speed_kHook::Hook();

		// Player, deny bow
		apply_deny_bow_Player();

		// Player, deny block
		PlayerBlockButtonHook::Hook();

		// Player, deny jump
		PlayerJumpHook::Hook();

		// Player, deny melee & bash
		PlayerDenyAttackBashHook::Hook();

		// Player, scale damage
		GetDamageHook::Hook();

		// Cost bow
		FireKeepBowHook::Hook();

		// Cost block
		OnBlockedHook::Hook();

		// Cost jump
		OnJumpHook::Hook();

		// Cost melee && bash
		apply_meleeBash_cost();

		// Regen rate & delay
		StaminaRegenHook::Hook();

		// Swimming
		SwimmingHook::Hook();
	}

	float GetDamageHook::get_damage(void* _weap, RE::ActorValueOwner* a, float DamageMult, char isbow)
	{
		auto ans = _get_damage(_weap, a, DamageMult, isbow);

		if (isbow || !a->GetIsPlayerOwner())
			return ans;

		return Denying::Player::get_scaled_damage(RE::PlayerCharacter::GetSingleton(), ans);
	}

	float GetThisAttackChanceHook::get_thisattack_chance(RE::Actor* me, RE::Actor* he, RE::BGSAttackData* my_attackdata)
	{
		return Denying::NPC::is_strong_NPC_melee_bash(me, my_attackdata) ?
		           _get_thisattack_chance(me, he, my_attackdata) :
                   0.0f;
	}

	float ActorState__get_speed_kHook::ActorState__get_speed_k(RE::ActorState* a, float speed)
	{
		auto actor = reinterpret_cast<RE::Actor*>(reinterpret_cast<char*>(a) - 0xB8);
		return _ActorState__get_speed_k(a, Denying::NPC::get_speed_NPC(actor, speed));
	}

	uint32_t PlayerBlockButtonHook::sub_140705530(RE::PlayerControls* controls, uint32_t a2, uint32_t er8_0)
	{
		return Denying::Player::is_strong_Player_block() ? _sub_140705530(controls, a2, er8_0) : 0;
	}

	void PlayerJumpHook::Jump(RE::Actor* a)
	{
		if (Denying::Player::is_strong_Player_jump(a))
			return _Jump(a);
	}

	float PlayerDenyAttackBashHook::get_attack_cost(RE::ActorValueOwner* _a, RE::BGSAttackData* attack)
	{
		auto a = reinterpret_cast<RE::Actor*>(reinterpret_cast<char*>(_a) - 0xB0);
		return Denying::Player::is_strong_Player_melee_bash(a, attack) ? 0.0f : 1.0f;
	}

	void FireKeepBowHook::Fire(RE::TESObjectWEAP* weap, RE::TESObjectREFR* source, RE::TESAmmo* overwriteAmmo,
		RE::EnchantmentItem* ammoEnchantment, RE::AlchemyItem* poison)
	{
		_Fire(weap, source, overwriteAmmo, ammoEnchantment, poison);
		auto a = source->As<RE::Character>();
		if (!a || !weap || weap->weaponData.animationType != RE::WEAPON_TYPE::kBow)
			return;

		if (Settings::Costs::bowshot.is_enabled())
			FenixUtils::damagestamina_delay_blink(a, Costs::Bow::get_bow_cost(a));
	}

	void FireKeepBowHook::RestoreActorValue(RE::Actor* a, RE::ActorValue av, float val)
	{
		if (a->actorState1.meleeAttackState != RE::ATTACK_STATE_ENUM::kBowDrawn ||
			!Settings::Costs::bowshot.is_enabled()) {
			_RestoreActorValue(a, av, val);
		}
	}

	bool FireKeepBowHook::update_RegenDelay(RE::Actor* a, RE::ActorValue av, float passed_time)
	{
		if (a->actorState1.meleeAttackState == RE::ATTACK_STATE_ENUM::kBowDrawn &&
			Settings::Costs::bowkeep.is_enabled()) {
			FenixUtils::damageav(a, RE::ActorValue::kStamina, passed_time * Costs::Bow::get_bow_cost_keep(a));
		}
		return _update_RegenDelay(a, av, passed_time);
	}

	float GetBlockCostHook::get_block_cost(RE::HitData* hitdata)
	{
		return Costs::Block::get_block_cost(hitdata, _get_block_cost(hitdata));
	}

	float OnJumpHook::GetScale(RE::Actor* a)
	{
		if (Settings::Costs::jump.is_enabled())
			Costs::Jump::on_jump(a);

		return _GetScale(a);
	}

	float OnBlockedHook::get_block_cost(RE::HitData* hit)
	{
		return Costs::Block::get_block_cost(hit, _get_block_cost(hit));
	}

	void StaminaRegenHook::RestoreActorValue(RE::Actor* a, RE::ActorValue stamina, float val)
	{
		float mul = Regen::calculate_regen_mult(a);
		if (mul > 0.0f)
			_RestoreActorValue(a, stamina, val * mul);
	}
	void StaminaRegenHook::Custom_StaminaRegenDelay(RE::AIProcess* proc, RE::ActorValue ind, float origin,
		float modifier_delta_neg)
	{
		_set_RegenDelay(proc, ind,
			ind == RE::ActorValue::kStamina ? Regen::get_regen_delay(modifier_delta_neg, origin) : origin);
	}

	void SwimmingHook::RestoreActorValue(RE::Actor* a, RE::ActorValue av, float val)
	{
		if (!a->actorState1.swimming || !Settings::Costs::swimming.is_enabled()) {
			_RestoreActorValue(a, av, val);
		}
	}

	bool SwimmingHook::update_RegenDelay(RE::Actor* a, RE::ActorValue av, float passed_time)
	{
		if (a->actorState1.swimming && Settings::Costs::swimming.is_enabled()) {
			FenixUtils::damageav(a, RE::ActorValue::kStamina, passed_time * Costs::Swimming::get_swimming_cost(a));
		}
		return _update_RegenDelay(a, av, passed_time);
	}
}
