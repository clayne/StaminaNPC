#pragma once

#include "PCH.h"
#include "SKSE/Trampoline.h"
#include <xbyak\xbyak.h>
#include "UselessFenixUtils.h"

namespace Hooks
{
	class GetDamageHook
	{
	public:
		static void Hook()
		{
			_get_damage = SKSE::GetTrampoline().write_call<5>(REL::ID(42832).address() + 0x1a5, get_damage);  // SkyrimSE.exe+7429F5
		}

	private:
		static float get_damage(void* _weap, RE::ActorValueOwner* a, float DamageMult, char isbow);
		static inline REL::Relocation<decltype(get_damage)> _get_damage;
	};

	class GetThisAttackChanceHook
	{
	public:
		static void Hook()
		{
			_get_thisattack_chance = SKSE::GetTrampoline().write_call<5>(REL::ID(48139).address() + 0x2ae,
				get_thisattack_chance);  // SkyrimSE.exe+80C2CE
		}

	private:
		static float get_thisattack_chance(RE::Actor* me, RE::Actor* he, RE::BGSAttackData* my_attackdata);
		static inline REL::Relocation<decltype(get_thisattack_chance)> _get_thisattack_chance;
	};

	class ActorState__get_speed_kHook
	{
	public:
		static void Hook()
		{
			_ActorState__get_speed_k = SKSE::GetTrampoline().write_call<5>(REL::ID(49311).address() + 0x2e1,
				ActorState__get_speed_k);  // SkyrimSE.exe+83CB51
		}

	private:
		static float ActorState__get_speed_k(RE::ActorState* a, float speed);
		static inline REL::Relocation<decltype(ActorState__get_speed_k)> _ActorState__get_speed_k;
	};

	class PlayerBlockButtonHook
	{
	public:
		static void Hook()
		{
			_sub_140705530 = SKSE::GetTrampoline().write_call<5>(REL::ID(41361).address() + 0xcb,
				sub_140705530);  // SkyrimSE.exe+70977B
		}

	private:
		static uint32_t sub_140705530(RE::PlayerControls* controls, uint32_t a2, uint32_t er8_0);
		static inline REL::Relocation<decltype(sub_140705530)> _sub_140705530;
	};

	class PlayerJumpHook
	{
	public:
		static void Hook()
		{
			_Jump = SKSE::GetTrampoline().write_branch<5>(REL::ID(41349).address() + 0x114,
				Jump);  // SkyrimSE.exe+7091B4
		}

	private:
		static void Jump(RE::Actor* a);
		static inline REL::Relocation<decltype(Jump)> _Jump;
	};

	class PlayerDenyAttackBashHook
	{
	public:
		static void Hook()
		{
			_get_attack_cost = SKSE::GetTrampoline().write_call<5>(REL::ID(38047).address() + 0xbb,
				get_attack_cost);  // SkyrimSE.exe+63D06B

			FenixUtils::writebytes<38047, 0xc8>("\xeb\x19\x0F\x1F\x44\x00\x00"sv);  // skip a.stamina > 0
		}

	private:
		static float get_attack_cost(RE::ActorValueOwner* _a, RE::BGSAttackData* attack);
		static inline REL::Relocation<decltype(get_attack_cost)> _get_attack_cost;
	};

	class FireKeepBowHook
	{
	public:
		static void Hook()
		{
			_Fire = SKSE::GetTrampoline().write_call<5>(REL::ID(41778).address() + 0x133, Fire);  // SkyrimSE.exe+7221E3
			_RestoreActorValue = SKSE::GetTrampoline().write_call<5>(REL::ID(37510).address() + 0x176,
				RestoreActorValue);  // SkyrimSE.exe+620806
			_update_RegenDelay = SKSE::GetTrampoline().write_call<5>(REL::ID(37510).address() + 0x1b,
				update_RegenDelay);  // SkyrimSE.exe+6206AB
		}

	private:
		static void Fire(RE::TESObjectWEAP* weap, RE::TESObjectREFR* source, RE::TESAmmo* overwriteAmmo,
			RE::EnchantmentItem* ammoEnchantment, RE::AlchemyItem* poison);
		static void RestoreActorValue(RE::Actor* a, RE::ActorValue av, float val);
		static bool update_RegenDelay(RE::Actor* a, RE::ActorValue av, float passed_time);
		static inline REL::Relocation<decltype(Fire)> _Fire;
		static inline REL::Relocation<decltype(RestoreActorValue)> _RestoreActorValue;
		static inline REL::Relocation<decltype(update_RegenDelay)> _update_RegenDelay;
	};

	class GetBlockCostHook
	{
	public:
		static void Hook()
		{
			_get_block_cost = SKSE::GetTrampoline().write_call<5>(REL::ID(37633).address() + 0x8d4,
				get_block_cost);  // SkyrimSE.exe+626CD4
		}

	private:
		static float get_block_cost(RE::HitData* hitdata);
		static inline REL::Relocation<decltype(get_block_cost)> _get_block_cost;
	};

	class OnJumpHook
	{
	public:
		static void Hook()
		{
			_GetScale = SKSE::GetTrampoline().write_call<5>(REL::ID(36271).address() + 0x190,
				GetScale);  // SkyrimSE.exe+5D2110
		}

	private:
		static float GetScale(RE::Actor* a);
		static inline REL::Relocation<decltype(GetScale)> _GetScale;
	};

	class OnBlockedHook
	{
	public:
		static void Hook()
		{
			_get_block_cost = SKSE::GetTrampoline().write_call<5>(REL::ID(37633).address() + 0x8d4,
				get_block_cost);  // SkyrimSE.exe+626CD4
		}

	private:
		static float get_block_cost(RE::HitData* hit);
		static inline REL::Relocation<decltype(get_block_cost)> _get_block_cost;
	};

	class StaminaRegenHook
	{
	public:
		static void Hook()
		{
			_RestoreActorValue = SKSE::GetTrampoline().write_call<5>(REL::ID(37510).address() + 0x176,
				RestoreActorValue);  // SkyrimSE.exe+620806

			// SkyrimSE.exe+621490
			const int ID = 37525;
			constexpr REL::ID funcOffset = REL::ID(ID);
			auto retAddr = std::uintptr_t(funcOffset.address() + 0xC0 + 5);
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t func, uintptr_t retaddr)
				{
					movss(xmm3, xmm8);

					//sub(rsp, 0x20);  // mb useless
					mov(rax, func);
					call(rax);
					//add(rsp, 0x20);  // mb useless
					mov(rax, retaddr);
					jmp(rax);
				}
			} xbyakCode{ uintptr_t(Custom_StaminaRegenDelay), retAddr };
			_set_RegenDelay = add_trampoline<5, ID, 0xC0>(&xbyakCode);
		}

	private:
		static void RestoreActorValue(RE::Actor* a, RE::ActorValue av, float val);
		static inline REL::Relocation<decltype(RestoreActorValue)> _RestoreActorValue;
		static inline REL::Relocation<void(RE::AIProcess*, RE::ActorValue, float time)> _set_RegenDelay;
		static void Custom_StaminaRegenDelay(RE::AIProcess* proc, RE::ActorValue ind, float origin,
			float modifier_delta_neg);
	};

	class SwimmingHook
	{
	public:
		static void Hook()
		{
			_RestoreActorValue = SKSE::GetTrampoline().write_call<5>(REL::ID(37510).address() + 0x176,
				RestoreActorValue);  // SkyrimSE.exe+620806
			_update_RegenDelay = SKSE::GetTrampoline().write_call<5>(REL::ID(37510).address() + 0x1b,
				update_RegenDelay);  // SkyrimSE.exe+6206AB
		}

	private:
		static void RestoreActorValue(RE::Actor* a, RE::ActorValue av, float val);
		static bool update_RegenDelay(RE::Actor* a, RE::ActorValue av, float passed_time);
		static inline REL::Relocation<decltype(RestoreActorValue)> _RestoreActorValue;
		static inline REL::Relocation<decltype(update_RegenDelay)> _update_RegenDelay;
	};

	void apply_hooks();
}
