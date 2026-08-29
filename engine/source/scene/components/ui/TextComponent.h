#pragma once

#include "scene/components/ui/UIElementComponent.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <string>

namespace eng
{
    class Font;

    class TextComponent : public UIElementComponent
    {
        COMPONENT_2(TextComponent, UIElementComponent)
    public:
        void LoadProperties(const nlohmann::json& json) override;
        void Render(CanvasComponent* canvas) override;

        const std::string& GetText() const;
        void SetText(const std::string& text);

        const glm::vec4& GetColor() const;
        void SetColor(const glm::vec4& color);

        const std::shared_ptr<Font>& GetFont() const;
        void SetFont(const std::shared_ptr<Font>& font);
        void SetFont(const std::string& path, int size);

        glm::vec2 GetPivotPos();

    private:
        std::string m_text;
        glm::vec4 m_color = glm::vec4(1.0f);
        std::shared_ptr<Font> m_font;
    };
}