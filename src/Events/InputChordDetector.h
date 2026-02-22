#pragma once

#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace Events {

    struct InputKeyState {
        bool isDown{false};
        bool consumed{false};  // Prevent repeat
        std::chrono::steady_clock::time_point downTime{};
    };

    class InputChordDetector {
    public:
        std::uint32_t key1{0};
        std::uint32_t key2{0};

        InputKeyState state1;
        InputKeyState state2;
        void SetKeys(std::uint32_t k1, std::uint32_t k2);
        bool OnButtonEvent(RE::ButtonEvent* btn);
        void ResetOnKeyUp(std::uint32_t id);

    private:
        void UpdateKeyState(std::uint32_t id, RE::ButtonEvent* btn, std::chrono::steady_clock::time_point now);
    };
}  // namespace Input