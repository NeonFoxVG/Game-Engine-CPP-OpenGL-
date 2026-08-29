#pragma once
#include <array>
#include <glm/vec2.hpp>

namespace eng
{
    class InputManager
    {
    private:
        InputManager() = default;
        InputManager(const InputManager&) = delete;
        InputManager(InputManager&&) = delete;
        InputManager& operator=(const InputManager&) = delete;
        InputManager& operator=(InputManager&&) = delete;

    public:
        void SetKeyPressed(int key, bool pressed);
        bool IsKeyPressed(int key);

        void SetMouseButtonPressed(int button, bool pressed);
        bool IsMouseButtonPressed(int button);

        void SetMouseButtonWasPressed(int button, bool pressed);
        bool WasMouseButtonPressed(int button) const;

        void SetMouseButtonWasReleased(int button, bool pressed);
        bool WasMouseButtonReleased(int button) const;

        void SetMousePositionOld(const glm::vec2& pos);
        const glm::vec2& GetMousePositionOld() const;

        void SetMousePositionCurrent(const glm::vec2& pos);
        const glm::vec2& GetMousePositionCurrent() const;

        void SetMousePositionChanged(bool changed);
        bool IsMousePositionChanged() const;

        void ClearStates();

    private:
        std::array<bool, 512> m_keys = { false };
        std::array<bool, 16> m_mouseKeys = { false };
        std::array<bool, 16> m_mouseKeyPressed = { false };
        std::array<bool, 16> m_mouseKeyReleased = { false };
        glm::vec2 m_mousePositionOld = glm::vec2(0.0f);
        glm::vec2 m_mousePositionCurrent = glm::vec2(0.0f);
        bool m_mousePositionChanged = false;

        friend class Engine;
    };
}