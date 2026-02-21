#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace Events {
    class MenuWatcher:public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
        void Register();
        static MenuWatcher* GetSingleton() {
            static MenuWatcher singleton;
            return &singleton;
        };

    private:
        MenuWatcher() = default;
        bool _registered{false};
    };

    class CellWatcher : public RE::BSTEventSink<RE::TESCellFullyLoadedEvent> {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCellFullyLoadedEvent* a_event,
                                                      RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*) override;
        void Register();
        static CellWatcher* GetSingleton() {
            static CellWatcher singleton;
            return &singleton;
        };

    private:
        CellWatcher() = default;
        bool _registered{false};
    };
}