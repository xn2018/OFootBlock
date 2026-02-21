#include "Hook.h"

#include <shared_mutex>

#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "REL/REL.h"
#include "RE/A/Actor.h"
#include "RE/B/BShkbAnimationGraph.h"
#include "RE/H/hkVector4.h"
#include "RE/H/hkbBehaviorGraph.h"
#include "RE/H/hkbClipGenerator.h"
#include "RE/N/NiAVObject.h"
#include "RE/N/NiNode.h"
#include "REL/Relocation.h"
#include "SKSE/Trampoline.h"
#include "Utils/BoneUtils.h"
#include "Utils/ActorUtils.h"

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace Hook {
    using NiAVObject = RE::NiAVObject;
    using NiNode = RE::NiNode;
    static std::atomic_bool g_Enable{true};
    static std::atomic_bool g_OStim{false};

    void CharacterHook::EnableHook() { 
        g_Enable.store(!g_Enable.load(std::memory_order_acquire), std::memory_order_release);
    }

    void CharacterHook::SetOStimState(bool state) { g_OStim.store(state, std::memory_order_release); }

    // Hook PlayerCharacter::UpdateAnimation - called every frame to update animations
    void CharacterHook::UpdateAnimation(RE::Actor* a_this, float a_delta) {
        // Call original UpdateAnimation first - this updates all bone animations
        _UpdateAnimation(a_this, a_delta);

        // 如果未启用，直接调用原函数
        if (g_Enable.load(std::memory_order_acquire)) {
            // 防御：空指针（理论上不会，但Hook里要谨慎）
            if (!a_this) {
                return;
            }

            // 判断是否为OStim场景角色
            auto actorManager = Utils::ActorUtils::ActorCheckUtils::GetSingleton();
            if (!actorManager->ShouldFreeze(a_this->formID)) {
                return;
            }

            // Note: We restore bones AFTER Update but update is called before UpdateAnimation
            // So we'll handle restoration in UpdateAnimation instead
            auto btfm = Utils::BoneUtils::BoneTransformManager::GetSingleton();
            if (btfm->HasCapturedBones()) {
                btfm->RestoreFootBones(a_this);
            }
        }
    }

    void CharacterHook::CharacterUpdate(RE::Actor* a_this, float a_delta) {
        // Call original UpdateAnimation first - this updates all bone animations
        _CharacterUpdate(a_this, a_delta);
        auto boneCache = Utils::BoneUtils::BoneCache::GetSingleton();
        if (boneCache->HasCached()) {
            return;
        }

        if (!a_this) {
            return;
        }

        // 判断是否为OStim场景角色
        auto actorManager = Utils::ActorUtils::ActorCheckUtils::GetSingleton();
        if (!actorManager->ShouldFreeze(a_this->formID)) {
            return;
        }

        // Note: We restore bones AFTER Update but update is called before UpdateAnimation
        // So we'll handle restoration in UpdateAnimation instead
        auto btfm = Utils::BoneUtils::BoneTransformManager::GetSingleton();
        if (btfm->HasCapturedBones()) {
            btfm->MarkFootBones(a_this);
        }
    }

    // Hook Character::Update - called every frame to update animations
    void CharacterHook::UpdateDownwardPass(NiNode* a_this, RE::NiUpdateData& a_data, std::uint32_t a_arg2) {
        // Call original UpdateAnimation first - this updates all bone animations
        _UpdateDownwardPass(a_this, a_data, a_arg2);
        // 如果未启用，直接调用原函数
        if (g_Enable.load(std::memory_order_acquire)) {
            if (!g_OStim.load(std::memory_order_acquire)) {
                return;
            }

            auto boneCache = Utils::BoneUtils::BoneCache::GetSingleton();
            if (boneCache->IsTarget(a_this)) {
                // 直接从管理器获取之前捕捉的变换数据并应用
                auto btfm = Utils::BoneUtils::BoneTransformManager::GetSingleton();
                btfm->RestoreNPCFootBones(a_this);
            }
        }
    }

    // ===== 安装 Hook =====
    void CharacterHook::Install() {
        // auto& trampoline = SKSE::GetTrampoline();
        
        // Hook Character vtable
        REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{RE::VTABLE_PlayerCharacter[0]};
        REL::Relocation<std::uintptr_t> CharacterVtbl{RE::VTABLE_Character[0]};
        REL::Relocation<std::uintptr_t> NiNodeVtbl{RE::VTABLE_NiNode[0]};

        // UpdateAnimation is at offset 0x7D (125 decimal)
        _UpdateAnimation = PlayerCharacterVtbl.write_vfunc(0x7D, UpdateAnimation);
        // UpdateAnimation is at offset 0xAD (125 decimal)
        _CharacterUpdate = CharacterVtbl.write_vfunc(0xAD, CharacterUpdate);
        // UpdateDownwardPass is at offset 0x2C (125 decimal)
        _UpdateDownwardPass = NiNodeVtbl.write_vfunc(0x2C, UpdateDownwardPass);
    }
}
