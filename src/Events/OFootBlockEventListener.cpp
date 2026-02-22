#include "OFootBlockEventListener.h"
#include "Hook.h"
#include "Utils/ActorUtils.h"
#include "Utils/BoneUtils.h"

#include <string_view>

namespace Events {
    OFootBlockEventListener* OFootBlockEventListener::GetSingleton() {
        static OFootBlockEventListener instance;
        return std::addressof(instance);
    }

    void OFootBlockEventListener::Register() {
        auto* modEventSource = SKSE::GetModCallbackEventSource();
        if (modEventSource) {
            modEventSource->AddEventSink(this);
            logger::info("OFootBlock ModEvent listener registered");
        }
    }

    void OFootBlockEventListener::Unregister() {
        auto* modEventSource = SKSE::GetModCallbackEventSource();
        if (modEventSource) {
            modEventSource->RemoveEventSink(this);
            logger::info("OFootBlock ModEvent listener unregistered");
        }
    }

    RE::BSEventNotifyControl OFootBlockEventListener::ProcessEvent(
        const SKSE::ModCallbackEvent* a_event, RE::BSTEventSource<SKSE::ModCallbackEvent>* /*a_eventSource*/) {
        if (!a_event || a_event->eventName.empty()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const std::string_view eventName{a_event->eventName};
        const std::string& str = a_event->strArg.c_str();

        if (eventName == "ofb_thread_start") {
            ParseScenePayload(str, true);
            logger::info("[OFootBlock] OFB_ThreadStart Event recevied: {}", str);
        } else if (eventName == "ofb_thread_end") {
            ParseScenePayload(str, false);
            logger::info("[OFootBlock] OFB_ThreadEnd Event recevied: {}", str);
        }

        

        return RE::BSEventNotifyControl::kContinue;
    }

    void OFootBlockEventListener::ParseScenePayload(const std::string& payload, bool threadStart) {
        logger::info("[OFootBlock] payload: {}", payload);

        constexpr std::string_view key = "Actors=";
        auto pos = payload.find(key);
        if (pos == std::string::npos) {
            logger::warn("[OFootBlock] Actors field not found");
            return;
        }

        pos += key.size();

        // End after finding the Actors field.
        auto end = payload.find('|', pos);
        std::string_view actorsView = (end == std::string::npos) ? std::string_view(payload).substr(pos)
                                                                 : std::string_view(payload).substr(pos, end - pos);

        if (actorsView.empty()) {
            logger::warn("[OFootBlock] Actors field empty");
            return;
        }

        auto btfm = Utils::BoneUtils::BoneTransformManager::GetSingleton();
        auto acu = Utils::ActorUtils::ActorCheckUtils::GetSingleton();

        size_t start = 0;

        while (start < actorsView.size()) {
            size_t comma = actorsView.find(',', start);

            std::string_view idView =
                (comma == std::string::npos) ? actorsView.substr(start) : actorsView.substr(start, comma - start);

            // Remove "N:" / "P:"
            if (idView.size() > 2 && idView[1] == ':') {
                idView.remove_prefix(2);
            }

            if (!idView.empty()) {
                uint32_t formID = 0;

                auto result = std::from_chars(idView.data(), idView.data() + idView.size(), formID, 10);

                if (result.ec == std::errc()) {
                    auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);

                    if (actor) {
                        if (threadStart) {
                            // ⭐ Scene Start
                            btfm->CaptureFootBones(actor);
                            acu->RegisterOStimActors(actor);
                            Hook::CharacterHook::SetOStimState(threadStart);
                            logger::info("[OFootBlock] Scene start register actor {}", actor->GetName());
                        } else {
                            // ⭐ Scene End
                            btfm->RestoreFootBones(actor);
                            acu->UnregisterOStimActor(actor);
                            Hook::CharacterHook::SetOStimState(threadStart);
                            logger::info("[OFootBlock] Scene end unregister actor {}", actor->GetName());
                        }
                    } else {
                        logger::warn("[OFootBlock] Actor not found for FormID {}", formID);
                    }

                } else {
                    logger::error("[OFootBlock] Failed to parse actor id: {}", idView);
                }
            }

            if (comma == std::string::npos) break;

            start = comma + 1;
        }

        // ⭐ If the thread has ended, you can choose to clear the cache.
        if (!threadStart) {
            btfm->Clear();
        }
    }



}  // namespace Events
