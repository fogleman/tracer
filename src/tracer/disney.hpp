#pragma once

#include <glm/glm.hpp>

#include "config.hpp"
#include "material.hpp"
#include "util.hpp"

// https://www.shadertoy.com/view/XdyyDd
// https://schuttejoe.github.io/post/disneybsdf/
// https://github.com/schuttejoe/Selas/blob/dev/Source/Core/Shading/Disney.cpp
// https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf

class DisneyParameters {
public:
    vec3 BaseColor;
    real Metallic;
    real Subsurface;
    real Specular;
    real Roughness;
    real SpecularTint;
    real Anisotropic;
    real Sheen;
    real SheenTint;
    real Clearcoat;
    real ClearcoatGloss;
};

namespace {

real Pow2(const real x) {
    return x * x;
}

real SchlickWeight(const real cosTheta) {
    const real m = glm::clamp(1 - cosTheta, real(0), real(1));
    return (m * m) * (m * m) * m;
}

real GTR1(const real NdotH, const real a) {
    if (a >= 1) {
        return 1 / PI;
    }
    const real a2 = a * a;
    const real t = 1 + (a2 - 1) * NdotH * NdotH;
    return (a2 - 1) / (PI * std::log(a2) * t);
}

real GTR2(const real NdotH, const real a) {
    const real a2 = a * a;
    const real t = 1 + (a2 - 1) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

real GTR2(
    const real NdotH, const real HdotX, const real HdotY,
    const real ax, const real ay)
{
    return 1 / (PI * ax * ay * Pow2(
        Pow2(HdotX / ax) + Pow2(HdotY / ay) + NdotH * NdotH));
}

real GGX(const real NdotV, const real alphaG) {
    const real a = alphaG * alphaG;
    const real b = NdotV * NdotV;
    return 1 / (std::abs(NdotV) + std::max(std::sqrt(a + b - a * b), EPS));
}

real GGX(
    const real NdotV, const real VdotX, const real VdotY,
    const real ax, const real ay)
{
    return 1 / (NdotV + std::sqrt(
        Pow2(VdotX * ax) + Pow2(VdotY * ay) + Pow2(NdotV)));
}

}

class Disney : public Material {
public:
    Disney(const DisneyParameters &params) :
        m_Params(params) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return bsdfEvaluate(wi, wo, vec3(1, 0, 0), vec3(0, 1, 0));
    }

    vec3 disneyDiffuse(real NdotL, real NdotV, real LdotH) const {
        real FL = SchlickWeight(NdotL);
        real FV = SchlickWeight(NdotV);
        real Fd90 = 0.5 + 2 * LdotH * LdotH * m_Params.Roughness;
        real Fd = glm::mix(real(1), Fd90, FL) * glm::mix(real(1), Fd90, FV);
        return Fd * m_Params.BaseColor / PI;
    }

    vec3 disneySubsurface(real NdotL, real NdotV, real LdotH) const {
        real FL = SchlickWeight(NdotL);
        real FV = SchlickWeight(NdotV);
        real Fss90 = LdotH * LdotH * m_Params.Roughness;
        real Fss = glm::mix(real(1), Fss90, FL) * glm::mix(real(1), Fss90, FV);
        real ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - 0.5) + 0.5);
        return ss * m_Params.BaseColor / PI;
    }

    vec3 disneyCtint() const {
        real Cdlum =
            0.3 * m_Params.BaseColor.r +
            0.6 * m_Params.BaseColor.g +
            0.1 * m_Params.BaseColor.b;
        return Cdlum > 0 ? m_Params.BaseColor / Cdlum : vec3(1);
    }

    vec3 disneyCspec0() const {
        vec3 Ctint = disneyCtint();
        return glm::mix(
            m_Params.Specular * 0.08 *
                glm::mix(vec3(1), Ctint, m_Params.SpecularTint),
            m_Params.BaseColor, m_Params.Metallic);
    }

    vec3 disneyMicrofacetIsotropic(
        real NdotL, real NdotV, real NdotH, real LdotH) const
    {
        vec3 Cspec0 = disneyCspec0();
        real a = std::max(0.001, Pow2(m_Params.Roughness));
        real Ds = GTR2(NdotH, a);
        real FH = SchlickWeight(LdotH);
        vec3 Fs = glm::mix(Cspec0, vec3(1), FH);
        real Gs = GGX(NdotL, a) * GGX(NdotV, a);
        return Gs * Fs * Ds;
    }

    vec3 disneyMicrofacetAnisotropic(
        real NdotL, real NdotV, real NdotH, real LdotH,
        vec3 L, vec3 V, vec3 H, vec3 X, vec3 Y) const
    {
        vec3 Cspec0 = disneyCspec0();
        real aspect = std::sqrt(1 - m_Params.Anisotropic * 0.9);
        real ax = std::max(0.001, Pow2(m_Params.Roughness) / aspect);
        real ay = std::max(0.001, Pow2(m_Params.Roughness) * aspect);
        real Ds = GTR2(NdotH, glm::dot(H, X), glm::dot(H, Y), ax, ay);
        real FH = SchlickWeight(LdotH);
        vec3 Fs = glm::mix(Cspec0, vec3(1), FH);
        real Gs = (
            GGX(NdotL, glm::dot(L, X), glm::dot(L, Y), ax, ay) *
            GGX(NdotV, glm::dot(V, X), glm::dot(V, Y), ax, ay));
        return Gs * Fs * Ds;
    }

    real disneyClearCoat(real NdotL, real NdotV, real NdotH, real LdotH) const {
        real gloss = glm::mix(0.1, 0.001, m_Params.ClearcoatGloss);
        real Dr = GTR1(std::abs(NdotH), gloss);
        real FH = SchlickWeight(LdotH);
        real Fr = glm::mix(0.04, 1.0, FH);
        real Gr = GGX(NdotL, 0.25) * GGX(NdotV, 0.25);
        return m_Params.Clearcoat * Fr * Gr * Dr;
    }

    vec3 disneySheen(real LdotH) const {
        real FH = SchlickWeight(LdotH);
        vec3 Ctint = disneyCtint();
        vec3 Csheen = glm::mix(vec3(1), Ctint, m_Params.SheenTint);
        return FH * m_Params.Sheen * Csheen;
    }

    vec3 bsdfEvaluate(vec3 wi, vec3 wo, vec3 X, vec3 Y) const {
        vec3 N(0, 0, 1);
        real NdotL = glm::dot(N, wo);
        real NdotV = glm::dot(N, wi);
        if (NdotL < 0 || NdotV < 0) {
            return vec3(0);
        }
        vec3 H = glm::normalize(wo + wi);
        real NdotH = glm::dot(N, H);
        real LdotH = glm::dot(wo, H);
        vec3 diffuse = disneyDiffuse(NdotL, NdotV, LdotH);
        vec3 subSurface = disneySubsurface(NdotL, NdotV, LdotH);
        vec3 glossy = disneyMicrofacetAnisotropic(
            NdotL, NdotV, NdotH, LdotH, wi, wo, H, X, Y);
        real clearCoat = disneyClearCoat(NdotL, NdotV, NdotH, LdotH);
        vec3 sheen = disneySheen(LdotH);
        return (glm::mix(diffuse, subSurface, m_Params.Subsurface) + sheen) *
            (1 - m_Params.Metallic) + glossy + clearCoat;
    }

private:
    DisneyParameters m_Params;
};
