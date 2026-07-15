#pragma once

#include "shader.hpp"
#include "buffers.hpp"
#include <memory>

struct PipelineSpecs{
    std::shared_ptr<Shader> _targetShader;
    BufferLayout _layout;
};

class Pipeline{
public:
    virtual ~Pipeline() = default;
    virtual void Bind() = 0;
    virtual const PipelineSpecs& GetSpecs() const = 0;
    static std::shared_ptr<Pipeline> Create(const PipelineSpecs& specs);
};