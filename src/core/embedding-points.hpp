#pragma once
#include "simplicial_complex.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <random>
#include <cmath>
using Vec3 = glm::vec3;

class Embedding {
 private:
    const SimplicialComplex& K;

    std::map<int, Vec3> pos;
    std::map<int, Vec3> force;

    float k_rep  = 0.05f;
    float k_edge = 0.1f;
    float k_area = 0.05f;
    float k_vol  = 0.05f;
    float k_face = 0.1f;
    std::mt19937 mt{42};

    // --------------- calculate energies for a complex ---------------

    float edge_energy() const {
        float energy = 0.0f;
        for (const auto& s : K.get_simplices(1)) {
            auto u = s.v[0], v = s.v[1];
            float dist = glm::length(pos.at(u) - pos.at(v));
            float diff = dist - 1.0f;
            energy += 0.5f * k_edge * diff * diff;
        }
        return energy;
    }

    float repulsion_energy() const {
        float energy = 0.0f;
        for (auto v1 : K.get_vertices()) {
            for (auto v2 : K.get_vertices()) {
                if (v1 >= v2) continue;
                float dist = glm::length(pos.at(v1) - pos.at(v2)) + 1e-5f;
                energy += k_rep / dist;
            }
        }
        return energy;
    }

    float triangle_energy() const {
        if (K.max_dim() < 2) return 0.0f;
        float energy = 0.0f;
        for (const auto& s : K.get_simplices(2)) {
            auto a = s.v[0], b = s.v[1], c = s.v[2];
            Vec3 ab = pos.at(b) - pos.at(a);
            Vec3 ac = pos.at(c) - pos.at(a);
            float area2 = glm::length(glm::cross(ab, ac));
            energy += k_area / (area2 + 1e-5f);
        }
        return energy;
    }

    float tetra_energy() const {
        if (K.max_dim() < 3) return 0.0f;
        float energy = 0.0f;
        for (const auto& s : K.get_simplices(3)) {
            auto a = s.v[0], b = s.v[1], c = s.v[2], d = s.v[3];
            Vec3 ab = pos.at(b) - pos.at(a);
            Vec3 ac = pos.at(c) - pos.at(a);
            Vec3 ad = pos.at(d) - pos.at(a);
            float vol6 = std::abs(glm::dot(glm::cross(ab, ac), ad));
            energy += k_vol / (vol6 + 1e-5f);
        }
        return energy;
    }

    float total_energy() const {
        return repulsion_energy() + edge_energy() + triangle_energy() + tetra_energy();
    }
    // --------------- calculate forces for a complex ---------------
    void compute_forces() {
        for (auto& [v, f] : force) f = Vec3(0.0f);
        apply_repulsion();
        apply_edge_forces();
        apply_triangle_forces();
        apply_two_triangle_forces();
        apply_two_segment_forces();
        apply_tetra_forces();
    }

    void apply_repulsion() {
        for (auto v1 : K.get_vertices()) {
            for (auto v2 : K.get_vertices()) {
                if (v1 >= v2) continue;

                Vec3 d = pos[v1] - pos[v2];
                float dist = glm::length(d) + 1e-5f;

                Vec3 f = k_rep * d / (dist * dist);

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

            Vec3 n = glm::cross(ab, ac);
            float area = glm::length(n) + 1e-6f;

            Vec3 grad_a = glm::cross(pos[c] - pos[b], n) / area;
            Vec3 grad_b = glm::cross(pos[a] - pos[c], n) / area;
            Vec3 grad_c = glm::cross(pos[b] - pos[a], n) / area;

            force[a] += k_area * grad_a;
            force[b] += k_area * grad_b;
            force[c] += k_area * grad_c;
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

    void apply_two_triangle_forces() {
        if (K.max_dim() < 2) return;

        for (const auto& s : K.get_simplices(2)) {
            for (const auto& t : K.get_simplices(2)) {
                if (s == t) continue;

                auto a = s.v[0], b = s.v[1], c = s.v[2];
                auto d = t.v[0], e = t.v[1], f = t.v[2];
                Vec3 cs = (pos[a] + pos[b] + pos[c]) / 3.0f;
                Vec3 ct = (pos[d] + pos[e] + pos[f]) / 3.0f;
                Vec3 diff = cs - ct;
                float dist = glm::length(diff) + 1e-4f;

                float cutoff = 1.5f;
                if (dist > cutoff) continue;

                Vec3 dir = diff / dist;
                float strength = k_face * (1.0f / (dist * dist));

                Vec3 force_applied = strength * dir;

                force[a] += force_applied / 3.0f;
                force[b] += force_applied / 3.0f;
                force[c] += force_applied / 3.0f;

                force[d] -= force_applied / 3.0f;
                force[e] -= force_applied / 3.0f;
                force[f] -= force_applied / 3.0f;
            }
        }
    }

    void apply_two_segment_forces() {
        if (K.max_dim() < 1) return;

        for (const auto& s : K.get_simplices(1)) {
            for (const auto& t : K.get_simplices(1)) {
                if (s == t) continue;

                auto a = s.v[0], b = s.v[1];
                auto c = t.v[0], d = t.v[1];
                Vec3 cs = (pos[a] + pos[b]) / 2.0f;
                Vec3 ct = (pos[c] + pos[d]) / 2.0f;
                Vec3 diff = cs - ct;
                float dist = glm::length(diff) + 1e-4f;

                float cutoff = 1.0f;
                if (dist > cutoff) continue;

                Vec3 dir = diff / dist;
                float strength = k_edge * (1.0f / (dist * dist));

                Vec3 force_applied = strength * dir;

                force[a] += force_applied / 2.0f;
                force[b] += force_applied / 2.0f;

                force[c] -= force_applied / 2.0f;
                force[d] -= force_applied / 2.0f;
            }
        }
    }
    // apply forces
    void integrate(float step) {
        for (auto& [v, p] : pos) {
            p += step * force[v];
        }
    }
public:
    explicit Embedding(const SimplicialComplex& K) : K(K) {}

    // initialize positions
    void initialize_random(float scale = 1.0f, int seed = 53) {
        mt.seed(seed);
        std::uniform_real_distribution<float> dist(-scale, scale);

        for (auto v : K.get_vertices()) {
            pos[v] = Vec3(dist(mt), dist(mt), dist(mt));
        }
    }
    // annealing step
    void step(float temp) {
        if (temp <= 1e-6f) return;
        // random shift
        std::normal_distribution<float> perturb(0.0f, temp);
        auto old_pos = pos;
        float old_energy = total_energy();
        // force apply
        compute_forces();

        for (auto& [v, p] : pos) {
            Vec3 random_shift(perturb(mt), perturb(mt), perturb(mt));
            Vec3 force_shift = temp * force[v];
            p += force_shift + random_shift;
        }

        float new_energy = total_energy();
        float dE = new_energy - old_energy;

        if (dE <= 0.0f) return;

        std::uniform_real_distribution<float> coin(0.0f, 1.0f);
        float accept_prob = std::exp(-dE / temp);
        if (coin(mt) > accept_prob) {
            pos = old_pos;
        }
    }

    const Vec3& get(int v) const {
        return pos.at(v);
    }

    const std::map<int, Vec3>& get_positions() const {
        return pos;
    }
};