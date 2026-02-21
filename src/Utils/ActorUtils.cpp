#include "RE/N/NiTransform.h"
#include "Utils/ActorUtils.h"
#include "Utils/BoneUtils.h"

using namespace SKSE;
using namespace SKSE::log;

namespace Utils::ActorUtils {
    bool ActorCheckUtils::IsEquippedHeels(RE::Actor* actor) {
        bool hasHeels = false;
        auto& weightModel = actor->GetBiped(0);
        if (weightModel) {
            for (int i = 0; i < 42; ++i) {
                auto& data = weightModel->objects[i];
                if (data.partClone) {
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

        // 必须是护甲（Armor）
        auto armor = form->As<RE::TESObjectARMO>();
        if (!armor) {
            return false;
        }

        // 判断是否占用脚部槽位（鞋子）
        return armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kFeet);
    }


    void ActorCheckUtils::RegisterOStimActors(RE::Actor* actor) {
        if (!actor) {
            return;
        }

        auto actorbase = actor->GetActorBase();
        if (!actorbase->IsFemale()) {
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
        // operator[] 会：
        // - 如果不存在 → 自动创建默认 ActorData
        // - 如果存在 → 返回引用

        data.hasHeels = equiped;

        logger::info("SetHeelsForActor: {} -> hasHeels = {}", actor->GetName(), equiped ? "true" : "false");
    }

    bool ActorCheckUtils::ShouldFreeze(RE::FormID id) {
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

    void ActorCheckUtils::SetActorVisiabled(RE::FormID formId, bool _frozen) {
        auto form = RE::TESForm::LookupByID(formId);

        if (!form) {
            logger::warn("SetActorVisiabled: Form not found {:X}", formId);
            return;
        }

        auto actor = form->As<RE::Actor>();
        if (!actor) {
            logger::warn("SetActorVisiabled: Form is not Actor {:X}", formId);
            return;
        }

        // 确保3D已加载
        if (!actor->Is3DLoaded()) {
            logger::warn("SetActorVisiabled: Actor 3D not loaded {:X}", formId);
            return;
        }

        if (!_frozen) {
            // 隐藏 Actor（完全透明）
            actor->SetAlpha(0.0f);
        } else {
            actor->SetAlpha(1.0f);
        }

        logger::info("Actor hidden {:X}", formId);
    }

    RE::Actor* ActorCheckUtils::GetActorFromNode(RE::NiAVObject* a_node) {
        if (!a_node) return nullptr;

        // 向上遍历层级，但限制层数防止死循环或过度消耗
        int limit = 0;
        RE::NiAVObject* current = a_node;

        while (current && limit < 20) {
            // 关键防御 1：检查指针地址是否看起来合法（简单过滤空指针和极小地址）
            if (reinterpret_cast<uintptr_t>(current) < 0x10000) break;

            // 关键防御 2：检查 userData
            auto userData = current->GetUserData();
            if (userData) {
                // 关键防御 3：利用 RTTI 或 FormType 校验
                // 不要直接转 Actor，先转 TESObjectREFR 并检查类型
                auto ref = static_cast<RE::TESObjectREFR*>(userData);

                // 校验地址合法性以及是否真的是 Actor
                // 注意：必须先通过内存保护检查或 RTTI 确认
                try {
                    if (ref && ref->formType == RE::FormType::ActorCharacter) {
                        return static_cast<RE::Actor*>(ref);
                    }
                } catch (...) {
                    // 捕获可能的内存访问异常
                    break;
                }
            }
            current = current->parent;
            limit++;
        }
        return nullptr;
    }
}