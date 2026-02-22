#include "EquipEventSink.h"
#include "RE/A/Actor.h"
#include "RE/T/TESDataHandler.h"
#include "RE/T/TESObjectARMO.h"

#include "Utils/ActorUtils.h"
#include "Hook.h"

using namespace SKSE;
using namespace SKSE::log;

namespace Events {
    
    EquipListener* EquipListener::GetSingleton() {
        static EquipListener singleton;
        return std::addressof(singleton);
    }

    void EquipListener::Register() {
        if (_registered) return;

        auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
        if (holder) {
            holder->AddEventSink<RE::TESEquipEvent>(this);
            _registered = true;
            logger::info("OFootBlock TESEquipEvent listener registered");
        }
    }

    RE::BSEventNotifyControl EquipListener::ProcessEvent(const RE::TESEquipEvent* ev,
                                                         RE::BSTEventSource<RE::TESEquipEvent>*) {
        if (!ev || !ev->actor) return RE::BSEventNotifyControl::kContinue;

        auto* actor = ev->actor->As<RE::Actor>();
        if (!actor) return RE::BSEventNotifyControl::kContinue;

        auto acu = Utils::ActorUtils::ActorCheckUtils::GetSingleton();

        // Only handle OStim scene characters
        if (!acu->IsOStimActors(actor)) return RE::BSEventNotifyControl::kContinue;

        // -------------------------------------
        // 🎯 Important: Determine if the item in the event is a high heel.
        // -------------------------------------
        auto obj = ev->baseObject;
        if (!obj) return RE::BSEventNotifyControl::kContinue;

        if (!acu->IsHeels(obj))  // ← 新函数：判断物品是否为高跟鞋
            return RE::BSEventNotifyControl::kContinue;

        // -------------------------------------
        // It is now safe to make a judgment. equip / unequip
        // -------------------------------------
        acu->SetHeelsForActor(actor, ev->equipped);
        logger::info("Actor {} unequipped heels", actor->GetName());

        return RE::BSEventNotifyControl::kContinue;
    }

}

