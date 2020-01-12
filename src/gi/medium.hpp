#pragma once

#include <glm/glm.hpp>

#include "config.hpp"
#include "hit.hpp"

class ConstantMedium : public Hittable {
public:
    ConstantMedium(
        const P_Hittable &boundary, const P_Texture &texture,
        const real density) :
    m_Boundary(boundary),
    m_Material(std::make_shared<Isotropic>(texture)),
    m_Density(density) {}

    virtual bool Hit(
        const Ray &ray, const real tmin, const real tmax, HitInfo &hit) const
    {
        HitInfo boundaryHit;
        if (m_Boundary->Hit(ray, EPS, INF, boundaryHit)) {
            if (glm::dot(ray.Direction(), boundaryHit.Normal) > 0) {
                const real distanceInsideBoundary = boundaryHit.T * glm::length(ray.Direction());
                const real hitDistance = -std::log(Random()) / m_Density;
                if (hitDistance < distanceInsideBoundary) {
                    hit.T = hitDistance / glm::length(ray.Direction());
                    hit.Position = ray.At(hit.T);
                    hit.Normal = vec3(0, 1, 0);
                    hit.Material = m_Material;
                    return true;
                }
            } else {
                const real t = boundaryHit.T;
                if (m_Boundary->Hit(ray, t + EPS, INF, boundaryHit)) {
                    if (glm::dot(ray.Direction(), boundaryHit.Normal) > 0) {
                        const real distanceInsideBoundary = (boundaryHit.T - t) * glm::length(ray.Direction());
                        const real hitDistance = -std::log(Random()) / m_Density;
                        if (hitDistance < distanceInsideBoundary) {
                            hit.T = hitDistance / glm::length(ray.Direction());
                            hit.Position = ray.At(hit.T);
                            hit.Normal = glm::normalize(RandomInUnitSphere());
                            hit.Material = m_Material;
                            return true;
                        }
                    }
                }
            }
        }
        // HitInfo hit1, hit2;
        // if (m_Boundary->Hit(ray, -INF, INF, hit1)) {
        //     if (m_Boundary->Hit(ray, hit1.T + EPS, INF, hit2)) {
        //         if (hit1.T < tmin) {
        //             hit1.T = tmin;
        //         }
        //         if (hit2.T > tmax) {
        //             hit2.T = tmax;
        //         }
        //         if (hit1.T >= hit2.T) {
        //             return false;
        //         }
        //         if (hit1.T < 0) {
        //             hit1.T = 0;
        //         }
        //         const real distanceInsideBoundary = (hit2.T - hit1.T) * ray.Direction().Length();
        //         const real hitDistance = -std::log(Random()) / m_Density;
        //         if (hitDistance < distanceInsideBoundary) {
        //             hit.T = hit1.T + hitDistance / ray.Direction().Length();
        //             hit.Position = ray.At(hit.T);
        //             hit.Normal = RandomInUnitSphere();
        //             hit.Material = m_Material;
        //             return true;
        //         }
        //     }
        // }
        return false;
    }

private:
    P_Hittable m_Boundary;
    P_Material m_Material;
    real m_Density;
};
