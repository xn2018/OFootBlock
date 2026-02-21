#pragma once

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "RE/A/Actor.h"
#include "RE/N/NiNode.h"

namespace Utils::ActorUtils {
    struct BoneTransform {
        RE::NiPoint3 position;
        RE::NiMatrix3 rotation;
        float scale;
    };

    struct ActorData {
        bool hasHeels{false};
        bool isOStimActor{false};
        bool sceneActive{false};  
        std::unordered_map<std::string, BoneTransform> m_footBoneTransforms;
    };

    class ActorCheckUtils {
    public:
        static ActorCheckUtils* GetSingleton() {
            static ActorCheckUtils singleton;
            return &singleton;
        }

        bool IsEquippedHeels(RE::Actor* actor);
        bool IsOStimActors(RE::Actor* actor);
        bool IsHeels(RE::FormID formid);
        bool ShouldFreeze(RE::FormID id);
        std::unordered_map<RE::FormID, Utils::ActorUtils::ActorData> GetOStimActorsFormID() const;
        void SetHeelsForActor(RE::Actor* actor, bool equiped);
        void UnregisterOStimActor(RE::Actor* actor);
        void RegisterOStimActors(RE::Actor* actor);
        void SetActorVisiabled(RE::FormID formId, bool doToggle);
        RE::Actor* GetActorFromNode(RE::NiAVObject* a_node);
    private:
        mutable std::shared_mutex _actor_mutex;
        std::unordered_map<RE::FormID, ActorData> _actorFrozenMap;
    };
    
}  // namespace Utils