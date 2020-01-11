#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#include "constants.hpp"
#include "microfacet.hpp"
#include "ray.hpp"
#include "texture.hpp"
#include "util.hpp"

class Material {
public:
    virtual glm::vec3 f(
        const glm::vec3 &p,
        const glm::vec3 &wo, const glm::vec3 &wi) const = 0;

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        // wi = glm::normalize(RandomInUnitSphere());
        // if (wo.z * wi.z <= 0) {
        //     wi = glm::vec3(wi.x, wi.y, -wi.z);
        // }
        // pdf = 0.5 / PI;
        wi = CosineSampleHemisphere();
        if (wo.z < 0) {
            wi = glm::vec3(wi.x, wi.y, -wi.z);
        }
        pdf = Pdf(wo, wi);
        specular = false;
        return f(p, wo, wi);
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        if (wo.z * wi.z <= 0) {
            return 0;
        }
        return std::abs(wi.z) / PI;
    }

    virtual glm::vec3 Emitted(
        const float u, const float v, const glm::vec3 &p) const
    {
        return glm::vec3();
    }

    virtual bool Emits() const {
        return false;
    }

    virtual ~Material() {}
};

typedef std::shared_ptr<Material> P_Material;

class Isotropic : public Material {
public:
    Isotropic(const P_Texture &albedo) :
        m_Albedo(albedo) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        return glm::vec3();
        // return m_Albedo->Sample(0, 0, p) / (2 * PI);
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        wi = glm::normalize(RandomInUnitSphere());
        pdf = 1;
        specular = true;
        return m_Albedo->Sample(0, 0, p);
        // pdf = Pdf(wo, wi);
        // specular = false;
        // return f(p, wo, wi);
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        return 0;
        // return 0.25 / PI;
    }

private:
    P_Texture m_Albedo;
};

inline float G(const glm::vec3 &wo, const glm::vec3 &wi, const glm::vec3 &wh) {
    const float NdotWh = AbsCosTheta(wh);
    const float NdotWo = AbsCosTheta(wo);
    const float NdotWi = AbsCosTheta(wi);
    const float WOdotWh = std::abs(glm::dot(wo, wh));
    return std::min(float(1), std::min(
        2 * NdotWh * NdotWo / WOdotWh,
        2 * NdotWh * NdotWi / WOdotWh));
}

class FresnelBlend : public Material {
public:
    FresnelBlend(
        const P_Texture &Rd, const P_Texture &Rs,
        const P_MicrofacetDistribution &distribution) :
    m_Rd(Rd), m_Rs(Rs), m_Distribution(distribution) {}

    glm::vec3 SchlickFresnel(const glm::vec3 &rs, const float costheta) const {
        return rs + std::pow(1 - costheta, 5.f) * (glm::vec3(1) - rs);
    }

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        const glm::vec3 rd = m_Rd->Sample(0, 0, p);
        const glm::vec3 rs = m_Rs->Sample(0, 0, p);
        const glm::vec3 diffuse = float(28 / (23 * PI)) * rd *
            (glm::vec3(1) - rs) *
            (1 - std::powf(1 - 0.5 * AbsCosTheta(wi), 5)) *
            (1 - std::powf(1 - 0.5 * AbsCosTheta(wo), 5));
        const glm::vec3 wh = glm::normalize(wi + wo);
        const glm::vec3 specular = m_Distribution->D(wh) /
            (4 * std::abs(glm::dot(wi, wh)) *
            std::max(AbsCosTheta(wi), AbsCosTheta(wo))) *
            SchlickFresnel(rs, glm::dot(wi, wh));
        return diffuse + specular;
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        if (!SameHemisphere(wo, wi)) {
            return 0;
        }
        return 0.5 * (AbsCosTheta(wi) / PI + m_Distribution->Pdf(wo, wi));
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        if (Random() < 0.5) {
            wi = CosineSampleHemisphere();
            if (wo.z < 0) {
                wi = glm::vec3(wi.x, wi.y, -wi.z);
            }
        } else {
            m_Distribution->Sample_f(p, wo, wi, pdf);
            if (!SameHemisphere(wo, wi)) {
                return glm::vec3();
            }
        }
        pdf = Pdf(wo, wi);
        specular = false;
        return f(p, wo, wi);
    }

private:
    P_Texture m_Rd;
    P_Texture m_Rs;
    P_MicrofacetDistribution m_Distribution;
};

class Microfacet : public Material {
public:
    Microfacet(
        const P_Texture &albedo, const P_MicrofacetDistribution &distribution,
        const float eta) :
    m_Albedo(albedo), m_Distribution(distribution), m_Eta(eta) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        const float cosThetaO = AbsCosTheta(wo);
        const float cosThetaI = AbsCosTheta(wi);
        if (cosThetaO == 0 || cosThetaI == 0) {
            return glm::vec3();
        }
        const glm::vec3 wh = glm::normalize(wi + wo);
        const float cosThetaH = glm::dot(wi, wh);
        const float F = Schlick(cosThetaH, m_Eta);
        const glm::vec3 R = m_Albedo->Sample(0, 0, p);
        return R * m_Distribution->D(wh) * G(wo, wi, wh) * F /
            (4 * cosThetaI * cosThetaO);
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        if (!SameHemisphere(wo, wi)) {
            return 0;
        }
        return m_Distribution->Pdf(wo, wi);
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        m_Distribution->Sample_f(p, wo, wi, pdf);
        if (!SameHemisphere(wo, wi)) {
            return glm::vec3();
        }
        specular = false;
        return f(p, wo, wi);
    }

private:
    P_Texture m_Albedo;
    P_MicrofacetDistribution m_Distribution;
    float m_Eta;
};

class Metal : public Material {
public:
    Metal(const P_Texture &albedo) :
        m_Albedo(albedo) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        return glm::vec3(0);
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        return 0;
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        wi = glm::vec3(-wo.x, -wo.y, wo.z);
        pdf = 1;
        specular = true;
        return m_Albedo->Sample(0, 0, p);
    }

private:
    P_Texture m_Albedo;
};

class Dielectric : public Material {
public:
    Dielectric(const P_Texture &albedo, const float eta) :
        m_Albedo(albedo), m_Eta(eta) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        return glm::vec3(0);
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        return 0;
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        glm::vec3 outwardNormal;
        float ratio;
        if (wo.z < 0) {
            outwardNormal = glm::vec3(0, 0, -1);
            ratio = m_Eta;
        } else {
            outwardNormal = glm::vec3(0, 0, 1);
            ratio = 1 / m_Eta;
        }

        glm::vec3 refracted;
        float reflectProbability;
        if (Refract(-wo, outwardNormal, ratio, refracted)) {
            reflectProbability = Schlick(AbsCosTheta(wo), m_Eta);
        } else {
            reflectProbability = 1;
        }

        if (Random() < reflectProbability) {
            wi = glm::vec3(-wo.x, -wo.y, wo.z);
        } else {
            wi = refracted;
        }

        pdf = 1;
        specular = true;
        return m_Albedo->Sample(0, 0, p);
    }

private:
    P_Texture m_Albedo;
    float m_Eta;
};

class SpecularReflection : public Material {
public:
    SpecularReflection(const P_Texture &albedo, const float eta) :
        m_Albedo(albedo), m_Eta(eta) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        return glm::vec3();
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        wi = glm::vec3(-wo.x, -wo.y, wo.z);
        pdf = 1;
        specular = true;
        const float fr = Schlick(AbsCosTheta(wo), m_Eta);
        return m_Albedo->Sample(0, 0, p) * fr;
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        return 0;
    }

private:
    P_Texture m_Albedo;
    float m_Eta;
};

class Lambertian : public Material {
public:
    Lambertian(const P_Texture &albedo) :
        m_Albedo(albedo) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        return m_Albedo->Sample(0, 0, p) / PI;
    }

private:
    P_Texture m_Albedo;
};

class OrenNayar : public Material {
public:
    OrenNayar(const P_Texture &albedo, const float sigma_degrees) :
        m_Albedo(albedo)
    {
        const float sigma = sigma_degrees * PI / 180;
        const float sigma2 = sigma * sigma;
        m_A = 1 - (sigma2 / (2 * (sigma2 + 0.33)));
        m_B = 0.45 * sigma2 / (sigma2 + 0.09);
    }

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        const float sinthetai = SinTheta(wi);
        const float sinthetao = SinTheta(wo);
        float maxcos = 0;
        if (sinthetai > EPS && sinthetao > EPS) {
            const float sinphii = SinPhi(wi);
            const float cosphii = CosPhi(wi);
            const float sinphio = SinPhi(wo);
            const float cosphio = CosPhi(wo);
            const float dcos = cosphii * cosphio + sinphii * sinphio;
            maxcos = std::max(float(0), dcos);
        }
        float sinalpha, tanbeta;
        if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
            sinalpha = sinthetao;
            tanbeta = sinthetai / AbsCosTheta(wi);
        } else {
            sinalpha = sinthetai;
            tanbeta = sinthetao / AbsCosTheta(wo);
        }
        return m_Albedo->Sample(0, 0, p) *
            (m_A + m_B * maxcos * sinalpha * tanbeta) / PI;
    }

private:
    P_Texture m_Albedo;
    float m_A, m_B;
};

class DiffuseLight : public Material {
public:
    DiffuseLight(const P_Texture &emit) :
        m_Emit(emit) {}

    virtual glm::vec3 f(
        const glm::vec3 &p, const glm::vec3 &wo, const glm::vec3 &wi) const
    {
        return glm::vec3();
    }

    virtual glm::vec3 Sample_f(
        const glm::vec3 &p, const glm::vec3 &wo,
        glm::vec3 &wi, float &pdf, bool &specular) const
    {
        pdf = 0;
        specular = false;
        return glm::vec3();
    }

    virtual float Pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
        return 0;
    }

    virtual glm::vec3 Emitted(
        const float u, const float v, const glm::vec3 &p) const
    {
        return m_Emit->Sample(u, v, p);
    }

    virtual bool Emits() const {
        return true;
    }

private:
    P_Texture m_Emit;
};
