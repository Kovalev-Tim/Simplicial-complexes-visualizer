#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "../core/simplicial_complex.hpp"

class Renderer {
public:
    void init();

    void upload(
        const SimplicialComplex& K,
        const std::map<int, glm::vec3>& positions
    );

    void draw(const glm::mat4& MVP);

    void create_grid(const int N = 20, const float step = 1.0f);

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO_edges = 0;
    unsigned int EBO_triangles = 0;

    unsigned int shaderProgram = 0;
    unsigned int gridVAO, gridVBO;
    int gridVertexCount;

    int edge_count = 0;
    int tri_count = 0;

    void setup_shaders();
};

