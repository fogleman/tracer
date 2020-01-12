#pragma once

#include <embree3/rtcore.h>
#include <vector>

#include "material.hpp"
#include "triangle.hpp"

typedef struct {
    float x, y, z;
} EmbreeVertex;

typedef struct {
    int v0, v1, v2;
} EmbreeTriangle;

// TODO: add a mesh class with indexed vertices

class EmbreeMesh : public Hittable {
public:
    EmbreeMesh(
        RTCDevice device,
        const std::vector<Triangle> &triangles,
        const P_Material &material) :
        m_Triangles(triangles),
        m_Material(material)
    {
        m_Scene = rtcNewScene(device);
        RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

        EmbreeVertex *vertexBuf = (EmbreeVertex *)rtcSetNewGeometryBuffer(
            geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
            sizeof(EmbreeVertex), m_Triangles.size() * 3);
        for (int i = 0; i < m_Triangles.size(); i++) {
            const auto &t = m_Triangles[i];
            vertexBuf[i*3+0].x = t.v1.x;
            vertexBuf[i*3+0].y = t.v1.y;
            vertexBuf[i*3+0].z = t.v1.z;
            vertexBuf[i*3+1].x = t.v2.x;
            vertexBuf[i*3+1].y = t.v2.y;
            vertexBuf[i*3+1].z = t.v2.z;
            vertexBuf[i*3+2].x = t.v3.x;
            vertexBuf[i*3+2].y = t.v3.y;
            vertexBuf[i*3+2].z = t.v3.z;
        }

        EmbreeTriangle *triangleBuf = (EmbreeTriangle *)rtcSetNewGeometryBuffer(
            geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
            sizeof(EmbreeTriangle), m_Triangles.size());
        for (int i = 0; i < m_Triangles.size(); i++) {
            triangleBuf[i] = EmbreeTriangle{i*3+0, i*3+1, i*3+2};
        }

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

        hit.T = t;
        hit.Position = vec3(x, y, z);
        hit.Normal = m_Triangles[r.hit.primID].NormalAt(hit.Position);
        hit.Material = m_Material;
        return true;
    }

private:
    RTCScene m_Scene;
    std::vector<Triangle> m_Triangles;
    P_Material m_Material;
};
