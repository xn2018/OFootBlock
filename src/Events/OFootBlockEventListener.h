#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace Events {
    class OFootBlockEventListener final : public RE::BSTEventSink<SKSE::ModCallbackEvent> {
    public:
        static OFootBlockEventListener* GetSingleton();

        void Register();
        void Unregister();

        virtual RE::BSEventNotifyControl ProcessEvent(
            const SKSE::ModCallbackEvent* a_event, RE::BSTEventSource<SKSE::ModCallbackEvent>* a_eventSource) override;
        static void ParseScenePayload(const std::string& payload, bool threadStart);
    private:
        OFootBlockEventListener() = default;
    };
}  // namespace OFootBlock
