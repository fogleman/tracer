#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

#include "config.hpp"
#include "hit.hpp"
#include "ray.hpp"

class Cube : public Hittable {
public:
    Cube(const vec3 &min, const vec3 &max, const P_Material &material) :
        m_Min(min), m_Max(max), m_Material(material) {}

    virtual bool Hit(
        const Ray &ray, const real tmin, const real tmax, HitInfo &hit) const
    {
        const vec3 n = (m_Min - ray.Origin()) / ray.Direction();
        const vec3 f = (m_Max - ray.Origin()) / ray.Direction();
        const vec3 min = glm::min(n, f);
        const vec3 max = glm::max(n, f);
        const real t0 = glm::compMax(min);
        const real t1 = glm::compMin(max);
        if (t0 > t1) {
            return false;
        }
        real t;
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

    vec3 NormalAt(const vec3 &p) const {
        if (p.x < m_Min.x + EPS) {
            return vec3(-1, 0, 0);
        }
        if (p.x > m_Max.x - EPS) {
            return vec3(1, 0, 0);
        }
        if (p.y < m_Min.y + EPS) {
            return vec3(0, -1, 0);
        }
        if (p.y > m_Max.y - EPS) {
            return vec3(0, 1, 0);
        }
        if (p.z < m_Min.z + EPS) {
            return vec3(0, 0, -1);
        }
        if (p.z > m_Max.z - EPS) {
            return vec3(0, 0, 1);
        }
        return vec3(0, 1, 0);
    }

private:
    vec3 m_Min;
    vec3 m_Max;
    P_Material m_Material;
};
