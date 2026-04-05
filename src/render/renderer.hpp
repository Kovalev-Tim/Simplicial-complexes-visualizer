#include "render.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


static unsigned int compile_shader(unsigned int type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cout << "Shader error:\n" << info << std::endl;
    }

    return shader;
}


void Renderer::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO_edges);
    glGenBuffers(1, &EBO_triangles);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setup_shaders();
    create_grid();
}

void Renderer::upload(const SimplicialComplex& K, const std::map<int, glm::vec3>& pos) {
    std::vector<glm::vec3> vertices;
    std::map<int, int> id_to_idx;

    int idx = 0;
    for (auto &[v, p] : pos) {
        id_to_idx[v] = idx++;
        vertices.push_back(p);
    }

    // edges
    std::vector<unsigned int> edges;
    for (auto &e : K.get_simplices(1)) {
        edges.push_back(id_to_idx[e.v[0]]);
        edges.push_back(id_to_idx[e.v[1]]);
    }

    // triangles
    std::vector<unsigned int> tris;
    if (K.max_dim() >= 2) {
        for (auto &t : K.get_simplices(2)) {
            tris.push_back(id_to_idx[t.v[0]]);
            tris.push_back(id_to_idx[t.v[1]]);
            tris.push_back(id_to_idx[t.v[2]]);
        }
    }

    edge_count = edges.size();
    tri_count = tris.size();


    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,edges.size() * sizeof(unsigned int), edges.data(), GL_STATIC_DRAW);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_triangles);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(unsigned int), tris.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}


void Renderer::draw(const glm::mat4& MVP) {
    glUseProgram(shaderProgram);

    int loc = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_triangles);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.3f, 0.7f, 1.0f);
    glDrawElements(GL_TRIANGLES, tri_count, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 1.0f);
    glDrawElements(GL_LINES, edge_count, GL_UNSIGNED_INT, 0);

    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.8f, 0.8f, 0.8f);
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridVertexCount);

    glBindVertexArray(0);
}

void Renderer::create_grid(int N, float step) {
    std::vector<float> grid;

    for (int i = -N; i <= N; i++) {
        float x = i * step;

        // Z coord
        grid.push_back(x); grid.push_back(0); grid.push_back(-N * step);
        grid.push_back(x); grid.push_back(0); grid.push_back(N * step);

        // X coord
        grid.push_back(-N * step); grid.push_back(0); grid.push_back(x);
        grid.push_back(N * step);  grid.push_back(0); grid.push_back(x);
    }

    gridVertexCount = grid.size() / 3;

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, grid.size() * sizeof(float), grid.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Renderer::setup_shaders() {
    const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 MVP;
        void main() {
            gl_Position = MVP * vec4(aPos, 1.0);
        }
    )";

    const char* fs = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";

    unsigned int v = compile_shader(GL_VERTEX_SHADER, vs);
    unsigned int f = compile_shader(GL_FRAGMENT_SHADER, fs);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, v);
    glAttachShader(shaderProgram, f);
    glLinkProgram(shaderProgram);

    glDeleteShader(v);
    glDeleteShader(f);
}

