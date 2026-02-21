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
                    // --- 游戏开始 Loading ---
                    boneMgr->Clear();  // 清理你的骨骼缓存
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
            // 这里你可以添加你想要的逻辑，比如根据 cellID 或 worldspaceID 来决定是否清理骨骼缓存
            // 例如，如果你只想在特定的 cell 或 worldspace 中清理缓存：
            // if (cellID == 0x123456 || worldspaceID == 0x654321) {
            //     auto boneMgr = Utils::BoneUtils::BoneCache::GetSingleton();
            //     boneMgr->Clear();
            // }
            // 或者你也可以直接在每次加载新 cell 时清理缓存：
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
