#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <utility>
#include <vector>
#include "shader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

enum class REV_TEXTURE_TYPE : uint8_t{
    REV_DIFFUSE,
    REV_SPECULAR,
    REV_NORMAL,
    REV_NONE
};
