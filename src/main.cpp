#define GLM_ENABLE_EXPERIMENTAL
#include "tracer/tracer.hpp"

const int width = 1600;
const int height = 1600;

const int samplesPerFrame = 16;
const int numFrames = -1;
const int numThreads = -1;

const vec3 eye(3, 0, 1);
const vec3 center(0, 0, -0.075);
const vec3 up(0, 0, 1);
const real fovy = 25;
const real aspect = real(width) / height;
const real aperture = 0.01;
const real focalDistance = 3;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: tracer input.stl" << std::endl;
        return 1;
    }

    RTCDevice device = rtcNewDevice(NULL);

    auto world = std::make_shared<HittableList>();

    auto mesh = LoadBinarySTL(argv[1]);
    // mesh->SmoothNormals();
    mesh->FitInUnitCube();
    mesh->Rotate(glm::radians(60.f), up);

    // model
    {
        const DisneyParameters params = {
            HexColor(0x777880), // BaseColor
            0, // Metallic
            0.1, // Subsurface
            0.1, // Specular
            0.2, // Roughness
            0, // SpecularTint
            0, // Anisotropic
            0, // Sheen
            0, // SheenTint
            0.1, // Clearcoat
            0, // ClearcoatGloss
        };
        const auto material = std::make_shared<Disney>(params);
        world->Add(std::make_shared<EmbreeMesh>(device, mesh, material));
    }

    // floor
    {
        const DisneyParameters params = {
            HexColor(0x2A2C2B), // BaseColor
            0, // Metallic
            0.1, // Subsurface
            0.5, // Specular
            0.5, // Roughness
            0, // SpecularTint
            0, // Anisotropic
            0, // Sheen
            0, // SheenTint
            0.5, // Clearcoat
            0.5, // ClearcoatGloss
        };
        const auto material = std::make_shared<Disney>(params);
        const real z = mesh->BoundingBox().Min().z;
        world->Add(std::make_shared<Cube>(vec3(-100), vec3(100, 100, z), material));
    }

    // lights
    {
        auto light = std::make_shared<DiffuseLight>(
            std::make_shared<SolidTexture>(Kelvin(5000) * real(10)));
        world->Add(std::make_shared<Sphere>(vec3(5, 3, 3), 2, light));
        // world->Add(std::make_shared<Sphere>(vec3(5, 0, 3), 2, light));
        world->Add(std::make_shared<Sphere>(vec3(5, -3, 3), 2, light));

        auto backlight = std::make_shared<DiffuseLight>(
            std::make_shared<SolidTexture>(Kelvin(5000) * real(2)));
        world->Add(std::make_shared<Sphere>(vec3(-5, 0, 3), 2, backlight));
    }

    Camera camera(eye, center, up, fovy, aspect, aperture, focalDistance);
    Sampler sampler(world);
    Image image(width, height);

    Run(image, sampler, camera, numFrames, samplesPerFrame, numThreads);

    return 0;
}

// auto blinn = std::make_shared<BlinnDistribution>(100);
// auto albedo = std::make_shared<SolidTexture>(HexColor(0xFFF0A5));
// auto material = std::make_shared<Microfacet>(albedo, blinn, 2);
// auto material = std::make_shared<Metal>(albedo);
// auto material = std::make_shared<FresnelBlend>(albedo, albedo, blinn);
// auto material = std::make_shared<SpecularReflection>(albedo, 1.5);
// auto material = std::make_shared<Lambertian>(std::make_shared<SolidTexture>(HexColor(0x2980B9) * 0.9));
