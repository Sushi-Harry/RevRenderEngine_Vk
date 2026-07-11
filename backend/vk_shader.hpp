#pragma once

#include "shader.hpp"

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <string>
#include <vector>

class VulkanContext;

class VulkanShader : Shader{
public:
    VulkanShader(VulkanContext* context, const std::string& v_path, const std::string& f_path);
    ~VulkanShader() override = default;

    void SetInt(const std::string& name, int val) const override;
    void SetBool(const std::string& name, bool value) const override;
    void SetFloat(const std::string& name, float value) const override;
    void SetVec2(const std::string& name, const glm::vec2& vec) const override;
    void SetVec3(const std::string& name, const glm::vec3& vec) const override;
    void SetVec4(const std::string& name, const glm::vec4& vec) const override;
    void SetMat2(const std::string& name, const glm::mat2& mat) const override;
    void SetMat3(const std::string& name, const glm::mat3& mat) const override;
    void SetMat4(const std::string& name, const glm::mat4& mat) const override;

    inline const vk::raii::ShaderModule& GetVertexModule() const { }
private:
    std::vector<char> ReadFile(const std::string& path);
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code);

    VulkanContext* _context;
    vk::raii::ShaderModule _vertexModule = nullptr;
    vk::raii::ShaderModule _fragmentModule = nullptr;

};