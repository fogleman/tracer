#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#include "config.hpp"
#include "microfacet.hpp"
#include "ray.hpp"
#include "texture.hpp"
#include "util.hpp"

class Material {
public:
    virtual vec3 f(
        const vec3 &p,
        const vec3 &wo, const vec3 &wi) const = 0;

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        // wi = glm::normalize(RandomInUnitSphere());
        // if (wo.z * wi.z <= 0) {
        //     wi = vec3(wi.x, wi.y, -wi.z);
        // }
        // pdf = 0.5 / PI;
        wi = CosineSampleHemisphere();
        if (wo.z < 0) {
            wi = vec3(wi.x, wi.y, -wi.z);
        }
        pdf = Pdf(wo, wi);
        specular = false;
        return f(p, wo, wi);
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        if (wo.z * wi.z <= 0) {
            return 0;
        }
        return std::abs(wi.z) / PI;
    }

    virtual vec3 Emitted(
        const real u, const real v, const vec3 &p) const
    {
        return vec3();
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

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return vec3();
        // return m_Albedo->Sample(0, 0, p) / (2 * PI);
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        wi = glm::normalize(RandomInUnitSphere());
        pdf = 1;
        specular = true;
        return m_Albedo->Sample(0, 0, p);
        // pdf = Pdf(wo, wi);
        // specular = false;
        // return f(p, wo, wi);
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        return 0;
        // return 0.25 / PI;
    }

private:
    P_Texture m_Albedo;
};

inline real G(const vec3 &wo, const vec3 &wi, const vec3 &wh) {
    const real NdotWh = AbsCosTheta(wh);
    const real NdotWo = AbsCosTheta(wo);
    const real NdotWi = AbsCosTheta(wi);
    const real WOdotWh = std::abs(glm::dot(wo, wh));
    return std::min(real(1), std::min(
        2 * NdotWh * NdotWo / WOdotWh,
        2 * NdotWh * NdotWi / WOdotWh));
}

class FresnelBlend : public Material {
public:
    FresnelBlend(
        const P_Texture &Rd, const P_Texture &Rs,
        const P_MicrofacetDistribution &distribution) :
    m_Rd(Rd), m_Rs(Rs), m_Distribution(distribution) {}

    vec3 SchlickFresnel(const vec3 &rs, const real costheta) const {
        return rs + std::pow(1 - costheta, 5.f) * (vec3(1) - rs);
    }

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        const vec3 rd = m_Rd->Sample(0, 0, p);
        const vec3 rs = m_Rs->Sample(0, 0, p);
        const vec3 diffuse = real(28 / (23 * PI)) * rd *
            (vec3(1) - rs) *
            (1 - std::pow(1 - 0.5 * AbsCosTheta(wi), 5)) *
            (1 - std::pow(1 - 0.5 * AbsCosTheta(wo), 5));
        const vec3 wh = glm::normalize(wi + wo);
        const vec3 specular = m_Distribution->D(wh) /
            (4 * std::abs(glm::dot(wi, wh)) *
            std::max(AbsCosTheta(wi), AbsCosTheta(wo))) *
            SchlickFresnel(rs, glm::dot(wi, wh));
        return diffuse + specular;
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        if (!SameHemisphere(wo, wi)) {
            return 0;
        }
        return 0.5 * (AbsCosTheta(wi) / PI + m_Distribution->Pdf(wo, wi));
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        if (Random() < 0.5) {
            wi = CosineSampleHemisphere();
            if (wo.z < 0) {
                wi = vec3(wi.x, wi.y, -wi.z);
            }
        } else {
            m_Distribution->Sample_f(p, wo, wi, pdf);
            if (!SameHemisphere(wo, wi)) {
                return vec3();
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
        const real eta) :
    m_Albedo(albedo), m_Distribution(distribution), m_Eta(eta) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        const real cosThetaO = AbsCosTheta(wo);
        const real cosThetaI = AbsCosTheta(wi);
        if (cosThetaO == 0 || cosThetaI == 0) {
            return vec3();
        }
        const vec3 wh = glm::normalize(wi + wo);
        const real cosThetaH = glm::dot(wi, wh);
        const real F = Schlick(cosThetaH, m_Eta);
        const vec3 R = m_Albedo->Sample(0, 0, p);
        return R * m_Distribution->D(wh) * G(wo, wi, wh) * F /
            (4 * cosThetaI * cosThetaO);
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        if (!SameHemisphere(wo, wi)) {
            return 0;
        }
        return m_Distribution->Pdf(wo, wi);
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        m_Distribution->Sample_f(p, wo, wi, pdf);
        if (!SameHemisphere(wo, wi)) {
            return vec3();
        }
        specular = false;
        return f(p, wo, wi);
    }

private:
    P_Texture m_Albedo;
    P_MicrofacetDistribution m_Distribution;
    real m_Eta;
};

class Metal : public Material {
public:
    Metal(const P_Texture &albedo) :
        m_Albedo(albedo) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return vec3(0);
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        return 0;
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        wi = vec3(-wo.x, -wo.y, wo.z);
        pdf = 1;
        specular = true;
        return m_Albedo->Sample(0, 0, p);
    }

private:
    P_Texture m_Albedo;
};

class Dielectric : public Material {
public:
    Dielectric(const P_Texture &albedo, const real eta) :
        m_Albedo(albedo), m_Eta(eta) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return vec3(0);
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        return 0;
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        vec3 outwardNormal;
        real ratio;
        if (wo.z < 0) {
            outwardNormal = vec3(0, 0, -1);
            ratio = m_Eta;
        } else {
            outwardNormal = vec3(0, 0, 1);
            ratio = 1 / m_Eta;
        }

        vec3 refracted;
        real reflectProbability;
        if (Refract(-wo, outwardNormal, ratio, refracted)) {
            reflectProbability = Schlick(AbsCosTheta(wo), m_Eta);
        } else {
            reflectProbability = 1;
        }

        if (Random() < reflectProbability) {
            wi = vec3(-wo.x, -wo.y, wo.z);
        } else {
            wi = refracted;
        }

        pdf = 1;
        specular = true;
        return m_Albedo->Sample(0, 0, p);
    }

private:
    P_Texture m_Albedo;
    real m_Eta;
};

class SpecularReflection : public Material {
public:
    SpecularReflection(const P_Texture &albedo, const real eta) :
        m_Albedo(albedo), m_Eta(eta) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return vec3();
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        wi = vec3(-wo.x, -wo.y, wo.z);
        pdf = 1;
        specular = true;
        const real fr = Schlick(AbsCosTheta(wo), m_Eta);
        return m_Albedo->Sample(0, 0, p) * fr;
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        return 0;
    }

private:
    P_Texture m_Albedo;
    real m_Eta;
};

class Lambertian : public Material {
public:
    Lambertian(const P_Texture &albedo) :
        m_Albedo(albedo) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return m_Albedo->Sample(0, 0, p) / PI;
    }

private:
    P_Texture m_Albedo;
};

class OrenNayar : public Material {
public:
    OrenNayar(const P_Texture &albedo, const real sigma_degrees) :
        m_Albedo(albedo)
    {
        const real sigma = sigma_degrees * PI / 180;
        const real sigma2 = sigma * sigma;
        m_A = 1 - (sigma2 / (2 * (sigma2 + 0.33)));
        m_B = 0.45 * sigma2 / (sigma2 + 0.09);
    }

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        const real sinthetai = SinTheta(wi);
        const real sinthetao = SinTheta(wo);
        real maxcos = 0;
        if (sinthetai > EPS && sinthetao > EPS) {
            const real sinphii = SinPhi(wi);
            const real cosphii = CosPhi(wi);
            const real sinphio = SinPhi(wo);
            const real cosphio = CosPhi(wo);
            const real dcos = cosphii * cosphio + sinphii * sinphio;
            maxcos = std::max(real(0), dcos);
        }
        real sinalpha, tanbeta;
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
    real m_A, m_B;
};

class DiffuseLight : public Material {
public:
    DiffuseLight(const P_Texture &emit) :
        m_Emit(emit) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return vec3();
    }

    virtual vec3 Sample_f(
        const vec3 &p, const vec3 &wo,
        vec3 &wi, real &pdf, bool &specular) const
    {
        pdf = 0;
        specular = false;
        return vec3();
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        return 0;
    }

    virtual vec3 Emitted(
        const real u, const real v, const vec3 &p) const
    {
        return m_Emit->Sample(u, v, p);
    }

    virtual bool Emits() const {
        return true;
    }

private:
    P_Texture m_Emit;
};
