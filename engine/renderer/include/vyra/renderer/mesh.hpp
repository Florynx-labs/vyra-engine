#pragma once

#include "vyra/core/base.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <string>

namespace vyra::renderer {

    // -----------------------------------------------------------------------
    // Vertex layout for 3D meshes
    // -----------------------------------------------------------------------
    struct Vertex3D {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    };

    // -----------------------------------------------------------------------
    // Mesh — CPU-side geometry data
    // Uploaded to the GPU via RHIBuffer.
    // -----------------------------------------------------------------------
    struct Mesh {
        std::string              Name;
        std::vector<Vertex3D>    Vertices;
        std::vector<uint32_t>    Indices;

        // Factory helpers
        static Mesh CreateCube(float halfExtent = 0.5f);
        static Mesh CreateGrid(int divisions = 10, float spacing = 1.0f);
        static Mesh CreateSphere(float radius = 0.5f, int rings = 16, int sectors = 16);
    };

} // namespace vyra::renderer
