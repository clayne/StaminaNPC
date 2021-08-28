#pragma once

#include "PCH.h"
#include "SKSE/Trampoline.h"
#include <xbyak\xbyak.h>

namespace Hooks
{
	void apply_hooks(std::uintptr_t is_strong, std::uintptr_t is_strong_shield);
}
