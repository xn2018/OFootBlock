#pragma once

#include <RE/Skyrim.h>

namespace OFootBlock {
    bool RegisterHitCounter(RE::BSScript::IVirtualMachine* vm);

    void InitializeHook(SKSE::Trampoline& trampoline);
}