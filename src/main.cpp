#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include "core/simplicial_complex.hpp"
#include "core/embedding-points.hpp"


void print_stats(const SimplicialComplex& K) {
    std::cout << "Vertices: " << K.get_vertices().size() << std::endl;
    for (int d = 0; d <= K.max_dim(); d++) {
        std::cout << "Dim " << d << ": " << K.get_simplices(d).size() << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

void embedding_check() {
    SimplicialComplex K;
    K.add_simplex({1,2,3});
    K.add_simplex({3,4});

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


int main() {
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
    embedding_check();
}

