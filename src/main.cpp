#pragma once

#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "core/simplicial_complex.hpp"
#include "core/embedding-points.hpp"
#include "render/renderer.hpp"


void print_stats(const SimplicialComplex& K) {
    std::cout << "Vertices: " << K.get_vertices().size() << std::endl;
    for (int d = 0; d <= K.max_dim(); d++) {
        std::cout << "Dim " << d << ": " << K.get_simplices(d).size() << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

void embedding_check(const SimplicialComplex& K) {
    std::cout << "Vertices: ";
    for (int v : K.get_vertices()) std::cout << v << " ";
    std::cout << std::endl;

    Embedding emb(K);
    emb.initialize_random();
    auto pos = emb.get_positions();

    for (auto [v, p] : pos) {
        std::cout << "Vertex " << v << ": (" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;

        assert(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z));
    }

    auto dist = [](const glm::vec3 &a, const glm::vec3 &b){
        return glm::length(a-b);
    };

    for (auto &e : K.get_simplices(1)) {
        int u = e.v[0], v = e.v[1];
        float d = dist(pos[u], pos[v]);
        std::cout << "Edge {" << u << "," << v << "} length = " << d << std::endl;
        assert(d > 0.01f);
    }

    for (auto &t1 : K.get_simplices(2)) {
        std::vector<int> t = t1.v;
        glm::vec3 a = pos[t[0]];
        glm::vec3 b = pos[t[1]];
        glm::vec3 c = pos[t[2]];
        glm::vec3 cross = glm::cross(b-a, c-a);
        float area = 0.5f * glm::length(cross);
        std::cout << "Triangle {" << t[0] << "," << t[1] << "," << t[2] << "} area = " << area << std::endl;
        assert(area > 0.01f);
    }

    std::cout << "Embedding test passed!" << std::endl;
}

void visual_check() {

}

void simplex_tests() {
    {
        std::cout << "Test 1: single vertex" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1});
        assert(K.get_vertices().size() == 1);
        assert(K.get_simplices(0).size() == 1);
        print_stats(K);
    }

    {
        std::cout << "Test 2: edge closure" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1, 2});
        assert(K.get_simplices(1).size() == 1);
        assert(K.get_simplices(0).size() == 2);
        print_stats(K);
    }

    {
        std::cout << "Test 3: triangle closure" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1, 2, 3});
        assert(K.get_simplices(2).size() == 1);
        assert(K.get_simplices(1).size() == 3);
        assert(K.get_simplices(0).size() == 3);
        print_stats(K);
    }

    {
        std::cout << "Test 4: tetrahedron closure" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1, 2, 3, 4});
        assert(K.get_simplices(3).size() == 1);
        assert(K.get_simplices(2).size() == 4);
        assert(K.get_simplices(1).size() == 6);
        assert(K.get_simplices(0).size() == 4);
        print_stats(K);
    }

    {
        std::cout << "Test 5: duplicate handling" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1, 2});
        K.add_simplex({2, 1});
        assert(K.get_simplices(1).size() == 1);
        assert(K.get_simplices(0).size() == 2);
        print_stats(K);
    }

    {
        std::cout << "Test 6: multiple simplices" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1, 2, 3});
        K.add_simplex({3, 4});
        assert(K.get_vertices().size() == 4);
        assert(K.get_simplices(1).size() == 4);
        print_stats(K);
    }

    {
        std::cout << "Test 7: max dimension tracking" << std::endl;
        SimplicialComplex K;
        K.add_simplex({1});
        assert(K.max_dim() == 0);
        K.add_simplex({1, 2});
        assert(K.max_dim() == 1);
        K.add_simplex({1, 2, 3});
        assert(K.max_dim() == 2);
        print_stats(K);
    }

    std::cout << "Simplex tests passed." << std::endl;
}

void read_input(SimplicialComplex& K) {
    int n;
    std::cout << "Enter number of simplices: ";
    std::cin >> n;

    std::cout << "Enter simplices:\n";
    for (int i = 0; i < n; i++) {
        std::cout << "Simplex #" << i+1 << " size(dimension): ";
        int m;
        std::cin >> m;
        std::cout << "Enter vertices: ";
        std::vector<int> simplex(m);
        for (int j = 0; j < m; j++) {
            std::cin >> simplex[j];
        }
        K.add_simplex(simplex);
    }
}
const int WIDTH = 2000, HEIGHT = 2000;
double lastX = WIDTH / 2;
double lastY = HEIGHT / 2;
bool firstMouse = true;
float radius = 5.0f;
float yaw = 0.0f;
float pitch = 0.0f;
glm::vec3 target = glm::vec3(0.0f);
glm::vec3 camPos;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float dx = xpos - lastX;
    float dy = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        float sensitivity = 0.005f;
        yaw   += dx * sensitivity;
        pitch += dy * sensitivity;

        pitch = std::clamp(pitch, -1.5f, 1.5f);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        float panSpeed = 0.003f * radius;
        glm::vec3 forward = glm::normalize(target - camPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));

        target += -right * dx * panSpeed;
        target += up * dy * panSpeed;
    }
}

void scroll_callback(GLFWwindow* window, double x_off, double y_off) {
    radius -= (float)y_off;
    radius = std::clamp(radius, 1.0f, 20.0f);
}

int main() {
    SimplicialComplex K;
    read_input(K);
    //simplex_tests();
    //embedding_check(K);

    std::cout << "Complex built\n";

    Embedding Emb(K);
    Emb.initialize_random(3.0f, 42);
    Emb.run(500, 0.01f);

    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Simplicial Viewer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window\n";
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Renderer renderer;
    renderer.init();
    renderer.upload(K, Emb.get_positions());

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camPos.x = radius * cos(pitch) * cos(yaw);
        camPos.y = radius * sin(pitch);
        camPos.z = radius * cos(pitch) * sin(yaw);

        camPos += target;

        glm::mat4 view = glm::lookAt(
            camPos,
            target,
            glm::vec3(0,1,0)
        );

        glm::mat4 proj = glm::perspective(
            glm::radians(45.0f),
            (float)WIDTH / HEIGHT,
            0.1f,
            100.0f
        );


        glm::mat4 MVP = proj * view;

        renderer.draw(MVP);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

