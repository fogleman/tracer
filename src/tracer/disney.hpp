#pragma once

#include <glm/glm.hpp>

#include "config.hpp"
#include "material.hpp"
#include "util.hpp"

// https://github.com/mmp/pbrt-v3/blob/master/src/materials/disney.cpp
// https://www.shadertoy.com/view/XdyyDd
// https://schuttejoe.github.io/post/disneybsdf/
// https://github.com/schuttejoe/Selas/blob/dev/Source/Core/Shading/Disney.cpp
// https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf

class DisneyParameters {
public:
    vec3 baseColor;
    real metallic;
    real subsurface;
    real specular;
    real roughness;
    real specularTint;
    real anisotropic;
    real sheen;
    real sheenTint;
    real clearcoat;
    real clearcoatGloss;
};

namespace {

real pow2(const real x) {
    return x * x;
}

real schlickWeight(const real cosTheta) {
    const real m = glm::clamp(1 - cosTheta, real(0), real(1));
    return (m * m) * (m * m) * m;
}

real gtr1(const real NdotH, const real a) {
    if (a >= 1) {
        return 1 / PI;
    }
    const real a2 = a * a;
    const real t = 1 + (a2 - 1) * NdotH * NdotH;
    return (a2 - 1) / (PI * std::log(a2) * t);
}

real gtr2(const real NdotH, const real a) {
    const real a2 = a * a;
    const real t = 1 + (a2 - 1) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

real gtr2(
    const real NdotH, const real HdotX, const real HdotY,
    const real ax, const real ay)
{
    return 1 / (PI * ax * ay * pow2(
        pow2(HdotX / ax) + pow2(HdotY / ay) + NdotH * NdotH));
}

real ggx(const real NdotV, const real alphaG) {
    const real a = alphaG * alphaG;
    const real b = NdotV * NdotV;
    return 1 / (std::abs(NdotV) + std::max(std::sqrt(a + b - a * b), EPS));
}

real ggx(
    const real NdotV, const real VdotX, const real VdotY,
    const real ax, const real ay)
{
    return 1 / (NdotV + std::sqrt(
        pow2(VdotX * ax) + pow2(VdotY * ay) + pow2(NdotV)));
}

}

class Disney : public Material {
public:
    Disney(const DisneyParameters &params) :
        m_Params(params) {}

    virtual vec3 f(
        const vec3 &p, const vec3 &wo, const vec3 &wi) const
    {
        return disneyEvaluate(
            wi, wo, vec3(0, 0, 1), vec3(1, 0, 0), vec3(0, 1, 0));
    }

    virtual real Pdf(const vec3 &wo, const vec3 &wi) const {
        return disneyPdf(wi, wo, vec3(1, 0, 0), vec3(0, 1, 0));
    }

    vec3 disneyDiffuse(real NdotL, real NdotV, real LdotH) const {
        real FL = schlickWeight(NdotL);
        real FV = schlickWeight(NdotV);
        real Fd90 = 0.5 + 2 * LdotH * LdotH * m_Params.roughness;
        real Fd = glm::mix(real(1), Fd90, FL) * glm::mix(real(1), Fd90, FV);
        return Fd * m_Params.baseColor / PI;
    }

    vec3 disneySubsurface(real NdotL, real NdotV, real LdotH) const {
        real FL = schlickWeight(NdotL);
        real FV = schlickWeight(NdotV);
        real Fss90 = LdotH * LdotH * m_Params.roughness;
        real Fss = glm::mix(real(1), Fss90, FL) * glm::mix(real(1), Fss90, FV);
        real ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - 0.5) + 0.5);
        return ss * m_Params.baseColor / PI;
    }

    vec3 disneyCtint() const {
        real Cdlum =
            0.3 * m_Params.baseColor.r +
            0.6 * m_Params.baseColor.g +
            0.1 * m_Params.baseColor.b;
        return Cdlum > 0 ? m_Params.baseColor / Cdlum : vec3(1);
    }

    vec3 disneyCspec0() const {
        vec3 Ctint = disneyCtint();
        return glm::mix(
            m_Params.specular * 0.08 *
                glm::mix(vec3(1), Ctint, m_Params.specularTint),
            m_Params.baseColor, m_Params.metallic);
    }

    vec3 disneyMicrofacetIsotropic(
        real NdotL, real NdotV, real NdotH, real LdotH) const
    {
        vec3 Cspec0 = disneyCspec0();
        real a = std::max(0.001, pow2(m_Params.roughness));
        real Ds = gtr2(NdotH, a);
        real FH = schlickWeight(LdotH);
        vec3 Fs = glm::mix(Cspec0, vec3(1), FH);
        real Gs = ggx(NdotL, a) * ggx(NdotV, a);
        return Gs * Fs * Ds;
    }

    vec3 disneyMicrofacetAnisotropic(
        real NdotL, real NdotV, real NdotH, real LdotH,
        vec3 L, vec3 V, vec3 H, vec3 X, vec3 Y) const
    {
        vec3 Cspec0 = disneyCspec0();
        real aspect = std::sqrt(1 - m_Params.anisotropic * 0.9);
        real ax = std::max(0.001, pow2(m_Params.roughness) / aspect);
        real ay = std::max(0.001, pow2(m_Params.roughness) * aspect);
        real Ds = gtr2(NdotH, glm::dot(H, X), glm::dot(H, Y), ax, ay);
        real FH = schlickWeight(LdotH);
        vec3 Fs = glm::mix(Cspec0, vec3(1), FH);
        real Gs = (
            ggx(NdotL, glm::dot(L, X), glm::dot(L, Y), ax, ay) *
            ggx(NdotV, glm::dot(V, X), glm::dot(V, Y), ax, ay));
        return Gs * Fs * Ds;
    }

    real disneyClearCoat(real NdotL, real NdotV, real NdotH, real LdotH) const {
        real gloss = glm::mix(0.1, 0.001, m_Params.clearcoatGloss);
        real Dr = gtr1(std::abs(NdotH), gloss);
        real FH = schlickWeight(LdotH);
        real Fr = glm::mix(0.04, 1.0, FH);
        real Gr = ggx(NdotL, 0.25) * ggx(NdotV, 0.25);
        return m_Params.clearcoat * Fr * Gr * Dr;
    }

    vec3 disneySheen(real LdotH) const {
        real FH = schlickWeight(LdotH);
        vec3 Ctint = disneyCtint();
        vec3 Csheen = glm::mix(vec3(1), Ctint, m_Params.sheenTint);
        return FH * m_Params.sheen * Csheen;
    }

    vec3 disneyEvaluate(vec3 wi, vec3 wo, vec3 N, vec3 X, vec3 Y) const {
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
        return (glm::mix(diffuse, subSurface, m_Params.subsurface) + sheen) *
            (1 - m_Params.metallic) + glossy + clearCoat;
    }

    real pdfLambertian(vec3 wi, vec3 wo) const {
        if (wo.z * wi.z <= 0) {
            return 0;
        }
        return std::abs(wi.z) / PI;
    }

    real pdfMicrofacet(vec3 wi, vec3 wo) const {
        if (wo.z * wi.z <= 0) {
            return 0;
        }
        vec3 N(0, 0, 1);
        vec3 wh = glm::normalize(wo + wi);
        real NdotH = glm::dot(N, wh);
        real alpha2 = pow2(m_Params.roughness * m_Params.roughness);
        real cos2Theta = NdotH * NdotH;
        real denom = cos2Theta * (alpha2 - 1) + 1;
        if (denom == 0) {
            return 0;
        }
        real pdfDistribution = alpha2 * NdotH / (PI * denom * denom);
        return pdfDistribution / (4 * glm::dot(wo, wh));
    }

    real pdfMicrofacetAnisotropic(vec3 wi, vec3 wo, vec3 X, vec3 Y) const {
        if (wo.z * wi.z <= 0) {
            return 0;
        }
        vec3 N(0, 0, 1);
        vec3 wh = glm::normalize(wo + wi);
        real aspect = std::sqrt(1 - m_Params.anisotropic * 0.9);
        real alphax = std::max(real(0.001), pow2(m_Params.roughness) / aspect);
        real alphay = std::max(real(0.001), pow2(m_Params.roughness) * aspect);
        real alphax2 = alphax * alphax;
        real alphay2 = alphax * alphay;
        real hDotX = glm::dot(wh, X);
        real hDotY = glm::dot(wh, Y);
        real NdotH = glm::dot(N, wh);
        real denom = hDotX * hDotX / alphax2 + hDotY * hDotY / alphay2 + NdotH * NdotH;
        if (denom == 0) {
            return 0;
        }
        real pdfDistribution = NdotH / (PI * alphax * alphay * denom * denom);
        return pdfDistribution / (4 * glm::dot(wo, wh));
    }

    real pdfClearCoat(vec3 wi, vec3 wo) const {
        if (wo.z * wi.z <= 0) {
            return 0;
        }
        vec3 N(0, 0, 1);
        vec3 wh = glm::normalize(wo + wi);
        real NdotH = std::abs(glm::dot(wh, N));
        real Dr = gtr1(NdotH, glm::mix(0.1, 0.001, m_Params.clearcoatGloss));
        return Dr * NdotH / (4 * glm::dot(wo, wh));
    }

    real disneyPdf(vec3 wi, vec3 wo, vec3 X, vec3 Y) const {
        real sum = 0;
        sum += pdfLambertian(wi, wo);
        sum += pdfMicrofacetAnisotropic(wi, wo, X, Y);
        sum += pdfClearCoat(wi, wo);
        return sum / 3;
    }

private:
    DisneyParameters m_Params;
};
