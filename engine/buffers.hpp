#pragma once
#include <cstdint>
#include <memory>
#include "buffer_layout.hpp"

class VertexBuffer{
public:
    virtual ~VertexBuffer() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetData(const void* data, uint32_t size) = 0;

    virtual const BufferLayout& GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout& layout) = 0;

    static std::shared_ptr<VertexBuffer> Create(float* vertices, uint32_t size);
};

class IndexBuffer {
public:
    virtual ~IndexBuffer() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual uint32_t GetCount() const = 0;
    static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);
};