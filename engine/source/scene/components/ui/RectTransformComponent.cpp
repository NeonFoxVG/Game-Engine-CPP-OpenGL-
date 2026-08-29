#include "scene/components/ui/RectTransformComponent.h"
#include "scene/GameObject.h"

namespace eng
{
    void RectTransformComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("size"))
        {
            auto& sizeObj = json["size"];
            SetSize(glm::vec2(
                sizeObj.value("x", 1.0f),
                sizeObj.value("y", 1.0f)
            ));
        }

        if (json.contains("anchor"))
        {
            auto& anchorObj = json["anchor"];
            SetAnchor(glm::vec2(
                anchorObj.value("x", 0.0f),
                anchorObj.value("y", 0.0f)
            ));
        }

        if (json.contains("pivot"))
        {
            auto& pivotObj = json["pivot"];
            SetPivot(glm::vec2(
                pivotObj.value("x", 0.0f),
                pivotObj.value("y", 0.0f)
            ));
        }
    }

    const glm::vec2& RectTransformComponent::GetSize() const
    {
        return m_size;
    }

    void RectTransformComponent::SetSize(const glm::vec2& size)
    {
        m_size = size;
    }

    const glm::vec2& RectTransformComponent::GetAnchor() const
    {
        return m_anchor;
    }

    void RectTransformComponent::SetAnchor(const glm::vec2& anchor)
    {
        m_anchor = anchor;
    }

    const glm::vec2& RectTransformComponent::GetPivot() const
    {
        return m_pivot;
    }

    void RectTransformComponent::SetPivot(const glm::vec2& pivot)
    {
        m_pivot = pivot;
    }

    glm::vec2 RectTransformComponent::GetScreenPosition()
    {
        auto parent = GetOwner()->GetParent();
        if (!parent || !parent->GetComponent<RectTransformComponent>())
        {
            return GetOwner()->GetPosition2D();
        }

        auto rect = parent->GetComponent<RectTransformComponent>();
        glm::vec2 parentAnchorPos = rect->GetScreenPosition() +
            (rect->GetAnchor() - rect->GetPivot()) * rect->GetSize();

        return GetOwner()->GetPosition2D() + parentAnchorPos;
    }
}