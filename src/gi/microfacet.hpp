#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <memory>

#include "constants.hpp"
#include "util.hpp"

class MicrofacetDistribution {
public:
    virtual float D(const glm::vec3 &wh) const = 0;
    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const = 0;
    virtual void Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo, glm::vec3 &wi, float &pdf) const = 0;
    virtual ~MicrofacetDistribution() {}
};

typedef std::shared_ptr<MicrofacetDistribution> P_MicrofacetDistribution;

class BlinnDistribution : public MicrofacetDistribution {
public:
    BlinnDistribution(const float exponent) :
        m_Exponent(exponent) {}

    virtual float D(const glm::vec3 &wh) const {
        return (m_Exponent + 2) * std::pow(AbsCosTheta(wh), m_Exponent) / PI;
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        const glm::vec3 wh = glm::normalize(wo + wi);
        if (glm::dot(wo, wh) <= 0) {
            return 0;
        }
        const float costheta = AbsCosTheta(wh);
        return ((m_Exponent + 1) * std::pow(costheta, m_Exponent)) /
            (2 * PI * 4 * glm::dot(wo, wh));
    }

    virtual void Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo, glm::vec3 &wi, float &pdf) const
    {
        const float costheta = std::pow(Random(), 1 / (m_Exponent + 1));
        const float sintheta = std::sqrt(
            std::max(float(0), 1 - costheta * costheta));
        const float phi = Random() * 2 * PI;
        glm::vec3 wh = glm::vec3(
            sintheta * std::cos(phi), sintheta * std::sin(phi), costheta);
        if (wh.z * wo.z < 0) {
            wh = -wh;
        }
        wi = glm::normalize(-wo + 2 * glm::dot(wo, wh) * wh);
        pdf = Pdf(wo, wi);
    }

private:
    float m_Exponent;
};
