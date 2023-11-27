#include <pbrt/cpu/precompute.h>
#include <ext/spherical-harmonics/spherical_harmonics.h>
#include <pbrt/util/file.h>

namespace pbrt
{
    
std::string Voxel::ToString() const {
    return StringPrintf("[ Voxel id %d center: %s overlap: %d ]", id, box.Centroid(), overlap);
}

std::unique_ptr<PrtProbeIntegrator> PrtProbeIntegrator::Create(
    const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
    std::vector<Light> lights, const FileLoc *loc) {
    int maxDepth = parameters.GetOneInt("maxdepth", 5);
    int volUnit = parameters.GetOneInt("volunit", 1);
    return std::make_unique<PrtProbeIntegrator>(maxDepth, volUnit, sampler, aggregate,
                                                lights);
}

void PrtProbeIntegrator::Render() {
    Point2i pPixel(0, 0);
    int sampleIndex = 0;

    ScratchBuffer scratchBuffer(65536);
    Sampler tileSampler = samplerPrototype.Clone(Allocator());
    tileSampler.StartPixelSample(pPixel, sampleIndex);


    auto voxels = VoxelizeScene();

    

    WriteVoxels(voxels);
}

std::string PrtProbeIntegrator::ToString() const {
    return StringPrintf("[ PrtProbeIntegrator maxDepth: %d ]", maxDepth);
}

RayGeometryHit PrtProbeIntegrator::IntersectN(const Ray &ray, Float tMax /*= Infinity*/) {
    // Return hit count on the first hit object along the ray
    unsigned int hitCount = 0;
    //unsigned int firstHitGeometryId = InvalidGeometryId;

    Ray nextRay = ray;
    while (true) {
        pstd::optional<ShapeIntersection> si = aggregate.Intersect(nextRay);
        if (!si) {
            break;
        }

        //if (firstHitGeometryId == InvalidGeometryId) {
        //    firstHitGeometryId = si->geometryId;
        //    ++hitCount;
        //    Point3f hitPoint = ray(si->tHit);
        //} else {
        //    if (si->geometryId == firstHitGeometryId) {
        //        ++hitCount;
        //    }
        //}

        nextRay = si->intr.SpawnRay(nextRay.d);
    }

    return RayGeometryHit{hitCount};
}

pstd::vector<pbrt::Voxel> PrtProbeIntegrator::VoxelizeScene() {

    pstd::vector<pbrt::Voxel> voxels;

    Bounds3f bounds = aggregate.Bounds();
    Vector3f diagonal = bounds.Diagonal();
    int xres = std::lround(diagonal.x);
    int yres = std::lround(diagonal.y);
    int zres = std::lround(diagonal.z);

    int voxelId = 0;
    for (int x = 0; x <= xres; x += voxelUnit) 
    {
        for (int y = 0; y <= yres; y += voxelUnit) 
        {
            for (int z = 0; z <= zres; z += voxelUnit)
            {
                Point3f pMin(x, y, z);
                Point3f pMax(x + voxelUnit, y + voxelUnit, z + voxelUnit);

                Bounds3f box(bounds.pMin + pMin, bounds.pMin + pMax);
                voxels.emplace_back(Voxel{box, false, voxelId});

                ++voxelId;
            }
        }
    }

    SurfaceVoxelize(voxels);

    return voxels;
}

//#pragma optimize("", off)
void PrtProbeIntegrator::SurfaceVoxelize(pstd::vector<Voxel> &voxels) {

    for (auto& voxel : voxels)
    {
        const Float delta = 0.1;
        const Point3f offset = Point3f(delta, delta, delta);
        Point3f pMin = Point3f(voxel.box.pMin + offset);
        Point3f pMax = Point3f(voxel.box.pMax - offset);

        Bounds3f adjustedBox(pMin, pMax);
        voxel.overlap = aggregate.IntersectB(adjustedBox);
    }
}
//#pragma optimize("", on)

void PrtProbeIntegrator::WriteVoxels(const pstd::vector<Voxel> &voxels) {
    std::stringstream ss;

    if (voxels.empty()) {
        LOG_VERBOSE("no voxels generated.");
        return;
    }

    for (const Voxel& voxel : voxels)
    {
        // LOG_VERBOSE("%s", voxel);
        Point3f center = voxel.box.Centroid();
        // inverse X axis to match 3ds max coordinate
        center.x = -center.x;
        ss << voxel.id << " " << center.x << " " << center.y << " " << center.z << " "
           << voxel.overlap << std::endl;
    }

    WriteFileContents("voxels.txt", ss.str());
}


}  // namespace pbrt