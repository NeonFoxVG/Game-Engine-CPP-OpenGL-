#pragma once

#include "Common.h"
#include "scene/Component.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <memory>

namespace eng
{
    class UIElementComponent;
    class Texture;
    class Mesh;

    class CanvasComponent : public Component
    {
        COMPONENT(CanvasComponent)
    public:
        void LoadProperties(const nlohmann::json& json) override;
        void Update(float deltaTime) override;
        void Init() override;
        void Render(UIElementComponent* element);
        void BeginRendering();
        void Flush();
        void CollectUI(UIElementComponent* element, std::vector<UIElementComponent*>& out);

        void DrawRect(
            const glm::vec2& lowerLeftPos, const glm::vec2& upperRightPos,
            const glm::vec2& lowerLeftUV, const glm::vec2& upperRightUV,
            Texture* texture, const glm::vec4& color
        );
        void DrawRect(
            const glm::vec2& lowerLeftPos, const glm::vec2& upperRightPos,
            const glm::vec4& color
        );

        void SetActive(bool active);
        bool IsActive() const;

    private:
        void UpdateBatches(Texture* texture);

    private:
        std::vector<UIBatch> m_batches;
        std::vector<float> m_vertices;
        std::vector<uint32_t> m_indices;
        std::shared_ptr<Mesh> m_mesh;
        bool m_active = true;
    };
}