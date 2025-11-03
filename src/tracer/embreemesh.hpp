#pragma once

#include <embree4/rtcore.h>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"

typedef struct {
    float x, y, z;
} EmbreeVertex;

typedef struct {
    int v0, v1, v2;
} EmbreeTriangle;

class EmbreeMesh : public Hittable {
public:
    EmbreeMesh(
        RTCDevice device,
        const P_Mesh &mesh,
        const P_Material &material) :
        m_Mesh(mesh),
        m_Material(material)
    {
        m_Scene = rtcNewScene(device);
        RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

        const auto positions = mesh->Positions();
        const auto triangles = mesh->Triangles();

        EmbreeVertex *vertexBuf = (EmbreeVertex *)rtcSetNewGeometryBuffer(
            geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
            sizeof(EmbreeVertex), positions.size());
        for (int i = 0; i < positions.size(); i++) {
            const auto &p = positions[i];
            vertexBuf[i].x = p.x;
            vertexBuf[i].y = p.y;
            vertexBuf[i].z = p.z;
        }

        EmbreeTriangle *triangleBuf = (EmbreeTriangle *)rtcSetNewGeometryBuffer(
            geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
            sizeof(EmbreeTriangle), triangles.size());
        for (int i = 0; i < triangles.size(); i++) {
            const auto &t = triangles[i];
            triangleBuf[i].v0 = t.x;
            triangleBuf[i].v1 = t.y;
            triangleBuf[i].v2 = t.z;
        }

        rtcCommitGeometry(geom);
        rtcAttachGeometry(m_Scene, geom);
        rtcReleaseGeometry(geom);
        rtcCommitScene(m_Scene);
    }

    virtual bool Hit(
        const Ray &ray, const real tmin, const real tmax, HitInfo &hit) const
    {
        RTCIntersectArguments args;
        rtcInitIntersectArguments(&args);

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

        rtcIntersect1(m_Scene, &r, &args);

        if (r.hit.primID == RTC_INVALID_GEOMETRY_ID) {
            return false;
        }

        const real t = r.ray.tfar;
        const real x = r.ray.org_x + r.ray.dir_x * t;
        const real y = r.ray.org_y + r.ray.dir_y * t;
        const real z = r.ray.org_z + r.ray.dir_z * t;

        hit.T = t;
        hit.Position = vec3(x, y, z);
        hit.Normal = m_Mesh->TriangleNormalAt(r.hit.primID, hit.Position);
        hit.Material = m_Material;
        return true;
    }

private:
    RTCScene m_Scene;
    P_Mesh m_Mesh;
    P_Material m_Material;
};
