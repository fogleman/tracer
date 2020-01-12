#pragma once

#include <cstring>
#include <embree3/rtcore.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "config.hpp"
#include "hit.hpp"
#include "material.hpp"

typedef struct {
    float x;
    float y;
    float z;
    float r;
} EmbreeSphere;

class EmbreeSpheres : public Hittable {
public:
    EmbreeSpheres(
        RTCDevice device,
        const std::vector<EmbreeSphere> &spheres,
        const std::vector<P_Material> &materials) :
        m_NumSpheres(spheres.size()),
        m_Materials(materials)
    {
        m_Scene = rtcNewScene(device);
        // rtcSetSceneFlags(m_Scene, RTC_SCENE_FLAG_ROBUST);
        RTCGeometry geom = rtcNewGeometry(
            device, RTC_GEOMETRY_TYPE_SPHERE_POINT);
        EmbreeSphere *buf = (EmbreeSphere *)rtcSetNewGeometryBuffer(
            geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4,
            sizeof(EmbreeSphere), m_NumSpheres);

        memcpy(buf, spheres.data(), sizeof(EmbreeSphere) * m_NumSpheres);

        rtcCommitGeometry(geom);
        rtcAttachGeometry(m_Scene, geom);
        rtcReleaseGeometry(geom);
        rtcCommitScene(m_Scene);
    }

    virtual bool Hit(
        const Ray &ray, const real tmin, const real tmax, HitInfo &hit) const
    {
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);

        const vec3 &org = ray.Origin();
        const vec3 &dir = ray.Direction();

        RTCRayHit r;
        r.ray.org_x = org.x; r.ray.org_y = org.y; r.ray.org_z = org.z;
        r.ray.dir_x = dir.x; r.ray.dir_y = dir.y; r.ray.dir_z = dir.z;
        r.ray.tnear = tmin;
        r.ray.tfar = tmax;
        r.ray.mask = -1;
        r.ray.flags = 0;
        r.ray.time = 0;
        r.ray.id = 0;

        r.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        r.hit.primID = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(m_Scene, &context, &r);

        if (r.hit.primID == RTC_INVALID_GEOMETRY_ID) {
            return false;
        }

        const real t = r.ray.tfar;
        const real x = r.ray.org_x + r.ray.dir_x * t;
        const real y = r.ray.org_y + r.ray.dir_y * t;
        const real z = r.ray.org_z + r.ray.dir_z * t;

        const int i = m_Materials.size() * (real)r.hit.primID / m_NumSpheres;

        hit.T = t;
        hit.Position = vec3(x, y, z);
        hit.Normal = glm::normalize(vec3(r.hit.Ng_x, r.hit.Ng_y, r.hit.Ng_z));
        hit.Material = m_Materials[i];
        return true;
    }

private:
    int m_NumSpheres;
    RTCScene m_Scene;
    std::vector<P_Material> m_Materials;
};
