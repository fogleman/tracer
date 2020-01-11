#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <memory>

#include "constants.hpp"
#include "hit.hpp"
#include "ray.hpp"
#include "util.hpp"

class Sampler {
public:
    Sampler(const P_HittableList &world) :
        m_World(world),
        m_MinBounces(8),
        m_MaxBounces(64)
    {}

    glm::vec3 Background(const Ray &ray) const {
        return glm::vec3(0);
    }

    glm::vec3 Sample(const Ray &cameraRay) const {
        glm::vec3 color(0, 0, 0);
        glm::vec3 throughput(1, 1, 1);
        bool specular = true;
        Ray ray(cameraRay);

        // TODO: handle zero or many lights
        const P_Hittable light = m_World->Lights()[0];

        for (int bounces = 0; bounces < m_MaxBounces; bounces++) {
            HitInfo hit;
            if (!m_World->Hit(ray, EPS, INF, hit)) {
                color = color + throughput * Background(ray);
                break;
            }

            const glm::vec3 emitted = hit.Material->Emitted(0, 0, hit.Position);
            if (glm::compMax(emitted) > 0) {
                if (specular && glm::dot(hit.Normal, ray.Direction()) < 0) {
                    color = color + throughput * emitted;
                }
                break;
            }

            const ONB onb(hit.Normal);
            const glm::vec3 p(hit.Position);
            const glm::vec3 wo(onb.WorldToLocal(glm::normalize(-ray.Direction())));

            glm::vec3 wi;
            float pdf;
            const glm::vec3 a = hit.Material->Sample_f(p, wo, wi, pdf, specular);

            // direct lighting
            if (!specular) {
                const Ray lightRay = light->RandomRay(p);
                HitInfo lightHit;
                if (m_World->Hit(lightRay, EPS, INF, lightHit)) {
                    const glm::vec3 Li = lightHit.Material->Emitted(0, 0, lightHit.Position);
                    if (glm::compMax(Li) > 0 && glm::dot(lightHit.Normal, lightRay.Direction()) < 0) {
                        const float lightPdf = light->Pdf(lightRay);
                        const glm::vec3 lwi = onb.WorldToLocal(lightRay.Direction());
                        const glm::vec3 direct = hit.Material->f(p, wo, lwi) * Li / lightPdf;
                        color = color + throughput * direct * std::abs(lwi.z);
                    }
                }
            }

            // glm::vec3 a;
            // glm::vec3 wi;
            // float pdf;
            // if (Random() < 0.5) {
            //     // use light pdf
            //     const Ray lightRay = light->RandomRay(p);
            //     pdf = light->Pdf(lightRay);
            //     wi = onb.WorldToLocal(lightRay.Direction());
            //     a = hit.Material->f(p, wo, wi);
            //     pdf = (pdf + hit.Material->Pdf(wo, wi)) / 2;
            //     specular = false;
            // } else {
            //     // use material pdf
            //     a = hit.Material->Sample_f(p, wo, wi, pdf, specular);
            //     pdf = (pdf + light->Pdf(Ray(p, onb.LocalToWorld(wi)))) / 2;
            // }

            if (specular) {
                throughput = throughput * a;
            } else {
                if (pdf < EPS) {
                    break;
                }
                throughput = throughput * a * std::abs(wi.z) / pdf;
            }

            ray = Ray(p, onb.LocalToWorld(wi));

            if (bounces >= m_MinBounces) {
                const float prob = glm::compMax(throughput);
                if (Random() > prob) {
                    break;
                }
                throughput = throughput / prob;
            }
        }

        return color;
    }

private:
    P_HittableList m_World;
    int m_MinBounces;
    int m_MaxBounces;
};

typedef std::shared_ptr<Sampler> P_Sampler;
