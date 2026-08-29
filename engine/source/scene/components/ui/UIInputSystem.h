#pragma once

#include <vector>

namespace eng
{
    class CanvasComponent;
    class UIElementComponent;

    class UIInputSystem
    {
    public:
        void SetActive(bool active);
        bool IsActive() const;
        void SetCanvas(CanvasComponent* canvas);
        CanvasComponent* GetCanvas();
        void Update(float deltaTime);

        std::vector<UIElementComponent*> CollectUI(CanvasComponent* canvas);

    private:
        bool m_active = false;
        CanvasComponent* m_activeCanvas = nullptr;
        UIElementComponent* m_hovered = nullptr;
        UIElementComponent* m_pressed = nullptr;
    };
}