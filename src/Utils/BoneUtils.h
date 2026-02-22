#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_set>

#include "RE/A/Actor.h"
#include "RE/N/NiNode.h"

namespace Utils::BoneUtils {
    // Store original bone transforms for foot bones
    class BoneTransformManager {
    public:
        static BoneTransformManager* GetSingleton() {
            static BoneTransformManager singleton;
            return &singleton;
        }

        void CaptureFootBones(RE::Actor* a_player);
        void RestoreFootBones(RE::Actor* a_player);
        void RestoreNPCFootBones(RE::NiAVObject* a_Node);
        bool HasCapturedBones() const { return m_hasCapturedBones; }
        void MarkFootBones(RE::Actor* a_player);
        std::vector<std::string> GetFootBoneNames() const { return m_footBoneNames; }
        void Clear();

    private:
        BoneTransformManager() = default;

        struct BoneTransform {
            RE::NiPoint3 position;
            RE::NiMatrix3 rotation;
            float scale;
        };

        std::unordered_map<std::string, BoneTransform> m_footBoneTransforms;
        bool m_hasCapturedBones = false;

        // Foot bone names (commonly used in Skyrim skeletons)
        const std::vector<std::string> m_footBoneNames = {"NPC L Foot [Lft ]", "NPC R Foot [Rft ]", "NPC L Toe0 [LToe]",
                                                          "NPC R Toe0 [RToe]"};
    };

    class BoneCache {
    public:
        // Get singleton
        static BoneCache* GetSingleton() {
            static BoneCache singleton;
            return &singleton;
        }

        // Disable copy and assignment (singleton pattern)
        BoneCache(const BoneCache&) = delete;
        void operator=(const BoneCache&) = delete;

        // Default constructor
        BoneCache() = default;

        // Register bone
        void RegisterBone(RE::NiAVObject* a_node);

        // Check whether node is target bone (fast lookup)
        bool IsTarget(RE::NiAVObject* a_node);

        // Clear cache
        void Clear();

        // Reset cache and re-enable capture
        void Reset();

        bool HasCached();
        // Mark capture as complete / set ready
        void SetReady();
    };
}  // namespace Utils::BoneUtils