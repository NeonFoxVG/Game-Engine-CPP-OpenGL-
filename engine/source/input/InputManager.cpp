#include "input/InputManager.h"

namespace eng
{
    void InputManager::SetKeyPressed(int key, bool pressed)
    {
        if (key < 0 || key >= static_cast<int>(m_keys.size()))
        {
            return;
        }
        m_keys[key] = pressed;
    }

    bool InputManager::IsKeyPressed(int key)
    {
        if (key < 0 || key >= static_cast<int>(m_keys.size()))
        {
            return false;
        }

        return m_keys[key];
    }

    void InputManager::SetMouseButtonPressed(int button, bool pressed)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeys.size()))
        {
            return;
        }
        m_mouseKeys[button] = pressed;
    }

    bool InputManager::IsMouseButtonPressed(int button)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeys.size()))
        {
            return false;
        }
        return m_mouseKeys[button];
    }

    void InputManager::SetMouseButtonWasPressed(int button, bool pressed)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeyPressed.size()))
        {
            return;
        }
        m_mouseKeyPressed[button] = pressed;
    }

    bool InputManager::WasMouseButtonPressed(int button) const
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeyPressed.size()))
        {
            return false;
        }
        return m_mouseKeyPressed[button];
    }

    void InputManager::SetMouseButtonWasReleased(int button, bool pressed)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeyReleased.size()))
        {
            return;
        }
        m_mouseKeyReleased[button] = pressed;
    }

    bool InputManager::WasMouseButtonReleased(int button) const
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeyReleased.size()))
        {
            return false;
        }
        return m_mouseKeyReleased[button];
    }

    void InputManager::SetMousePositionOld(const glm::vec2& pos)
    {
        m_mousePositionOld = pos;
    }

    const glm::vec2& InputManager::GetMousePositionOld() const
    {
        return m_mousePositionOld;
    }

    void InputManager::SetMousePositionCurrent(const glm::vec2& pos)
    {
        m_mousePositionCurrent = pos;
    }

    const glm::vec2& InputManager::GetMousePositionCurrent() const
    {
        return m_mousePositionCurrent;
    }

    void InputManager::SetMousePositionChanged(bool changed)
    {
        m_mousePositionChanged = changed;
    }

    bool InputManager::IsMousePositionChanged() const
    {
        return m_mousePositionChanged;
    }

    void InputManager::ClearStates()
    {
        SetMousePositionChanged(false);
        for (auto k : m_mouseKeyPressed)
        {
            SetMouseButtonWasPressed(k, false);
        }
        for (auto k : m_mouseKeyReleased)
        {
            SetMouseButtonWasReleased(k, false);
        }
    }
}