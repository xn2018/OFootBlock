#include "Utils/BoneUtils.h"

#include "RE/N/NiTransform.h"

using namespace SKSE;
using namespace SKSE::log;

namespace Utils::BoneUtils {
    // ------------------------------------------------------------
    // Capture initial bone transformations (position, rotation, scaling) for later use.
    // ------------------------------------------------------------
    void BoneTransformManager::CaptureFootBones(RE::Actor* actor) {
        if (!actor) {
            log::warn("[OFootBlock] Cannot capture foot bones - playeris null");
            return;
        }

        auto root = actor->Get3D();
        if (!root) {
            log::warn("[OFootBlock] Cannot capture foot bones - 3D model is null Actor:{}", actor->GetName());
            return;
        }

        m_footBoneTransforms.clear();
        
        for (const auto& boneName : m_footBoneNames) {
            auto bone = root->GetObjectByName(boneName);
            if (bone) {
                BoneTransform transform;
                // transform.position = bone->local.translate;
                transform.rotation = bone->local.rotate;
                // transform.scale = bone->local.scale;
                m_footBoneTransforms[boneName] = transform;
            } else {
                log::info("[OFootBlock] Could not find bone: {}", boneName);
            }
        }

        m_hasCapturedBones = !m_footBoneTransforms.empty();
        log::info("[OFootBlock] Captured {} foot bones", m_footBoneTransforms.size());

        if (!m_hasCapturedBones) {
            log::error("[OFootBlock] Failed to capture any foot bones! Check if bone names match your skeleton.");
        }
    }

    // ------------------------------------------------------------
    // Freeze Core: Use previously captured bone transformations (position, rotation, scaling)
    // ------------------------------------------------------------
    void BoneTransformManager::RestoreFootBones(RE::Actor* actor) {
        if (!actor->Get3D()) {
            return;
        }

        if (m_footBoneTransforms.empty()) {
            return;
        }

        auto* root = actor->Get3D();

        for (const auto& [boneName, transform] : m_footBoneTransforms) {
            auto* bone = root->GetObjectByName(boneName);
            if (bone) {
                // Restore the captured transform
                // bone->local.translate = transform.position;
                bone->local.rotate = transform.rotation;
            } else {
                logger::info("Can not found bone: {}", boneName);
            }
        }
    }

    void BoneTransformManager::MarkFootBones(RE::Actor* actor) {
        if (!actor->Get3D()) {
            return;
        }

        if (m_footBoneTransforms.empty()) {
            return;
        }

        auto* root = actor->Get3D();
        auto boneCache = BoneCache::GetSingleton();
        for (const auto& [boneName, transform] : m_footBoneTransforms) {
            auto* bone = root->GetObjectByName(boneName);
            if (bone) {
                // Restore the captured transform
                // bone->local.translate = transform.position;
                boneCache->RegisterBone(bone);
            } else {
                logger::info("Can not found bone: {}", boneName);
            }
        }

        boneCache->SetReady();
    }


    void BoneTransformManager::RestoreNPCFootBones(RE::NiAVObject* a_Node) {
        for (const auto& [boneName, transform] : m_footBoneTransforms) {
            if (boneName == a_Node->name.c_str()) {
                // Restore the captured transform
                // bone->local.translate = transform.position;
                a_Node->local.rotate = transform.rotation;
            } else {
                logger::info("Can not found bone: {}", boneName);
            }
        }
    }

    void BoneTransformManager::Clear() {
        m_footBoneTransforms.clear();
        m_hasCapturedBones = false;
    }

    static std::unordered_set<uintptr_t> g_TargetBones;
    static std::shared_mutex g_Mutex;
    static std::atomic<bool> g_isCached{false};

    void BoneCache::RegisterBone(RE::NiAVObject* a_node) {
        if (!a_node) return;

        std::unique_lock lock(g_Mutex);
        g_TargetBones.insert(reinterpret_cast<uintptr_t>(a_node));
    }

    bool BoneCache::IsTarget(RE::NiAVObject* a_node) {
        // If the switch is not turned on, return directly.
        if (!g_isCached.load(std::memory_order_relaxed)) {
            return false;
        }

        std::shared_lock lock(g_Mutex);
        return g_TargetBones.contains(reinterpret_cast<uintptr_t>(a_node));
    }

    void BoneCache::Clear() {
        std::unique_lock lock(g_Mutex);
        g_TargetBones.clear();
    }

    void BoneCache::Reset() {
        std::unique_lock lock(g_Mutex);
        g_TargetBones.clear();
        g_isCached.store(false);
    }

    bool BoneCache::HasCached() {
        return g_isCached.load(std::memory_order_relaxed); 
    }

    void BoneCache::SetReady() { g_isCached.store(true); }
}  // namespace Utils::BoneUtils
