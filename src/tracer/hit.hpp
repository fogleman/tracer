#pragma once

#include <memory>
#include <vector>

#include "config.hpp"
#include "material.hpp"
#include "ray.hpp"

struct HitInfo {
    real T;
    vec3 Position;
    vec3 Normal;
    P_Material Material;
};

class Hittable {
public:
    virtual bool Hit(
        const Ray &ray, const real tmin, const real tmax, HitInfo &hit) const = 0;

    virtual Ray RandomRay(const vec3 &o) const {
        return Ray();
    }

    virtual real Pdf(const Ray &ray) const {
        return 0;
    }

    virtual bool Emits() const {
        return false;
    }

    virtual ~Hittable() {}
};

typedef std::shared_ptr<Hittable> P_Hittable;

class HittableList : public Hittable {
public:
    void Add(const P_Hittable &item) {
        m_Items.push_back(item);
        if (item->Emits()) {
            m_Lights.push_back(item);
        }
    }

    const std::vector<P_Hittable> &Lights() const {
        return m_Lights;
    }

    virtual bool Hit(
        const Ray &ray, const real tmin, const real tmax, HitInfo &hit) const
    {
        bool result = false;
        real closest = tmax;
        for (const auto &item : m_Items) {
            HitInfo temp;
            if (item->Hit(ray, tmin, closest, temp)) {
                result = true;
                closest = temp.T;
                hit = temp;
            }
        }
        return result;
    }

private:
    std::vector<P_Hittable> m_Items;
    std::vector<P_Hittable> m_Lights;
};

typedef std::shared_ptr<HittableList> P_HittableList;
