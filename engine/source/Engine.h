#pragma once
#include "input/InputManager.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/Texture.h"
#include "render/RenderQueue.h"
#include "scene/Scene.h"
#include "io/FileSystem.h"
#include "physics/PhysicsManager.h"
#include "audio/AudioManager.h"
#include "font/FontManager.h"
#include "scene/components/ui/UIInputSystem.h"

#include <memory>
#include <chrono>

struct GLFWwindow;
namespace eng
{
    class Application;
    class Engine
    {
    public:
        static Engine& GetInstance();

    private:
        Engine() = default;
        Engine(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&) = delete;

    public:
        bool Init(int width, int height);
        void Run();
        void Destroy();
        void SetCursorEnabled(bool enabled);

        void SetApplication(Application* app);
        Application* GetApplication();
        InputManager& GetInputManager();
        GraphicsAPI& GetGraphicsAPI();
        RenderQueue& GetRenderQueue();
        FileSystem& GetFileSystem();
        TextureManager& GetTextureManager();
        PhysicsManager& GetPhysicsManager();
        AudioManager& GetAudioManager();
        FontManager& GetFontManager();
        UIInputSystem& GetUIInputSystem();

        void SetScene(const std::shared_ptr<Scene>& scene);
        const std::shared_ptr<Scene>& GetScene();

    private:
        std::unique_ptr<Application> m_application;
        std::chrono::steady_clock::time_point m_lastTimePoint;
        GLFWwindow* m_window = nullptr;
        InputManager m_inputManager;
        GraphicsAPI m_graphicsAPI;
        RenderQueue m_rederQueue;
        FileSystem m_fileSystem;
        TextureManager m_textureManager;
        PhysicsManager m_physicsManager;
        AudioManager m_audioManager;
        FontManager m_fontManager;
        UIInputSystem m_uiInputSystem;
        std::shared_ptr<Scene> m_currentScene;
    };
}