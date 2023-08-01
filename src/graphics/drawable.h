#pragma once

#include "glm/mat4x4.hpp"

typedef size_t Handle;

class Drawable {
public:
    Handle mesh_hdl;
    Handle material_hdl;
    glm::mat4 model;
};
