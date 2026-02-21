#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Events {
    class EquipListener : public RE::BSTEventSink<RE::TESEquipEvent> {
    public:
        static EquipListener* GetSingleton();

        void Register();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event,
                                              RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) override;

    private:
        EquipListener() = default;
        bool _registered{false};
        mutable std::shared_mutex _mtx;
    };
}  // namespace Events