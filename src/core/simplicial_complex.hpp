#pragma once
#include <vector>
#include <array>
#include <set>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <stdexcept>


struct Simplex {
    std::vector<int> v;

    Simplex(std::vector<int> vertices) {
        std::sort(vertices.begin(), vertices.end());
        vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
        v = vertices;
    }

    int dimension() {
        return v.size() - 1;
    }

    bool operator==(const Simplex& other) const {
        return v == other.v;
    }
};

struct SimplexHash {
    std::size_t operator()(const Simplex& s) const {
        size_t h = 0;
        for (auto x : s.v) {
            h ^= std::hash<int>()(x) + 4435769 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

class SimplicialComplex {
 private:
    std::set<int> vertices;
    std::unordered_set<Simplex, SimplexHash> all_simplices;
    std::map<int, std::vector<Simplex>> simplices_by_dimension;

    int max_dimension = 0;

    void add_simplex_recursive(Simplex s) {
        if (all_simplices.count(s)) return;
        all_simplices.insert(s);
        simplices_by_dimension[s.dimension()].push_back(s);
        max_dimension = std::max(max_dimension, s.dimension());
        for (int i = 0; i < (int)s.v.size(); i++) {
            std::vector<int> face = s.v;
            face.erase(face.begin() + i);
            if (!face.empty()) {
                add_simplex_recursive(Simplex(face));
            }
        }
    }
 public:
    void add_vertex(int id) {
        vertices.insert(id);
    }

    void add_simplex(const std::vector<int>& verts) {
        if (verts.empty() || verts.size() > 4) throw std::invalid_argument("Simplex must have size 1..4");

        Simplex s(verts);

        for (int x : s.v) {
            vertices.insert(x);
        }

        add_simplex_recursive(s);
    }

    const std::vector<Simplex>& get_simplices(int d) const {
        return simplices_by_dimension.at(d);
    }

    int max_dim() const {
        return max_dimension;
    }

    const std::set<int>& get_vertices() const {
        return vertices;
    }
};