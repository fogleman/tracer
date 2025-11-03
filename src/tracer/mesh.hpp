#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/normal.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include "box.hpp"
#include "config.hpp"

class Mesh {
public:
    Mesh(const std::vector<vec3> &data) {
        // deduplicate vertices
        std::unordered_map<vec3, int> lookup;
        for (const vec3 &v : data) {
            if (lookup.find(v) == lookup.end()) {
                lookup[v] = m_Positions.size();
                m_Positions.push_back(v);
            }
        }

        // create triangles
        for (int i = 0; i < data.size(); i += 3) {
            const int i0 = lookup[data[i + 0]];
            const int i1 = lookup[data[i + 1]];
            const int i2 = lookup[data[i + 2]];
            m_Triangles.emplace_back(i0, i1, i2);
        }
    }

    const std::vector<vec3> &Positions() const {
        return m_Positions;
    }

    const std::vector<vec3> &Normals() const {
        return m_Normals;
    }

    const std::vector<glm::ivec3> &Triangles() const {
        return m_Triangles;
    }

    void SmoothNormals() {
        m_Normals.resize(m_Positions.size(), vec3(0));
        for (const auto &t : m_Triangles) {
            const vec3 v1 = m_Positions[t.x];
            const vec3 v2 = m_Positions[t.y];
            const vec3 v3 = m_Positions[t.z];
            const vec3 n = glm::triangleNormal(v1, v2, v3);
            m_Normals[t.x] += n;
            m_Normals[t.y] += n;
            m_Normals[t.z] += n;
        }
        for (int i = 0; i < m_Normals.size(); i++) {
            m_Normals[i] = glm::normalize(m_Normals[i]);
        }
    }

    // TODO: cache bounding box?
    Box BoundingBox() const {
        if (m_Positions.empty()) {
            return Box();
        }
        vec3 min = m_Positions[0];
        vec3 max = m_Positions[0];
        for (const vec3 &v : m_Positions) {
            min = glm::min(min, v);
            max = glm::max(max, v);
        }
        return Box(min, max);
    }

    vec3 TriangleNormalAt(const int index, const vec3 &position) const {
        const auto t = m_Triangles[index];
        const vec3 v1 = m_Positions[t.x];
        const vec3 v2 = m_Positions[t.y];
        const vec3 v3 = m_Positions[t.z];
        if (m_Normals.empty()) {
            return glm::triangleNormal(v1, v2, v3);
        }
        const vec3 b = Barycentric(v1, v2, v3, position);
        const vec3 n1 = m_Normals[t.x];
        const vec3 n2 = m_Normals[t.y];
        const vec3 n3 = m_Normals[t.z];
        return n1 * b.x + n2 * b.y + n3 * b.z;
    }

    void Transform(const mat4 &m) {
        for (int i = 0; i < m_Positions.size(); i++) {
            m_Positions[i] = vec3(m * vec4(m_Positions[i], real(1)));
        }
        for (int i = 0; i < m_Normals.size(); i++) {
            m_Normals[i] = vec3(m * vec4(m_Normals[i], real(0)));
        }
    }

    mat4 MoveTo(const vec3 &position, const vec3 &anchor) {
        const Box box = BoundingBox();
        const vec3 d = position - box.Anchor(anchor);
        const mat4 m = glm::translate(mat4(1), d);
        Transform(m);
        return m;
    }

    mat4 Center() {
        return MoveTo(vec3(0), vec3(0.5));
    }

    mat4 FitInside(const Box &box, const vec3 &anchor) {
        const Box boundingBox = BoundingBox();
        const real scale = glm::compMin(box.Size() / boundingBox.Size());
        const vec3 extra = box.Size() - boundingBox.Size() * scale;
        mat4 m(1);
        m = glm::translate(m, box.Min() + extra * anchor);
        m = glm::scale(m, vec3(scale));
        m = glm::translate(m, -boundingBox.Min());
        Transform(m);
        return m;
    }

    mat4 FitInUnitCube() {
        const real r = 0.5;
        Box box(vec3(-r, -r, -r), vec3(r, r, r));
        return FitInside(box, vec3(0.5));
    }

    mat4 FitInBiUnitCube() {
        const real r = 1;
        Box box(vec3(-r, -r, -r), vec3(r, r, r));
        return FitInside(box, vec3(0.5));
    }

    void Rotate(const real radians, const vec3 &axis) {
        Transform(glm::rotate(mat4(1), radians, axis));
    }

private:
    std::vector<vec3> m_Positions;
    std::vector<vec3> m_Normals;
    std::vector<glm::ivec3> m_Triangles;
};

typedef std::shared_ptr<Mesh> P_Mesh;
