#pragma once

namespace Costs
{
	namespace Equipement
	{
		float get_curiass_weight(RE::Actor* a);
		float get_blocking_thing_weight(RE::Actor* a);
		float get_bow_weight(RE::Actor* a);
		float get_attacking_thing_weight(RE::Actor* a, bool left, bool bash);
	}

	namespace Level
	{
		float get_level_k(RE::Actor* a);
	}

	namespace Skill
	{
		float get_skill_k(RE::Actor* a, RE::ActorValue av);
	}

	namespace MeleeBash
	{
		class OnAttackHook
		{
		public:
			static void Hook();
			static float get_attack_cost(RE::ActorValueOwner* _a, RE::BGSAttackData* attack);
			static inline REL::Relocation<decltype(get_attack_cost)> _get_attack_cost;
		};

		float get_attack_cost(RE::Actor* a, RE::BGSAttackData* adata);
		void on_attack_nulldata(RE::Actor* a);
		float get_cost_meleeBash_base_(RE::Actor* a, bool bash, bool power, bool left);
		RE::ActorValue get_action_skill_(RE::Actor* a, bool left, bool bash);
	}

	namespace Block
	{
		float get_block_cost(RE::HitData* data, float origin);
		float get_block_cost_base(RE::Actor* target, float aggressor_weight);
	}

	namespace Jump
	{
		float get_jump_cost(RE::Actor* a);
		float get_jump_cost_base(RE::Actor* a);
		void on_jump(RE::Actor* a);
	}

	namespace Bow
	{
		float get_bow_cost_base(RE::Actor* a);
		float get_bow_cost(RE::Actor* a);
		float get_bow_cost_keep_base(RE::Actor* a);
		float get_bow_cost_keep(RE::Actor* a);
	}

	namespace Swimming
	{
		float get_swimming_cost_base(RE::Actor* a);
		float get_swimming_cost(RE::Actor* a);
	}
}

namespace Denying
{
	namespace NPC
	{
		bool is_strong_NPC_melee_bash(RE::Actor* a, RE::BGSAttackData* adata);
		bool is_strong_NPC_bow(RE::Actor* a);
		bool is_strong_NPC_block(RE::Actor* a);
		float get_speed_NPC(RE::Actor* a, float origin);
	}

	namespace Player
	{
		bool is_strong_Player_bow(RE::Actor* a);
		bool is_strong_Player_block();
		bool is_strong_Player_jump(RE::Actor* a);
		bool is_strong_Player_melee_bash(RE::Actor* a, RE::BGSAttackData* adata);
		float get_scaled_damage(RE::Actor* a, float origin);
	}
}

namespace Regen
{
	float calculate_regen_mult(RE::Actor* a);
	float get_w(RE::Actor* a);
	float get_cwp(RE::Actor* a);
	float get_carry_k(RE::Actor* a);
	float get_HP_k(RE::Actor* a);
	float get_SP_k(RE::Actor* a);
	float get_regen_delay(float modifier_delta_neg, float origin);
}
