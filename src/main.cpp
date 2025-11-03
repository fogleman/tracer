#define GLM_ENABLE_EXPERIMENTAL
#include "tracer/tracer.hpp"

const int width = 2048;
const int height = 2048;

const int samplesPerFrame = 16;
const int numFrames = -1;
const int numThreads = -1;

const vec3 eye(3, 0, 1);
const vec3 center(0, 0, 0);
const vec3 up(0, 0, 1);
const real fovy = 25;
const real aspect = real(width) / height;
const real aperture = 0.01;
const real focalDistance = 3;

int main(int argc, char **argv) {
    // if (argc != 2) {
    //     std::cout << "Usage: tracer input.stl" << std::endl;
    //     return 1;
    // }

    RTCDevice device = rtcNewDevice(NULL);

    auto world = std::make_shared<HittableList>();

    auto mesh = LoadBinarySTL(argv[2]);
    // mesh->SmoothNormals();
    mat4 matrix = mesh->FitInUnitCube();
    mesh->Rotate(glm::radians(90.f), up);

    // auto blinn = std::make_shared<BlinnDistribution>(100);
    // auto albedo = std::make_shared<SolidTexture>(HexColor(0xFFF0A5));
    // auto material = std::make_shared<Microfacet>(albedo, blinn, 2);
    // auto material = std::make_shared<Metal>(albedo);
    // auto material = std::make_shared<FresnelBlend>(albedo, albedo, blinn);
    // auto material = std::make_shared<SpecularReflection>(albedo, 1.5);
    auto material = std::make_shared<Lambertian>(
        std::make_shared<SolidTexture>(HexColor(0x2980B9) * 0.9));
    // DisneyParameters params = {
    //     vec3(0.025, 0.25, 0.018), // BaseColor
    //     0.5, // Metallic
    //     0, // Subsurface
    //     0, // Specular
    //     0.5, // Roughness
    //     0, // SpecularTint
    //     0, // Anisotropic
    //     0, // Sheen
    //     0, // SheenTint
    //     0, // Clearcoat
    //     0, // ClearcoatGloss
    // };
    // auto material = std::make_shared<Disney>(params);

    world->Add(std::make_shared<EmbreeMesh>(device, mesh, material));

    {
        auto mesh = LoadBinarySTL(argv[1]);
        // mesh->SmoothNormals();
        // mat4 matrix = mesh->FitInUnitCube();
        mesh->Transform(matrix);
        mesh->Rotate(glm::radians(90.f), up);
        auto material = std::make_shared<Lambertian>(
            std::make_shared<SolidTexture>(HexColor(0xE74C3C) * 0.9));
        world->Add(std::make_shared<EmbreeMesh>(device, mesh, material));
    }

    const real z = mesh->BoundingBox().Min().z;
    auto white = std::make_shared<Lambertian>(
        std::make_shared<SolidTexture>(HexColor(0xECF0F1) * 0.9));
    world->Add(std::make_shared<Cube>(vec3(-100), vec3(100, 100, z), white));

    auto light = std::make_shared<DiffuseLight>(
        std::make_shared<SolidTexture>(Kelvin(5000) * real(7.5)));
    world->Add(std::make_shared<Sphere>(vec3(3, 2, 2), 1, light));
    world->Add(std::make_shared<Sphere>(vec3(3, -2, 2), 1, light));

    Camera camera(eye, center, up, fovy, aspect, aperture, focalDistance);
    Sampler sampler(world);
    Image image(width, height);

    Run(image, sampler, camera, numFrames, samplesPerFrame, numThreads);

    return 0;
}
