#pragma once

#include <pbrt/cpu/integrators.h>


namespace pbrt
{

// Order starts at 0, 0 order has 1 coefficient
constexpr int ShOrder = 2;
// SH coefficient count
constexpr int ShCoefficientCount = (ShOrder + 1) * (ShOrder + 1);

struct Voxel {
    Bounds3f box;
    bool overlap;
    int id;

    std::string ToString() const;
};

struct Probe {
    Point3f pos;
    int id;
    Float density;
    pstd::array<pstd::array<Float, ShCoefficientCount>, 3> coeffs;
};

struct RayGeometryHit {
    unsigned int hitCount = 0;
};

// PrtProbeIntegrator Definition
class PrtProbeIntegrator : public Integrator {
  public:
    // RandomWalkIntegrator Public Methods
    PrtProbeIntegrator(int maxDepth, int volUnit, Float rho, Sampler sampler,
                       Primitive aggregate, std::vector<Light> lights)
        : Integrator(aggregate, lights),
          maxDepth(maxDepth),
          voxelUnit(volUnit),
          samplerPrototype(sampler),
          rhoProbes(rho) {}

    static std::unique_ptr<PrtProbeIntegrator> Create(
        const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
        std::vector<Light> lights, const FileLoc *loc);

    void Render();

    std::string ToString() const;

  private:

    pstd::vector<Voxel> VoxelizeScene();

    void CalcProbeSH(Probe &probe);

    int CoordinateToIndex(Point3f pMin);

    void SurfaceVoxelize(pstd::vector<Voxel> &voxels);

    Point3f OffsetProbeToSurface(Point3f pProbe, Point3f pVoxelMin,
                                 const pstd::vector<Voxel> &voxels);

    pstd::vector<Probe> FloodFillScene(const pstd::vector<Voxel> &voxels);

    int GetTargetProbeCount();

    Float CalcProbeDensity(const Probe &target, const pstd::vector<Probe> &probeList);

    void ReduceProbes(pstd::vector<Probe> &probes);

    void WriteVoxels(const pstd::vector<Voxel> &voxels);
    void WriteProbes(const pstd::vector<Probe> &probes);
    void WriteSampleDirections(const pstd::vector<Vector3f> &dirs);

  private:
    Sampler samplerPrototype;
    int maxDepth;
    int voxelUnit;
    const Float rhoProbes;
};

}  // namespace pbrt