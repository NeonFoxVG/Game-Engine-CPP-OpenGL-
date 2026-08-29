#include "scene/components/ui/UIInputSystem.h"
#include "scene/components/ui/CanvasComponent.h"
#include "scene/components/ui/UIElementComponent.h"
#include "Engine.h"

#include <GLFW/glfw3.h>

namespace eng
{
    void UIInputSystem::SetActive(bool active)
    {
        m_active = active;
    }

    bool UIInputSystem::IsActive() const
    {
        return m_active;
    }

    void UIInputSystem::SetCanvas(CanvasComponent* canvas)
    {
        m_activeCanvas = canvas;
    }

    CanvasComponent* UIInputSystem::GetCanvas()
    {
        return m_activeCanvas;
    }

    void UIInputSystem::Update(float deltaTime)
    {
        if (!m_active || !m_activeCanvas || !m_activeCanvas->IsActive())
        {
            return;
        }

        auto& input = Engine::GetInstance().GetInputManager();
        bool mouseDown = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool mousePressed = input.WasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool mouseReleased = input.WasMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);

        auto mousePos = input.GetMousePositionCurrent();
        mousePos.y = Engine::GetInstance().GetGraphicsAPI().GetViewport().height - mousePos.y;

        UIElementComponent* hit = nullptr;
        auto uiElements = CollectUI(m_activeCanvas);
        for (auto element : uiElements)
        {
            if (element->HitTest(mousePos))
            {
                hit = element;
                break;
            }
        }

        if (hit != m_hovered)
        {
            if (m_hovered)
            {
                m_hovered->OnPointerExit();
            }

            m_hovered = hit;

            if (m_hovered)
            {
                m_hovered->OnPointerEnter();
            }
            m_pressed = nullptr;
        }

        if (!m_pressed)
        {
            if (mousePressed && m_hovered)
            {
                m_pressed = m_hovered;
                m_pressed->OnPointerDown();
            }
        }

        if (mouseReleased)
        {
            if (m_pressed)
            {
                m_pressed->OnPointerUp();

                if (m_pressed == m_hovered)
                {
                    m_pressed->OnClick();
                }
            }

            m_pressed = nullptr;
        }
    }

    std::vector<UIElementComponent*> UIInputSystem::CollectUI(CanvasComponent* canvas)
    {
        std::vector<UIElementComponent*> result;
        GameObject* canvasObject = canvas->GetOwner();
        const auto& children = canvasObject->GetChildren();

        for (const auto& child : children)
        {
            if (auto component = child->GetComponent<UIElementComponent>())
            {
                canvas->CollectUI(component, result);
            }
        }
        return result;
    }

}