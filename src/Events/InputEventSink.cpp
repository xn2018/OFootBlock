#include <string>


#include "Hook.h"
#include "Utils/BoneUtils.h"
#include "InputEventSink.h"
#include "Utils/ActorUtils.h"

namespace Events {
    static std::mutex _lock;

    InputEventSink& InputEventSink::GetSingleton() noexcept {
        static InputEventSink inst;
        return inst;
    }

    void InputEventSink::Register() {
        auto dev = RE::BSInputDeviceManager::GetSingleton();
        if (dev) {
            dev->AddEventSink(&InputEventSink::GetSingleton());
            logger::info("InputEventSink installed.");
        }
    }

    void InputEventSink::SetToggleKeys(std::uint32_t key1, std::uint32_t key2) noexcept {
        std::lock_guard<std::mutex> lock(_lock);
        _key1 = key1;
        _key2 = key2;
        _chord.SetKeys(key1, key2);
    }

    RE::BSEventNotifyControl InputEventSink::ProcessEvent(RE::InputEvent* const* evns,
                                                          RE::BSTEventSource<RE::InputEvent*>*) {
        if (!evns || !*evns) return RE::BSEventNotifyControl::kContinue;

        auto ev = *evns;

        if (ev->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) return RE::BSEventNotifyControl::kContinue;

        auto* btn = ev->AsButtonEvent();
        auto isPaused = RE::UI::GetSingleton()->GameIsPaused();

        if (isPaused) return RE::BSEventNotifyControl::kContinue;

        if (!btn) return RE::BSEventNotifyControl::kContinue;

        bool doToggle = false;
        {
            std::lock_guard<std::mutex> lock(_lock);
            if (_chord.OnButtonEvent(btn)) {
                doToggle = true;
            }
        }

        if (doToggle) {
            Hook::CharacterHook::EnableHook();
        }
        
        return RE::BSEventNotifyControl::kContinue;
    }


}  // namespace Input