#include <pbrt/cpu/precompute.h>
#include <ext/spherical-harmonics/spherical_harmonics.h>

namespace pbrt
{

std::unique_ptr<PrtProbeIntegrator> PrtProbeIntegrator::Create(
    const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
    std::vector<Light> lights, const FileLoc *loc) {
    int maxDepth = parameters.GetOneInt("maxdepth", 5);
    return std::make_unique<PrtProbeIntegrator>(maxDepth, sampler, aggregate, lights);
}

void PrtProbeIntegrator::Render() {
    Point2i pPixel(0, 0);
    int sampleIndex = 0;

    ScratchBuffer scratchBuffer(65536);
    Sampler tileSampler = samplerPrototype.Clone(Allocator());
    tileSampler.StartPixelSample(pPixel, sampleIndex);

    Vector3f dir[6] = {{0.0, 0.0, 1.0},  {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0},
                       {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},  {0.0, -1.0, 0.0}};
    Point3f o = {0.0, 0.0, 1.0};

    Ray testRay({0.0, -2.5, 1.0}, {0.0, 1.0, 0.0});
    int hitCount = IntersectN(testRay);

    for (int i = 0; i != 6; ++i)
    {
        RayDifferential ray(o, dir[i]);
        int hitCount = IntersectN(ray);
        LOG_VERBOSE("hit count %d", hitCount);
    }
}

std::string PrtProbeIntegrator::ToString() const {
    return StringPrintf("[ PrtProbeIntegrator maxDepth: %d ]", maxDepth);
}

int PrtProbeIntegrator::IntersectN(const Ray &ray, Float tMax /*= Infinity*/) {
    // Return hit count on the first hit object
    int hitCount = 0;
    unsigned int firstGeometryId = InvalidGeometryId;

    Ray nextRay = ray;
    while (true) {
        pstd::optional<ShapeIntersection> si = aggregate.Intersect(nextRay);
        if (!si) {
            break;
        }

        if (firstGeometryId == InvalidGeometryId) {
            firstGeometryId = si->geometryId;
            ++hitCount;
            Point3f hitPoint = ray(si->tHit);
            LOG_VERBOSE("hitpoint %s", hitPoint);
        } else {
            if (si->geometryId == firstGeometryId) {
                ++hitCount;
            }
        }

        nextRay = si->intr.SpawnRay(nextRay.d);
    }

    return hitCount;
}

}  // namespace pbrt