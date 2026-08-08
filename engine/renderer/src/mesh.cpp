#include "vyra/renderer/mesh.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <numbers>

namespace vyra::renderer {

    Mesh Mesh::CreateCube(float h) {
        Mesh mesh;
        mesh.Name = "Cube";

        // 6 faces × 4 vertices each
        struct FaceData { glm::vec3 normal; glm::vec3 up; glm::vec3 right; float d; };
        const FaceData faces[] = {
            { { 0, 0, 1}, {0,1,0}, {1,0,0},  h }, // Front
            { { 0, 0,-1}, {0,1,0}, {-1,0,0}, h }, // Back
            { {-1, 0, 0}, {0,1,0}, {0,0,-1}, h }, // Left
            { { 1, 0, 0}, {0,1,0}, {0,0, 1}, h }, // Right
            { { 0, 1, 0}, {0,0,-1},{1,0, 0}, h }, // Top
            { { 0,-1, 0}, {0,0, 1},{1,0, 0}, h }, // Bottom
        };

        for (const auto& f : faces) {
            uint32_t base = static_cast<uint32_t>(mesh.Vertices.size());
            glm::vec3 center = f.normal * f.d;

            mesh.Vertices.push_back({ center - f.right * h - f.up * h, f.normal, {0,0} });
            mesh.Vertices.push_back({ center + f.right * h - f.up * h, f.normal, {1,0} });
            mesh.Vertices.push_back({ center + f.right * h + f.up * h, f.normal, {1,1} });
            mesh.Vertices.push_back({ center - f.right * h + f.up * h, f.normal, {0,1} });

            mesh.Indices.insert(mesh.Indices.end(),
                { base, base+1, base+2, base+2, base+3, base });
        }

        return mesh;
    }

    Mesh Mesh::CreateGrid(int divisions, float spacing) {
        Mesh mesh;
        mesh.Name = "Grid";

        float total = divisions * spacing;
        float half  = total * 0.5f;

        // Lines along X
        for (int i = 0; i <= divisions; ++i) {
            float z = -half + i * spacing;
            uint32_t base = static_cast<uint32_t>(mesh.Vertices.size());
            mesh.Vertices.push_back({ {-half, 0.0f, z}, {0,1,0}, {0,0} });
            mesh.Vertices.push_back({ { half, 0.0f, z}, {0,1,0}, {1,0} });
            mesh.Indices.push_back(base);
            mesh.Indices.push_back(base + 1);
        }
        // Lines along Z
        for (int i = 0; i <= divisions; ++i) {
            float x = -half + i * spacing;
            uint32_t base = static_cast<uint32_t>(mesh.Vertices.size());
            mesh.Vertices.push_back({ {x, 0.0f, -half}, {0,1,0}, {0,0} });
            mesh.Vertices.push_back({ {x, 0.0f,  half}, {0,1,0}, {0,1} });
            mesh.Indices.push_back(base);
            mesh.Indices.push_back(base + 1);
        }

        return mesh;
    }

    Mesh Mesh::CreateSphere(float radius, int rings, int sectors) {
        Mesh mesh;
        mesh.Name = "Sphere";

        const float pi  = std::numbers::pi_v<float>;
        const float tau = 2.0f * pi;

        for (int r = 0; r <= rings; ++r) {
            float phi = pi * static_cast<float>(r) / static_cast<float>(rings);
            for (int s = 0; s <= sectors; ++s) {
                float theta = tau * static_cast<float>(s) / static_cast<float>(sectors);

                float x = std::sin(phi) * std::cos(theta);
                float y = std::cos(phi);
                float z = std::sin(phi) * std::sin(theta);

                Vertex3D v;
                v.Position = { x * radius, y * radius, z * radius };
                v.Normal   = { x, y, z };
                v.TexCoord = { static_cast<float>(s) / sectors, static_cast<float>(r) / rings };
                mesh.Vertices.push_back(v);
            }
        }

        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                uint32_t a = r * (sectors + 1) + s;
                uint32_t b = a + (sectors + 1);
                mesh.Indices.insert(mesh.Indices.end(), {a, b, a+1, b, b+1, a+1});
            }
        }

        return mesh;
    }

} // namespace vyra::renderer
