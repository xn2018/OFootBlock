#pragma once

#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "Hook.h"  // 你的 Hooks::SetFreezeActor 声明
#include "Events/InputChordDetector.h"
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "Utils/BoneUtils.h"  // 你需要实现的骨骼工具

namespace Events {
    class InputEventSink : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static InputEventSink& GetSingleton() noexcept;
        void Register();
        void SetToggleKeys(std::uint32_t key1, std::uint32_t key2) noexcept;

        virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns,
                                                      RE::BSTEventSource<RE::InputEvent*>* src) override;
       
    private:
        InputEventSink() = default;
        InputChordDetector _chord;
        
        std::uint32_t _key1{0};
        std::uint32_t _key2{0};

        bool _key1Down{false};
        bool _key2Down{false};

        std::vector<std::string> _frozenBones;
    };
}  // namespace Input
