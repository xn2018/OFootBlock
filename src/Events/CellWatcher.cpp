#include "Events/CellWatcher.h"
#include "Utils/BoneUtils.h"

namespace Events {
    RE::BSEventNotifyControl MenuWatcher::ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                                  RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
        if (a_event) {
            static RE::BSFixedString loadingMenu("Loading Menu");

            if (a_event->menuName == loadingMenu) {
                if (a_event->opening) {
                    auto boneMgr = Utils::BoneUtils::BoneCache::GetSingleton();
                    // --- Game start Loading ---
                    boneMgr->Clear();
                    boneMgr->Reset();
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    void MenuWatcher::Register() {
        if (_registered) return;

        if (auto ui = RE::UI::GetSingleton()) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(this);
        }
    }



    RE::BSEventNotifyControl CellWatcher::ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event,
                                                       RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*) 
    {
        if (a_event && a_event->cell) {
            auto boneMgr = Utils::BoneUtils::BoneCache::GetSingleton();
            boneMgr->Clear();
            boneMgr->Reset();
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void CellWatcher::Register() {
        if (_registered) return;

        if (auto holder = RE::ScriptEventSourceHolder::GetSingleton()) {
            holder->AddEventSink<RE::TESCellFullyLoadedEvent>(this);
            _registered = true;
        }
    }
}
