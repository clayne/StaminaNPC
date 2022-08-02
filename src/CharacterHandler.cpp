#include "CharacterHandler.h"
#include "Utils.h"
#include "Settings.h"
#include "UselessFenixUtils.h"

namespace Costs
{
	namespace Equipement
	{
		static const float MASS_MULT = 11.0f;

		inline constexpr REL::ID UnarmedWeap(static_cast<std::uint64_t>(514923));

		RE::TESObjectWEAP* get_UnarmedWeap()
		{
			REL::Relocation<RE::NiPointer<RE::TESObjectWEAP>*> singleton{ UnarmedWeap };
			return singleton->get();
		}

		float get_curiass_weight(RE::Actor* a)
		{
			auto armo = a->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
			if (armo) {
				return armo->GetWeight();
			}

			auto ans = a->GetActorValue(RE::ActorValue::kMass);
			if (!FenixUtils::is_human(a))
				ans = ans * MASS_MULT;

			return ans;
		}

		RE::TESObjectARMO* get_shield(RE::Actor* a)
		{
			if (auto _shield = a->GetEquippedObject(true)) {
				if (auto shield = _shield->As<RE::TESObjectARMO>()) {
					return shield->GetSlotMask() == RE::BGSBipedObjectForm::BipedObjectSlot::kShield ? shield : nullptr;
				}
			}

			return nullptr;
		}

		RE::TESObjectWEAP* get_weapon(RE::Actor* a, bool left)
		{
			if (auto _weap = a->GetEquippedObject(left)) {
				if (auto weap = _weap->As<RE::TESObjectWEAP>(); weap && weap->IsWeapon()) {
					return weap;
				}
			}

			return get_UnarmedWeap();
		}

		RE::TESObjectWEAP* get_bow(RE::Actor* a)
		{
			auto weap = get_weapon(a, false);
			return weap->GetWeaponType() == RE::WEAPON_TYPE::kBow ? weap : nullptr;
		}

		float get_bow_weight(RE::Actor* a)
		{
			auto bow = get_bow(a);
			return bow ? bow->GetWeight() : 0.0f;
		}

		float get_blocking_thing_weight(RE::Actor* a)
		{
			if (auto shield = get_shield(a)) {
				return shield->GetWeight();
			} else {
				return get_weapon(a, false)->GetWeight();
			}
		}

		float get_attacking_thing_weight(RE::Actor* a, bool left, bool bash) {
			return bash ? Equipement::get_blocking_thing_weight(a) : Equipement::get_weapon(a, left)->GetWeight();
		}
	}

	static constexpr float DEFAULT_REQUIREMENT = 10.0f;

	namespace Skill
	{
		float get_skill_k(float skill) { return Settings::Costs::skillK.get(skill); }

		float get_skill_k(RE::Actor* a, RE::ActorValue av)
		{
			return Settings::Costs::skillK.is_enabled() ? Settings::Costs::skillK.get(a->GetActorValue(av) * 0.01f) :
                                                          1.0f;
		}

		float skill_mod(RE::Actor* a, RE::ActorValue av, float val)
		{
			return a->IsPlayerRef() && Settings::Costs::skillK.is_enabled() ? get_skill_k(a, av) * val : val;
		}
	}
	using Skill::skill_mod;

	namespace Level
	{
		float get_level_k(int lvl) { return Settings::Costs::Level::get(lvl); }

		float get_level_k(RE::Actor* a)
		{
			return Settings::Costs::Level::is_enabled() ? get_level_k(a->GetLevel()) : 1.0f;
		}

		float lvl_mod(RE::Actor* a, float val)
		{
			return a->IsPlayerRef() && Settings::Costs::Level::is_enabled() ? get_level_k(a) * val : val;
		}
	}
	using Level::lvl_mod;

	namespace MeleeBash
	{
		namespace Impl
		{
			void ApplyPerkEntryPoint_14032ECE0(int id, RE::Actor* a, void* a3, float* k)
			{
				_generic_foo_<23073, decltype(ApplyPerkEntryPoint_14032ECE0)>::eval(id, a, a3, k);
			}

			void ApplyPerkEntryPoint(RE::Actor* a, float* k)
			{
				auto weap = Equipement::get_weapon(a, false);
				ApplyPerkEntryPoint_14032ECE0(RE::BGSPerkEntry::EntryPoint::kModPowerAttackStamina, a, weap, k);
			}
		}

		float get_cost_meleeBash_base_(RE::Actor* a, bool bash, bool power, bool left) {
			float weight = Equipement::get_attacking_thing_weight(a, left, bash);
			float ans = bash ? Settings::Costs::bash.get(weight) : Settings::Costs::melee.get(weight);

			if (power) {
				ans *= Settings::Costs::get_powerActionMult(bash);
			}

			if (power || Settings::Costs::normalAttackPerks)
				Impl::ApplyPerkEntryPoint(a, &ans);

			return ans;
		}

		float get_cost_meleeBash_base(RE::Actor* a, RE::BGSAttackData* adata)
		{
			bool bash = adata && adata->data.flags.any(RE::AttackData::AttackFlag::kBashAttack);
			bool power = adata && adata->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack);

			float ans = get_cost_meleeBash_base_(a, bash, power, adata && adata->IsLeftAttack());

			// 1.0  almost always
			// 0.5  for power dual (I need count it)
			// 0.01 for sphere (norm)
			// 3.0  for one werewolf attack (ok then)
			if (adata)
				ans *= adata->data.staminaMult;

			return ans;
		}

		RE::ActorValue get_action_skill_(RE::Actor* a, bool left, bool bash)
		{
			auto weap = Equipement::get_weapon(a, left);

			if (bash) {
				return weap->GetWeaponType() == RE::WEAPON_TYPE::kBow ? RE::ActorValue::kTwoHanded :
                                                                        RE::ActorValue::kBlock;
			} else {
				return weap->IsTwoHandedAxe() || weap->IsTwoHandedSword() ? RE::ActorValue::kTwoHanded :
                                                                            RE::ActorValue::kOneHanded;
			}
		}

		RE::ActorValue get_action_skill(RE::Actor* a, RE::BGSAttackData* adata)
		{
			return get_action_skill_(a, adata && adata->IsLeftAttack(),
				adata && adata->data.flags.any(RE::AttackData::AttackFlag::kBashAttack));
		}

		float get_attack_cost_new(RE::Actor* a, RE::BGSAttackData* adata)
		{
			return lvl_mod(a, skill_mod(a, get_action_skill(a, adata), get_cost_meleeBash_base(a, adata)));
		}

		float get_attack_cost(RE::Actor* a, RE::BGSAttackData* attack)
		{
			bool bash = attack->data.flags.any(RE::AttackData::AttackFlag::kBashAttack);

			bool new_version =
				bash && Settings::Costs::bash.is_enabled() || !bash && Settings::Costs::melee.is_enabled();

			return new_version ? get_attack_cost_new(a, attack) :
                                 OnAttackHook::_get_attack_cost((RE::ActorValueOwner*)((char*)a + 0xB0), attack);
		}

		void on_attack_nulldata(RE::Actor* a)
		{
			if (Settings::Costs::melee.is_enabled())
				FenixUtils::damagestamina_delay_blink(a, get_attack_cost_new(a, nullptr));
		}

		void OnAttackHook::Hook()
		{
			_get_attack_cost = SKSE::GetTrampoline().write_call<5>(REL::ID(37650).address() + 0x16e,
				get_attack_cost);  // SkyrimSE.exe+627A9E
		}

		float OnAttackHook::get_attack_cost(RE::ActorValueOwner* _a, RE::BGSAttackData* attack)
		{
			auto a = reinterpret_cast<RE::Actor*>(reinterpret_cast<char*>(_a) - 0xB0);
			if (attack->data.flags.any(RE::AttackData::AttackFlag::kBashAttack)) {
				return Settings::Costs::bash.is_enabled() ? Costs::MeleeBash::get_attack_cost(a, attack) :
                                                            _get_attack_cost(_a, attack);
			} else {
				return Settings::Costs::melee.is_enabled() ? Costs::MeleeBash::get_attack_cost(a, attack) :
                                                             _get_attack_cost(_a, attack);
			}
		}
	}

	namespace Bow
	{
		float get_bow_cost_base(RE::Actor* a)
		{
			auto bow = Equipement::get_bow(a);
			return bow ? Settings::Costs::bowshot.get(bow->GetWeight()) : 0.0f;
		}

		float get_bow_cost(RE::Actor* a)
		{
			return Settings::Costs::bowshot.is_enabled() ?
			           lvl_mod(a, skill_mod(a, RE::ActorValue::kArchery, get_bow_cost_base(a))) :
                       DEFAULT_REQUIREMENT;
		}

		float get_bow_cost_keep_base(RE::Actor* a) {
			auto bow = Equipement::get_bow(a);
			return bow ? Settings::Costs::bowkeep.get(bow->GetWeight()) : 0.0f;
		}

		float get_bow_cost_keep(RE::Actor* a)
		{
			return lvl_mod(a, skill_mod(a, RE::ActorValue::kArchery, get_bow_cost_keep_base(a)));
		}
	}

	namespace Block
	{
		float get_block_cost_base(RE::Actor* target, float aggressor_weight)
		{
			float target_weight = Equipement::get_blocking_thing_weight(target);
			return Settings::Costs::block.get(target_weight + aggressor_weight);
		}

		float get_block_cost_base(RE::Actor* target, RE::Actor* aggressor)
		{
			float aggressor_weight = aggressor ? Equipement::get_attacking_thing_weight(aggressor, false, false) : 0.0f;
			return get_block_cost_base(target, aggressor_weight);
		}

		float get_block_cost(RE::Actor* target, RE::Actor* aggressor)
		{
			return lvl_mod(target, skill_mod(target, RE::ActorValue::kBlock, get_block_cost_base(target, aggressor)));
		}

		float get_block_cost_NPC(RE::Actor* target)
		{
			auto aggressor = target->currentCombatTarget.get().get();
			if (!aggressor)
				return 0.0f;
			return Settings::Costs::block.is_enabled() ? get_block_cost(target, aggressor) : DEFAULT_REQUIREMENT;
		}

		float get_block_cost(RE::HitData* data, float origin)
		{
			auto aggressor = data->aggressor.get().get();
			auto target = data->target.get().get();

			return Settings::Costs::block.is_enabled() ? get_block_cost(target, aggressor) : origin;
		}
	}

	namespace Jump
	{
		float get_jump_cost_base(RE::Actor* a)
		{
			return Settings::Costs::jump.get(Equipement::get_curiass_weight(a));
		}

		float get_jump_cost(RE::Actor* a) { return lvl_mod(a, get_jump_cost_base(a)); }

		void on_jump(RE::Actor* a)
		{
			float cost = get_jump_cost(a);
			if (cost <= 0.0f)
				return;

			FenixUtils::damagestamina_delay_blink(a, cost);
		}
	}

	namespace Swimming
	{
		float get_swimming_cost_base(RE::Actor* a)
		{
			return Settings::Costs::swimming.get(Equipement::get_curiass_weight(a));
		}

		float get_swimming_cost(RE::Actor* a) { return lvl_mod(a, get_swimming_cost_base(a)); }
	}
}

namespace Denying
{
	bool is_strong(RE::Actor* actor, float cost)
	{
		if (!actor->IsPlayerRef() && actor->GetActorValue(RE::ActorValue::kStaminaRateMult) *
											 actor->GetActorValue(RE::ActorValue::KStaminaRate) <
										 0.00001f)
			return true;

		auto cur_stamina = actor->GetActorValue(RE::ActorValue::kStamina);
		auto all_stamina = FenixUtils::get_total_av(actor, RE::ActorValue::kStamina);
		if (3 * cost > all_stamina) {
			cost = all_stamina / 3;
		}

		return all_stamina <= 0.0f || cur_stamina < 0.0f || cur_stamina >= cost;
	}

	namespace NPC
	{
		bool is_strong_NPC(RE::Actor* actor, float cost)
		{
			if (!actor || !actor->IsInCombat())
				return true;

			return is_strong(actor, cost);
		}

		// exported

		bool is_strong_NPC_melee_bash(RE::Actor* a, RE::BGSAttackData* adata)
		{
			bool bash = adata->data.flags.any(RE::AttackData::AttackFlag::kBashAttack);

			return (!bash || !Settings::DenyNPC::bash || is_strong_NPC(a, Costs::MeleeBash::get_attack_cost(a, adata))) &&
			       (bash || !Settings::DenyNPC::melee || is_strong_NPC(a, Costs::MeleeBash::get_attack_cost(a, adata)));
		}

		bool is_strong_NPC_bow(RE::Actor* a)
		{
			return !Settings::DenyNPC::bow || is_strong_NPC(a, Costs::Bow::get_bow_cost(a));
		}

		bool is_strong_NPC_block(RE::Actor* a)
		{
			return !Settings::DenyNPC::block || is_strong_NPC(a, Costs::Block::get_block_cost_NPC(a));
		}

		float get_speed_NPC(RE::Actor* a, float origin)
		{
			if (Settings::DenyNPC::speed) {
				auto pers = FenixUtils::get_total_av(a, RE::ActorValue::kStamina);
				pers = pers > 0.00001f ? a->GetActorValue(RE::ActorValue::kStamina) / pers : 1.0f;

				if (pers < 0.0f)
					pers = 0.0f;

				// TODO: border, k in settings
				if (pers <= 0.2f)
					origin *= 0.5f;
			}

			return origin;
		}
	}

	namespace Player
	{
		bool is_strong_blink(bool ans) {
			if (!ans)
				FenixUtils::FlashHudMenuMeter__blink(RE::ActorValue::kStamina);
			return ans;
		}

		// exported

		bool is_strong_Player_bow(RE::Actor* a)
		{
			return is_strong_blink(!Settings::DenyPlayer::bow || is_strong(a, Costs::Bow::get_bow_cost(a)));
		}

		bool is_strong_Player_block()
		{
			auto a = RE::PlayerCharacter::GetSingleton();
			return is_strong_blink(!Settings::DenyPlayer::block || is_strong(a, Costs::Block::get_block_cost(a, nullptr)));
		}

		bool is_strong_Player_jump(RE::Actor* a) {
			return is_strong_blink(!Settings::DenyPlayer::jump || is_strong(a, Costs::Jump::get_jump_cost(a)));
		}

		bool is_strong_Player_melee_bash(RE::Actor* a, RE::BGSAttackData* adata)
		{
			bool bash = adata->data.flags.any(RE::AttackData::AttackFlag::kBashAttack);
			if (bash) {
				return !Settings::DenyPlayer::bash || is_strong(a, Costs::MeleeBash::get_attack_cost(a, adata));
			}

			Settings::DenyModes mode = Settings::DenyPlayer::get_melee_mode(adata);

			switch (mode) {
			case Settings::DenyModes::Deny:
				return is_strong(a, Costs::MeleeBash::get_attack_cost(a, adata));
			case Settings::DenyModes::Scale:
				is_strong_blink(is_strong(a, Costs::MeleeBash::get_attack_cost(a, adata)));
			case Settings::DenyModes::Allow:
			default:
				return true;
			}
		}

		float get_scaled_damage(RE::Actor* a, float origin)
		{
			if (auto adata = FenixUtils::get_attackData(a).get()) {
				if (Settings::DenyPlayer::get_melee_mode(adata) == Settings::DenyModes::Scale) {
					float cost = Costs::MeleeBash::get_attack_cost(a, adata);
				
					float cur = a->GetActorValue(RE::ActorValue::kStamina);
					if (cost <= cur || cost <= 0.00001f)
						return origin;
				
					return origin * cur / cost;
				}
			}
			return origin;
		}
	}
}

namespace Regen
{
	float get_walktype_k(RE::Actor* a) { return Settings::Regen::walkK.get(a); }

	float get_w(RE::Actor* a) { return a->equippedWeight; }

	float get_cwp(RE::Actor* a) {
		float cwp = 1.0f;
		float denum = _generic_foo_<36456, float(RE::Actor * a)>::eval(a);  // GetTotalCarryWeight_5E1860
		if (denum > 0.00001f)
			cwp = a->GetActorValue(RE::ActorValue::kInventoryWeight) / denum;
		return cwp;
	}

	float get_carry_k(RE::Actor* a)
	{
		return Settings::Regen::carryK.get(get_w(a), a->IsPlayerRef() ? get_cwp(a) : 0.5f);
	}

	float get_HP_k(RE::Actor* a)
	{
		return Settings::Regen::HP_SP::get_HP(FenixUtils::getAV_pers(a, RE::ActorValue::kHealth));
	}

	float get_SP_k(RE::Actor* a)
	{
		return Settings::Regen::HP_SP::get_SP(FenixUtils::getAV_pers(a, RE::ActorValue::kStamina));
	}

	float calculate_regen_mult(RE::Actor* a) {
		float ans = 1.0f, mult;

		mult = get_walktype_k(a);
		if (mult <= 0.0f)
			return 0.0f;
		else
			ans *= mult;

		mult = get_carry_k(a);
		if (mult <= 0.0f)
			return 0.0f;
		else
			ans *= mult;

		mult = get_HP_k(a);
		if (mult <= 0.0f)
			return 0.0f;
		else
			ans *= mult;

		mult = get_SP_k(a);
		if (mult <= 0.0f)
			return 0.0f;
		else
			ans *= mult;

		return ans;
	}

	float get_regen_delay(float modifier_delta_neg, float origin)
	{
		return Settings::Regen::delay.is_enabled() ? Settings::Regen::delay.get(-modifier_delta_neg) : origin;
	}
}
