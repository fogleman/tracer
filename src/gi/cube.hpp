#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

#include "constants.hpp"
#include "hit.hpp"
#include "ray.hpp"

class Cube : public Hittable {
public:
    Cube(const glm::vec3 &min, const glm::vec3 &max, const P_Material &material) :
        m_Min(min), m_Max(max), m_Material(material) {}

    virtual bool Hit(
        const Ray &ray, const float tmin, const float tmax, HitInfo &hit) const
    {
        const glm::vec3 n = (m_Min - ray.Origin()) / ray.Direction();
        const glm::vec3 f = (m_Max - ray.Origin()) / ray.Direction();
        const glm::vec3 min = glm::min(n, f);
        const glm::vec3 max = glm::max(n, f);
        const float t0 = glm::compMax(min);
        const float t1 = glm::compMin(max);
        if (t0 > t1) {
            return false;
        }
        float t;
        if (t0 < tmax && t0 > tmin) {
            t = t0;
        } else if (t1 < tmax && t1 > tmin) {
            t = t1;
        } else {
            return false;
        }
        hit.T = t;
        hit.Position = ray.At(hit.T);
        hit.Normal = NormalAt(hit.Position);
        hit.Material = m_Material;
        return true;
    }

    glm::vec3 NormalAt(const glm::vec3 &p) const {
        if (p.x < m_Min.x + EPS) {
            return glm::vec3(-1, 0, 0);
        }
        if (p.x > m_Max.x - EPS) {
            return glm::vec3(1, 0, 0);
        }
        if (p.y < m_Min.y + EPS) {
            return glm::vec3(0, -1, 0);
        }
        if (p.y > m_Max.y - EPS) {
            return glm::vec3(0, 1, 0);
        }
        if (p.z < m_Min.z + EPS) {
            return glm::vec3(0, 0, -1);
        }
        if (p.z > m_Max.z - EPS) {
            return glm::vec3(0, 0, 1);
        }
        return glm::vec3(0, 1, 0);
    }

private:
    glm::vec3 m_Min;
    glm::vec3 m_Max;
    P_Material m_Material;
};
