#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>

#include <shared_mutex>
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
        bool HasCapturedBones() const { 
            return m_hasCapturedBones; 
        }
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
        const std::vector<std::string> m_footBoneNames = {
            "NPC L Foot [Lft ]",
            "NPC R Foot [Rft ]",
            "NPC L Toe0 [LToe]",
            "NPC R Toe0 [RToe]"
        };
    };



    class BoneCache {
    public:
        // 获取单例
        static BoneCache* GetSingleton(){
            static BoneCache singleton;
            return &singleton;
        }

        // 禁用拷贝和赋值 (单例模式规范)
        BoneCache(const BoneCache&) = delete;
        void operator=(const BoneCache&) = delete;

        // 默认构造函数
        BoneCache() = default;

        // 登记骨骼
        void RegisterBone(RE::NiAVObject* a_node);

        // 判断是否为目标骨骼 (极速查询)
        bool IsTarget(RE::NiAVObject* a_node);

        // 定期清理
        void Clear();

        // 重置缓存并重新开启捕获闸门
        void Reset();

        bool HasCached();
        // 完成捕获
        void SetReady();
    };
}