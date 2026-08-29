#pragma once

#include "scene/Component.h"

#include <glm/vec2.hpp>

namespace eng
{
    class RectTransformComponent : public Component
    {
        COMPONENT(RectTransformComponent)
    public:
        void LoadProperties(const nlohmann::json& json) override;

        const glm::vec2& GetSize() const;
        void SetSize(const glm::vec2& size);

        const glm::vec2& GetAnchor() const;
        void SetAnchor(const glm::vec2& anchor);

        const glm::vec2& GetPivot() const;
        void SetPivot(const glm::vec2& pivot);

        glm::vec2 GetScreenPosition();

    private:
        glm::vec2 m_size = glm::vec2(0.0f);
        glm::vec2 m_anchor = glm::vec2(0.0f);
        glm::vec2 m_pivot = glm::vec2(0.0f);
    };
}