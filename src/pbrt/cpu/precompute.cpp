#include <pbrt/cpu/precompute.h>
#include <ext/spherical-harmonics/spherical_harmonics.h>
#include <pbrt/util/file.h>
#include <atomic>

namespace pbrt
{

#pragma optimize("", off)

std::string Voxel::ToString() const {
    return StringPrintf("[ Voxel id %d center: %s overlap: %d ]", id, box.Centroid(), overlap);
}

std::unique_ptr<PrtProbeIntegrator> PrtProbeIntegrator::Create(
    const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
    std::vector<Light> lights, const FileLoc *loc) {
    int maxDepth = parameters.GetOneInt("maxdepth", 5);
    int volUnit = parameters.GetOneInt("volunit", 1);
    Float rhoProbes = parameters.GetOneFloat("rhoprobes", 10.0);
    return std::make_unique<PrtProbeIntegrator>(maxDepth, volUnit, rhoProbes, sampler,
                                                aggregate, lights);
}

void PrtProbeIntegrator::Render() {

    //auto voxels = VoxelizeScene();

    //auto probes = FloodFillScene(voxels);

    //ReduceProbes(probes);

    CalcProbeSH(Probe());

    //WriteVoxels(voxels);
    //WriteProbes(probes);
}

std::string PrtProbeIntegrator::ToString() const {
    return StringPrintf("[ PrtProbeIntegrator maxDepth: %d ]", maxDepth);
}

pstd::vector<pbrt::Voxel> PrtProbeIntegrator::VoxelizeScene() {

    pstd::vector<pbrt::Voxel> voxels;

    Bounds3f bounds = aggregate.Bounds();
    Vector3f diagonal = bounds.Diagonal();
    int xres = std::lround(diagonal.x);
    int yres = std::lround(diagonal.y);
    int zres = std::lround(diagonal.z);

    int voxelId = 0;
    for (int x = 0; x < xres; x += voxelUnit) 
    {
        for (int y = 0; y < yres; y += voxelUnit) 
        {
            for (int z = 0; z < zres; z += voxelUnit)
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

void PrtProbeIntegrator::CalcProbeSH(Probe &probe) {
    Point2i pPixel(0, 0);
    int sampleIndex = 0;

    ScratchBuffer scratchBuffer(65536);
    Sampler sampler = samplerPrototype.Clone(Allocator());
    
    int sampleCount = sampler.SamplesPerPixel();


    for (int sampleIndex = 0; sampleIndex != sampleCount; ++sampleIndex) {
        sampler.StartPixelSample(pPixel, sampleIndex);
        Point2f u = sampler.Get2D();
        Vector3f wp = SampleUniformSphere(u);

    }

    // WriteSampleDirections(dirs);
}

int PrtProbeIntegrator::CoordinateToIndex(Point3f pMin) {
    // Convert world space coordinate to voxel index
    // Input position is pMin of voxel box

    Bounds3f bounds = aggregate.Bounds();
    Vector3f diagonal = bounds.Diagonal();
    int xres = std::lround(diagonal.x);
    int yres = std::lround(diagonal.y);
    int zres = std::lround(diagonal.z);

    Point3f voxelPos = Point3f(pMin - bounds.pMin) / voxelUnit;

    if (voxelPos.x < 0 || voxelPos.y < 0 || voxelPos.z < 0) {
        return -1;
    }

    if (voxelPos.x >= (xres / voxelUnit) || voxelPos.y >= (yres / voxelUnit) ||
        voxelPos.z >= (zres / voxelUnit)) {
        return -1;
    }

    int index = std::lround(voxelPos.z) +
                std::lround(zres / voxelUnit) * std::lround(voxelPos.y) +
                std::lround(zres / voxelUnit) * std::lround(yres / voxelUnit) *
                    std::lround(voxelPos.x);
    return index;
}

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


pbrt::Point3f PrtProbeIntegrator::OffsetProbeToSurface(
    Point3f pProbe, Point3f pVoxelMin, const pstd::vector<Voxel> &voxels) {

    // There values must be equal to those in FloodFillScene
    const Vector3f neighbours[6] = {{0.0, 0.0, 1.0},  {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0},
                                    {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},  {0.0, -1.0, 0.0}};

    // Offset a little towards surface
    
    // average the direction
    Point3f sumOffset = {};
    for (int j = 0; j != 6; ++j) {
        Point3f pTargetMin = pVoxelMin + neighbours[j];
        int targetIndex = CoordinateToIndex(pTargetMin);
        const Voxel &nb = voxels[targetIndex];
        if (nb.overlap) {
            sumOffset += neighbours[j];
        }
    }

    Point3f pNewCenter = pProbe + (sumOffset * Float(0.3));
    return pNewCenter;
}

pstd::vector<pbrt::Probe> PrtProbeIntegrator::FloodFillScene(
    const pstd::vector<Voxel> &voxels) {

    pstd::vector<pbrt::Probe> probes;

    const Vector3f neighbours[6] = {{0.0, 0.0, 1.0},  {0.0, 0.0, -1.0}, {1.0, 0.0, 0.0},
                                    {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},  {0.0, -1.0, 0.0}};

    int probeIndex = 0;
    int vxIndex = 0;
    for (const auto &v : voxels) {

        if (v.overlap) {
            ++vxIndex;
            continue;
        }

        for (int i = 0; i != 6; ++i)
        {
            Point3f pTargetMin = v.box.pMin + neighbours[i];
            int targetIndex = CoordinateToIndex(pTargetMin);

            int test = CoordinateToIndex(v.box.pMin);

            if (targetIndex < 0 || targetIndex >= voxels.size()) {
                continue;
            }

            const Voxel &nv = voxels[targetIndex];
            if (nv.overlap) {
                // probe center
                Point3f voxelCenter = v.box.Centroid();
                Point3f probeCenter =
                    OffsetProbeToSurface(voxelCenter, v.box.pMin, voxels);

                probes.emplace_back(Probe{probeCenter, probeIndex});
                ++probeIndex;
                break;
            }
        }
        ++vxIndex;
    }

    return probes;
}


int PrtProbeIntegrator::GetTargetProbeCount() {

    // To avoid introducing additional parameters,
    // we set the target probe count to the number of points in a regular grid that
    // covers the scene with grid spacing set to ¦Ñ_probes
    Bounds3f bounds = aggregate.Bounds();
    Vector3f diagonal = bounds.Diagonal();

    int count = std::lround(std::max(diagonal.x / rhoProbes, Float(1.0))) *
                std::lround(std::max(diagonal.y / rhoProbes, Float(1.0))) *
                std::lround(std::max(diagonal.z / rhoProbes, Float(1.0)));
    return count;
}

Float PrtProbeIntegrator::CalcProbeDensity(const Probe &target,
                                           const pstd::vector<Probe> &probeList) {
    auto CalcSingleDensity = [&](const Probe &x, const Probe &p) -> Float {
        Float t = Distance(x.pos, p.pos) / rhoProbes;

        if (t < 0.0 || t > 1.0) {
            return 0.0;
        }

        Float d = 2.0 * std::powf(t, 3.0) - 3 * std::powf(t, 2.0) + 1.0;
        return d;
    };

    Float resultDensity = 0.0;
    for (const auto &p : probeList) {
        resultDensity += CalcSingleDensity(target, p);
    }

    return resultDensity;
}

void PrtProbeIntegrator::ReduceProbes(pstd::vector<Probe> &probes) {
    int nTargetProbeCount = GetTargetProbeCount();

    while (true) {
        // Update probe density
        size_t probeCount = probes.size();
        ParallelFor(0, probeCount, [&](int i) {
            Probe &p = probes[i];
            p.density = CalcProbeDensity(p, probes);
        });

        // sort probes from density min to max
        std::sort(probes.begin(), probes.end(), [](const Probe &lhs, const Probe &rhs) {
            return lhs.density < rhs.density;
        });

        // remove the largest density probe
        probes.pop_back();

        if (probes.size() <= nTargetProbeCount) {
            break;
        }
    }
}

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


void PrtProbeIntegrator::WriteProbes(const pstd::vector<Probe> &probes) {
    std::stringstream ss;

    if (probes.empty()) {
        LOG_VERBOSE("no probes generated.");
        return;
    }

    for (const Probe &probe : probes) {
        // LOG_VERBOSE("%s", voxel);
        Point3f center = probe.pos;
        // inverse X axis to match 3ds max coordinate
        center.x = -center.x;
        ss << probe.id << " " << center.x << " " << center.y << " " << center.z
           << std::endl;
    }

    WriteFileContents("probes.txt", ss.str());
}

void PrtProbeIntegrator::WriteSampleDirections(const pstd::vector<Vector3f> &dirs) {
    if (dirs.empty()) {
        LOG_VERBOSE("no dirs generated.");
        return;
    }

    std::stringstream ss;
    for (const Vector3f &dir : dirs) {
        ss << dir.x << " " << dir.y << " " << dir.z << std::endl;
    }

    WriteFileContents("sample_dirs.txt", ss.str());
}

#pragma optimize("", on)

}  // namespace pbrt