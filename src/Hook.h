// SkeletonHook.h
#pragma once
#include <mutex>
#include <unordered_set>

#include "RE/Skyrim.h"

namespace Hook {
    // Hook PlayerCharacter virtual functions for better performance
    class CharacterHook {
    public:
        static void Install();
        static void EnableHook();
        static void SetOStimState(bool state);
    private:

        static void UpdateAnimation(RE::Actor* a_this, float a_delta);
        static inline REL::Relocation<decltype(UpdateAnimation)> _UpdateAnimation;

        static void CharacterUpdate(RE::Actor* a_this, float a_delta);
        static inline REL::Relocation<decltype(CharacterUpdate)> _CharacterUpdate;

        static void UpdateDownwardPass(RE::NiNode* a_this, RE::NiUpdateData& a_data, std::uint32_t a_arg2);
        static inline REL::Relocation<decltype(UpdateDownwardPass)> _UpdateDownwardPass;
    };
}  // namespace Hook
