#pragma once

#include "components.hpp"
#include <string>
#include <cstdint>
#include <memory>

enum class PixelFormat{
    RGB8,
    RGBA8
};

struct TextureInfo{
    uint32_t _width = 1;
    uint32_t _height = 1;
    PixelFormat _format = PixelFormat::RGB8;
    bool _gen_mipmaps = true;
};

class Texture {
public:
    virtual ~Texture() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual const TextureInfo& GetTextureInfo() const = 0;

    virtual void Bind(uint32_t slot) const = 0;
};

class Texture2D : public Texture{
public:
    static std::shared_ptr<Texture2D> Create(const TextureInfo& info);
    static std::shared_ptr<Texture2D> CreateDefault(REV_TEXTURE_TYPE type);
    static std::shared_ptr<Texture2D> Create(const std::string& path);
};
