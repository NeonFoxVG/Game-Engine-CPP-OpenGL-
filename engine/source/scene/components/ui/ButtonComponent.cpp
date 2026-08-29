#include "scene/components/ui/ButtonComponent.h"
#include "scene/components/ui/CanvasComponent.h"
#include "scene/components/ui/RectTransformComponent.h"
#include "scene/GameObject.h"

namespace eng
{
    void ButtonComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("color"))
        {
            auto& colorObj = json["color"];
            SetColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            ));
        }

        if (json.contains("hovered"))
        {
            auto& colorObj = json["hovered"];
            SetHoveredColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            ));
        }

        if (json.contains("pressed"))
        {
            auto& colorObj = json["pressed"];
            SetPressedColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            ));
        }
    }

    void ButtonComponent::Render(CanvasComponent* canvas)
    {
        if (!canvas)
        {
            return;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt)
        {
            return;
        }

        auto ownerPos = rt->GetScreenPosition();
        ownerPos -= rt->GetSize() * rt->GetPivot();

        canvas->DrawRect(
            ownerPos,
            ownerPos + rt->GetSize(),
            *m_currentColor
        );
    }

    bool ButtonComponent::HitTest(const glm::vec2& pos)
    {
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt)
        {
            return false;
        }

        auto ownerPos = rt->GetScreenPosition();
        auto p1 = ownerPos - rt->GetSize() * rt->GetPivot();
        auto p2 = p1 + rt->GetSize();

        return (p1.x <= pos.x && p2.x >= pos.x && p1.y <= pos.y && p2.y >= pos.y);
    }

    void ButtonComponent::OnPointerEnter()
    {
        m_currentColor = &m_hoveredColor;
    }

    void ButtonComponent::OnPointerExit()
    {
        m_currentColor = &m_color;
    }

    void ButtonComponent::OnPointerUp()
    {
        m_currentColor = &m_hoveredColor;
    }

    void ButtonComponent::OnPointerDown()
    {
        m_currentColor = &m_pressedColor;
    }

    void ButtonComponent::OnClick()
    {
        if (onClick)
        {
            onClick();
        }
    }

    void ButtonComponent::SetColor(const glm::vec4& color)
    {
        m_color = color;
    }

    const glm::vec4& ButtonComponent::GetColor() const
    {
        return m_color;
    }

    void ButtonComponent::SetHoveredColor(const glm::vec4& color)
    {
        m_hoveredColor = color;
    }

    const glm::vec4& ButtonComponent::GetHoveredColor() const
    {
        return m_hoveredColor;
    }

    void ButtonComponent::SetPressedColor(const glm::vec4& color)
    {
        m_pressedColor = color;
    }

    const glm::vec4& ButtonComponent::GetPressedColor() const
    {
        return m_pressedColor;
    }
}