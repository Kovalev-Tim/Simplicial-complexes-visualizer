#pragma once
#include "simplicial_complex.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <random>

using Vec3 = glm::vec3;

class Embedding {
 private:
    const SimplicialComplex& K;

    std::map<int, Vec3> pos;
    std::map<int, Vec3> force;

    float k_rep = 0.1f;
    float k_edge = 1.0f;
    float k_area = 0.2f;
    float k_vol = 0.2f;

    void compute_forces() {
        for (auto& [v, f] : force) f = Vec3(0.0f);
        apply_repulsion();
        apply_edge_forces();
        apply_triangle_forces();
        apply_tetra_forces();
    }

    void apply_repulsion() {
        for (auto v1 : K.get_vertices()) {
            for (auto v2 : K.get_vertices()) {
                if (v1 >= v2) continue;

                Vec3 d = pos[v1] - pos[v2];
                float dist = glm::length(d) + 1e-5f;

                Vec3 f = k_rep * d / (dist * dist * dist * dist);

                force[v1] += f;
                force[v2] -= f;
            }
        }
    }

    void apply_edge_forces() {
        for (const auto& s : K.get_simplices(1)) {
            auto u = s.v[0], v = s.v[1];

            Vec3 d = pos[u] - pos[v];
            float dist = glm::length(d) + 1e-5f;

            float L = 1.0f;
            Vec3 f = k_edge * (dist - L) * (d / dist);

            force[u] -= f;
            force[v] += f;
        }
    }

    void apply_triangle_forces() {
        if (K.max_dim() < 2) return;
        for (const auto& s : K.get_simplices(2)) {
            auto a = s.v[0], b = s.v[1], c = s.v[2];

            Vec3 ab = pos[b] - pos[a];
            Vec3 ac = pos[c] - pos[a];

            Vec3 normal = glm::cross(ab, ac);
            float area = glm::length(normal) + 1e-6f;

            Vec3 dir = normal / area;
            force[a] += k_area * dir;
            force[b] += k_area * dir;
            force[c] += k_area * dir;
        }
    }

    void apply_tetra_forces() {
        if (K.max_dim() < 3) return;

        for (const auto& s : K.get_simplices(3)) {
            auto a = s.v[0], b = s.v[1], c = s.v[2], d = s.v[3];

            Vec3 ab = pos[b] - pos[a];
            Vec3 ac = pos[c] - pos[a];
            Vec3 ad = pos[d] - pos[a];

            float vol = glm::dot(glm::cross(ab, ac), ad);

            float sign = (vol >= 0) ? 1.0f : -1.0f;

            Vec3 grad = sign * glm::cross(ac, ad);

            force[a] -= k_vol * grad;
            force[b] += k_vol * grad;
            force[c] += k_vol * grad;
            force[d] += k_vol * grad;
        }
    }

    void integrate(float step) {
        for (auto& [v, p] : pos) {
            p += step * force[v];
        }
    }
public:
    explicit Embedding(const SimplicialComplex& K) : K(K) {}

    void initialize_random(float scale = 1.0f, int seed = 53) {
        std::mt19937 mt(seed);
        std::uniform_real_distribution<float> dist(-scale, scale);

        for (auto v : K.get_vertices()) {
            pos[v] = Vec3(dist(mt), dist(mt), dist(mt));
        }
    }

    void step(float temp) {
        compute_forces();
        integrate(temp);
    }

    void run(int iterations, float step) {
        for (int i = 0; i < iterations; i++) {
            compute_forces();
            integrate(step);
            step *= 0.99f;
        }
    }

    const Vec3& get(int v) const {
        return pos.at(v);
    }

    const std::map<int, Vec3>& get_positions() const {
        return pos;
    }
};