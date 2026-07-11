#include "vk_shader.hpp"
#include "vk_context.hpp"

#include <fstream>

// Helper functions to read the code from the provided file path
std::vector<char> VulkanShader::ReadFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    return buffer;
}

vk::raii::ShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };
    vk::raii::ShaderModule shaderModule{_context->GetDevice(), createInfo};
    return shaderModule;
}

VulkanShader::VulkanShader(VulkanContext* context, const std::string& vertexPath, const std::string& fragmentPath) : _context(context) {
    std::vector<char> vertexCode = ReadFile(vertexPath);
    std::vector<char> fragmentCode = ReadFile(fragmentPath);

    // Create the respective shader modules for the shaders at the given path
    _vertexModule = CreateShaderModule(vertexCode);
    _fragmentModule = CreateShaderModule(fragmentCode);
}
