#include "Utils/ActorUtils.h"

#include "RE/N/NiTransform.h"
#include "Utils/BoneUtils.h"

using namespace SKSE;
using namespace SKSE::log;

namespace Utils::ActorUtils {
    bool ActorCheckUtils::IsEquippedHeels(RE::Actor* actor) {
        if (!actor) {
            return false;
        }

        bool hasHeels = false;
        auto& weightModel = actor->GetBiped(0);
        if (weightModel) {
            for (int i = 0; i < 42; ++i) {
                auto& data = weightModel->objects[i];
                if (data.partClone && data.item && data.addon) {
                    RE::TESForm* bipedArmor = data.item;

                    // only check slot 37, as too many objects cause weird crashed here and heel offsets should only
                    // be on the shoes anyways
                    if (bipedArmor->formType == RE::TESObjectARMO::FORMTYPE &&
                        data.addon->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kFeet)) {
                        hasHeels = true;
                        break;
                    }
                }
            }
        }

        return hasHeels;
    }

    bool ActorCheckUtils::IsHeels(RE::FormID formid) {
        // FormID → TESForm
        auto form = RE::TESForm::LookupByID(formid);
        if (!form) {
            return false;
        }

        // Must be armor (TESObjectARMO)
        auto armor = form->As<RE::TESObjectARMO>();
        if (!armor) {
            return false;
        }

        // Check whether it occupies the feet slot (shoes)
        return armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kFeet);
    }

    void ActorCheckUtils::RegisterOStimActors(RE::Actor* actor) {
        if (!actor) {
            return;
        }

        auto actorbase = actor->GetActorBase();
        if (!actorbase || !actorbase->IsFemale()) {
            return;
        }

        const RE::FormID formID = actor->GetFormID();

        auto& data = _actorFrozenMap[formID];

        data.isOStimActor = true;
        data.hasHeels = IsEquippedHeels(actor);
    }

    bool ActorCheckUtils::IsOStimActors(RE::Actor* actor) {
        if (!actor) return false;

        const RE::FormID formID = actor->GetFormID();

        auto& data = _actorFrozenMap[formID];

        return data.isOStimActor;
    }

    void ActorCheckUtils::SetHeelsForActor(RE::Actor* actor, bool equiped) {
        if (!actor) {
            return;
        }

        const RE::FormID formID = actor->GetFormID();

        auto& data = _actorFrozenMap[formID];
        // operator[] will:
        // - If not present -> automatically create a default ActorData
        // - If present -> return a reference

        data.hasHeels = equiped;
    }

    bool ActorCheckUtils::ShouldFreeze(RE::FormID id) {
        std::shared_lock lk(_actor_mutex);
        auto it = _actorFrozenMap.find(id);
        if (it == _actorFrozenMap.end()) {
            return false;
        }

        const auto& data = it->second;
        return data.hasHeels && data.isOStimActor;
    }

    void ActorCheckUtils::UnregisterOStimActor(RE::Actor* actor) {
        if (!actor) return;

        const RE::FormID id = actor->GetFormID();

        std::unique_lock lk(_actor_mutex);

        auto it = _actorFrozenMap.find(id);
        if (it != _actorFrozenMap.end()) {
            it->second.isOStimActor = false;
        }
    }

    std::unordered_map<RE::FormID, Utils::ActorUtils::ActorData> ActorCheckUtils::GetOStimActorsFormID() const {
        std::shared_lock lk(_actor_mutex);
        return _actorFrozenMap;
    }

    RE::Actor* ActorCheckUtils::GetActorFromNode(RE::NiAVObject* a_node) {
        if (!a_node) return nullptr;

        // Traverse up the hierarchy, but limit the number of steps to avoid infinite loops or excessive work
        int limit = 0;
        RE::NiAVObject* current = a_node;

        while (current && limit < 20) {
            // Defensive check 1: verify pointer address looks valid (simple filter for null/very small addresses)
            if (reinterpret_cast<uintptr_t>(current) < 0x10000) break;

            // Defensive check 2: check userData
            auto userData = current->GetUserData();
            if (userData) {
                auto ref = skyrim_cast<RE::TESObjectREFR*>(userData);
                if (ref && ref->formType == RE::FormType::ActorCharacter) {
                    return skyrim_cast<RE::Actor*>(ref);
                }
            }

            current = current->parent;
            limit++;
        }

        return nullptr;
    }
}  // namespace Utils::ActorUtils