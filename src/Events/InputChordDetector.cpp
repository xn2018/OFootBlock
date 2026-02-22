#include "Events/InputChordDetector.h"

namespace Events {

    void InputChordDetector::SetKeys(std::uint32_t k1, std::uint32_t k2) {
        key1 = k1;
        key2 = k2;
    }

    bool InputChordDetector::OnButtonEvent(RE::ButtonEvent* btn) {
        const auto id = btn->GetIDCode();
        const auto now = std::chrono::steady_clock::now();

        UpdateKeyState(id, btn, now);

        // Key combination detection: Press and hold key1 + press and hold key2 (down)
        if (id == key2 && btn->IsDown()) {
            if (state1.isDown && !state2.consumed) {
                state2.consumed = true;
                return true;
            }
        }

        // Reverse Combination
        if (id == key1 && btn->IsDown()) {
            if (state2.isDown && !state1.consumed) {
                state1.consumed = true;
                return true;
            }
        }

        return false;
    }

    void InputChordDetector::ResetOnKeyUp(std::uint32_t id) {
        if (id == key1) state1 = {};
        if (id == key2) state2 = {};
    }

    void InputChordDetector::UpdateKeyState(std::uint32_t id, RE::ButtonEvent* btn,
                                            std::chrono::steady_clock::time_point now) {
        InputKeyState* s = nullptr;

        if (id == key1)
            s = &state1;
        else if (id == key2)
            s = &state2;
        else
            return;

        if (btn->IsDown()) {
            if (!s->isDown) {
                s->isDown = true;
                s->downTime = now;
                s->consumed = false;
            }
        } else if (btn->IsUp()) {
            ResetOnKeyUp(id);
        }
    }
}  // namespace Input
