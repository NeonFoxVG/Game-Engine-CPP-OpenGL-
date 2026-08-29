#pragma once

#include "scene/components/ui/UIElementComponent.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <functional>

namespace eng
{
    class ButtonComponent : public UIElementComponent
    {
        COMPONENT_2(ButtonComponent, UIElementComponent)

    public:
        void LoadProperties(const nlohmann::json& json) override;
        void Render(CanvasComponent* canvas) override;
        bool HitTest(const glm::vec2& pos) override;
        void OnPointerEnter() override;
        void OnPointerExit() override;
        void OnPointerUp() override;
        void OnPointerDown() override;
        void OnClick() override;

        void SetColor(const glm::vec4& color);
        const glm::vec4& GetColor() const;

        void SetHoveredColor(const glm::vec4& color);
        const glm::vec4& GetHoveredColor() const;

        void SetPressedColor(const glm::vec4& color);
        const glm::vec4& GetPressedColor() const;

        std::function<void()> onClick;

    private:
        glm::vec4 m_color = glm::vec4(1.0f);
        glm::vec4 m_hoveredColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 m_pressedColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4* m_currentColor = &m_color;
    };
}