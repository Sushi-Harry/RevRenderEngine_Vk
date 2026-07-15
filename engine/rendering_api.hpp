#pragma once

#include <cstdint>
#include "utilities.hpp"
#include "glm/glm.hpp"
#include <memory>

class RenderingAPI{
public:
    virtual ~RenderingAPI() = default;

    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(float r, float g, float b, float a) = 0;
    virtual void Clear() = 0;

    virtual bool BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual void DrawIndexed(uint32_t indexCount, const glm::mat4& viewProj, const glm::mat4& model) = 0;

    static std::unique_ptr<RenderingAPI> Create();
};