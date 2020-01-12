#include "gi/gi.hpp"

#include <sstream>
#include <vector>

typedef struct {
    int p;
    float x;
    float y;
    float z;
} Record;

std::vector<EmbreeSphere> LoadSpheres(const std::string &filename) {
    FILE *fp = fopen(filename.c_str(), "r");
    fseek(fp, 0, SEEK_END);
    int count = ftell(fp) / 16 / 2;
    fseek(fp, 0, SEEK_SET);

    std::vector<EmbreeSphere> spheres;
    spheres.reserve(count);

    float x0, y0, z0, x1, y1, z1;
    x0 = y0 = z0 = x1 = y1 = z1 = 0;
    for (int i = 0; i < count; i++) {
        Record r;
        fread(&r, sizeof(Record), 1, fp);
        fread(&r, sizeof(Record), 1, fp);
        spheres.push_back({r.x, r.y, r.z, 1.5f});
        x0 = std::min(x0, r.x);
        y0 = std::min(y0, r.y);
        z0 = std::min(z0, r.z);
        x1 = std::max(x1, r.x);
        y1 = std::max(y1, r.y);
        z1 = std::max(z1, r.z);
    }
    fclose(fp);

    const vec3 min(x0, y0, z0);
    const vec3 max(x1, y1, z1);
    const vec3 size = max - min;
    const vec3 center(0);// = min + size / 2;
    const real scale = 1 / glm::compMax(size);
    for (int i = 0; i < count; i++) {
        spheres[i].x = (spheres[i].x - center.x) * scale;
        spheres[i].y = (spheres[i].y - center.y) * scale;
        spheres[i].z = (spheres[i].z - center.z) * scale;
        spheres[i].r = spheres[i].r * scale;
    }

    return spheres;
}

int main(int argc, char **argv) {
    auto world = std::make_shared<HittableList>();

    const RTCDevice device = rtcNewDevice(NULL);

    const auto spheres = LoadSpheres(argv[1]);

    const int numMaterials = 1000;
    std::vector<P_Material> materials;
    for (int i = 0; i < numMaterials; i++) {
        const real t = i / real(numMaterials - 1);
        materials.push_back(std::make_shared<OrenNayar>(
            std::make_shared<SolidTexture>(Viridis.At(t)), 20));
    }

    world->Add(std::make_shared<EmbreeSpheres>(device, spheres, materials));

    const auto light = std::make_shared<DiffuseLight>(
        std::make_shared<SolidTexture>(Kelvin(5000) * real(45)));
    world->Add(std::make_shared<Sphere>(vec3(2, 3, 2), 1, light));

    const int width = 1600;
    const int height = 1600;
    const int numFrames = 0;
    const int numSamples = 16;
    const int numThreads = 16;

    const vec3 eye(3, 0, 0);
    const vec3 center(0, 0, 0);
    const vec3 up(0, 0, 1);
    const float fovy = 22;
    const float aspect = float(width) / height;
    const float aperture = 0.01;
    const float focalDistance = 2.75;

    Camera camera(eye, center, up, fovy, aspect, aperture, focalDistance);
    Sampler sampler(world);
    Image image(width, height);

    std::cout << "start" << std::endl;

    for (int i = 1; ; i++) {
        Render(image, sampler, camera, numSamples, numThreads);

        char path[100];
        snprintf(path, 100, "%08d.png", i - 1);

        image.SavePNG(path);
        std::cout << path << std::endl;

        if (numFrames > 0 && i == numFrames) {
            break;
        }
    }

    return 0;
}
