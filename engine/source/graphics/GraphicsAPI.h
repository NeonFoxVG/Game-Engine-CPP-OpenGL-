#pragma once

#include <GL/glew.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace eng
{
    class ShaderProgram;
    class Material;
    class Mesh;

    enum class BlendMode
    {
        Disabled,
        Alpha,
        Additive,
        Multiply
    };

    struct Rect
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
    };

    struct ShaderKey
    {
        std::string vertexSource;
        std::string fragmentSource;

        bool operator==(const ShaderKey& other) const
        {
            return vertexSource == other.vertexSource &&
                fragmentSource == other.fragmentSource;
        }
    };

    struct ShaderKeyHash
    {
        std::size_t operator()(const ShaderKey& key) const
        {
            std::size_t h1 = std::hash<std::string>{}(key.vertexSource);
            std::size_t h2 = std::hash<std::string>{}(key.fragmentSource);
            return h1 ^ (h2 << 1);
        }
    };

    class GraphicsAPI
    {
    public:
        bool Init();
        std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertexSource, 
            const std::string& fragmentSource);
        const std::shared_ptr<ShaderProgram>& GetDefaultShaderProgram();
        const std::shared_ptr<ShaderProgram>& GetDefault2DShaderProgram();
        const std::shared_ptr<ShaderProgram>& GetDefaultUIShaderProgram();
        GLuint CreateVertexBuffer(const std::vector<float>& vertices);
        GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);

        void SetClearColor(float r, float g, float b, float a);
        void ClearBuffers();
        const Rect& GetViewport() const;
        void SetViewport(int x, int y, int width, int height);
        void SetDepthTestEnabled(bool enabled);
        void SetBlendMode(BlendMode mode);

        void BindShaderProgram(ShaderProgram* shaderProgram);
        void BindMaterial(Material* material);
        void BindMesh(Mesh* mesh);
        void UnbindMesh(Mesh* mesh);
        void DrawMesh(Mesh* mesh);

    private:
        Rect m_viewport;
        std::shared_ptr<ShaderProgram> m_defaultShaderProgram;
        std::shared_ptr<ShaderProgram> m_default2DShaderProgram;
        std::shared_ptr<ShaderProgram> m_defaultUIShaderProgram;
        std::unordered_map<ShaderKey, std::shared_ptr<ShaderProgram>, ShaderKeyHash> m_shaderCache;
    };
}