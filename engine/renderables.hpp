#pragma once

#include "buffers.hpp"
#include <memory>
#include "camera.hpp"
#include "components.hpp"
#include "glm/glm.hpp"

struct Mesh{
    Mesh(const std::shared_ptr<VertexBuffer>& vb, const std::shared_ptr<IndexBuffer>& ib) : _vertexBuffer(vb), _indexBuffer(ib) {}

    std::shared_ptr<VertexBuffer> _vertexBuffer;
    std::shared_ptr<IndexBuffer> _indexBuffer;
    glm::mat4 _local_transform;
    uint32_t _material_idx;
};

struct Model{
    std::vector<Mesh> _meshes;
    std::vector<uint32_t> _material_idx;
    std::string _path;
};

struct TextureComponent{
    uint32_t _tex_id;
    std::string _path;
    REV_TEXTURE_TYPE _tex_type;
};

struct VertexComponent{
    glm::vec3 _position;
    glm::vec3 _normal;
    glm::vec2 _tex_coords;
};

// .======= .=======  //======
// ||       ||        \\
// ||=====  ||         `====.
// ||       ||              \\
// `======= `=======  ======//
struct TransformComponent{
private:
    glm::vec3 _position = {0, 0, 0};
    glm::vec3 _rotation = {0, 0, 0};
    glm::vec3 _scale    = {1.0F, 1.0F, 1.0F};

    mutable glm::mat4 _model_matrix = glm::mat4(1.0F);

    mutable bool _need_calc = true;
public:
    TransformComponent() = default;

    void setPosition(const glm::vec3& pos) { _position = pos; _need_calc = true; }
    void setRotation(const glm::vec3& angles) { _rotation = angles; _need_calc = true; };
    void setScale(const glm::vec3& scale) { _scale = scale; _need_calc = true; }

    // Relative coordinates taken as the parameters
    void Translate(const glm::vec3& delta) { _position += delta; _need_calc = true; }
    // Relative angles taken as the parameters
    void Rotate(const glm::vec3& deltaDegrees) { _rotation += deltaDegrees; _need_calc = true; }

    const glm::vec3& GetPosition() const { return _position; }
    const glm::vec3& GetRotation() const { return _rotation; }
    const glm::vec3& GetScale()    const { return _scale; }


    glm::mat4 getTransform() const {
        if (_need_calc) {
            glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), _position);

            // 1. Convert Euler degrees straight to a stable Quaternion
            glm::quat quadRotation = glm::quat(glm::radians(_rotation));

            // 2. Convert Quaternion to a clean 4x4 Rotation Matrix
            glm::mat4 rotationMat = glm::toMat4(quadRotation);

            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), _scale);

            // 3. Cache the final result
            _model_matrix = translationMat * rotationMat * scaleMat;
            _need_calc = false;
        }
        return _model_matrix;
    }
};

struct TagComponent{
    std::string _tag;
};

struct MeshComponent{
    uint32_t _model_id;
    uint32_t _shader_id = 0;
};

struct DirectionalLightComponent{
    
    alignas(16) glm::vec3 _direction = {0.0, -1.0, 0.0};
    alignas(16) glm::vec3 _color = {1.0, 1.0, 1.0};
    
    float _ambient = 0.02F;
    float _specular = 1.0F;
    float _diffuse = 1.0F;
    
    bool _enabled = true;

    glm::mat4 _light_space_matrix;
};

struct SpotLightComponent{

    alignas(16) glm::vec3 _position = {0.0, 0.0, 0.0};
    alignas(16) glm::vec3 _direction = { 0.0, 0.0, -1.0};
    alignas(16) glm::vec3 _color = { 1.0, 1.0, 1.0 };

    // Attenuation variables
    float _constant = 1.0F;
    float _linear = 0.0F;
    float _quadratic = 0.0F;

    float _diffuse = 1.0F;
    float _ambient = 0.02F;
    float _specular = 1.0F;

    float _inner_cutoff = 30.0F;
    float _outer_cutoff = 60.0F;

    bool _enabled = true;

    glm::mat4 _light_space_matrix;
};

struct CameraComponent{
    uint32_t _id;
};

// .=====================.
// ||    SCENE DATA     ||
// `=====================`

struct SceneData{
    Camera3D _camera;
    DirectionalLightComponent _directional_light;
    std::vector<SpotLightComponent> _spot_lights;

    uint32_t _directional_shadow_map_id = 0;
    uint32_t _spot_shadow_map_id = 0;
};
