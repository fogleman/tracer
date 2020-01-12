#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <memory>

#include "config.hpp"
#include "util.hpp"

class MicrofacetDistribution {
public:
    virtual real D(const vec3 &wh) const = 0;
    virtual real Pdf(const vec3 &wo, const vec3 &wi) const = 0;
    virtual void Sample_f(
        const vec3 &p, const vec3 &wo, vec3 &wi, real &pdf) const = 0;
    virtual ~MicrofacetDistribution() {}
};

typedef std::shared_ptr<MicrofacetDistribution> P_MicrofacetDistribution;

class BlinnDistribution : public MicrofacetDistribution {
public:
    BlinnDistribution(const real exponent) :
        m_Exponent(exponent) {}

    virtual real D(const vec3 &wh) const {
        return (m_Exponent + 2) * std::pow(AbsCosTheta(wh), m_Exponent) / PI;
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        const vec3 wh = glm::normalize(wo + wi);
        if (glm::dot(wo, wh) <= 0) {
            return 0;
        }
        const real costheta = AbsCosTheta(wh);
        return ((m_Exponent + 1) * std::pow(costheta, m_Exponent)) /
            (2 * PI * 4 * glm::dot(wo, wh));
    }

    virtual void Sample_f(
        const vec3 &p, const vec3 &wo, vec3 &wi, real &pdf) const
    {
        const real costheta = std::pow(Random(), 1 / (m_Exponent + 1));
        const real sintheta = std::sqrt(
            std::max(real(0), 1 - costheta * costheta));
        const real phi = Random() * 2 * PI;
        vec3 wh = vec3(
            sintheta * std::cos(phi), sintheta * std::sin(phi), costheta);
        if (wh.z * wo.z < 0) {
            wh = -wh;
        }
        wi = glm::normalize(-wo + 2 * glm::dot(wo, wh) * wh);
        pdf = Pdf(wo, wi);
    }

private:
    real m_Exponent;
};
