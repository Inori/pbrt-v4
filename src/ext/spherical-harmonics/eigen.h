#pragma once

#include <array>
#include <cmath>

// Simple implementation of Eigen vectors and arrays
// just used for google spherical-harmonics library

namespace Eigen
{
    struct Vector2d
    {
    public:
        Vector2d(double x, double y)
        {
            elements[0] = x;
            elements[1] = y;
        }

        ~Vector2d() = default;

        double x() const { return elements[0]; }
        double y() const { return elements[1]; }

        double squaredNorm() const
        {
            return std::pow(elements[0], 2) + std::pow(elements[1], 2);
        }
    private:
        std::array<double, 2> elements = {};
    };

    struct Vector3d
    {
    public:
        Vector3d(double x, double y, double z)
        {
            elements[0] = x;
            elements[1] = y;
            elements[2] = z;
        }
        ~Vector3d() = default;

        double x() const { return elements[0]; }
        double y() const { return elements[1]; }
        double z() const { return elements[2]; }

        double squaredNorm() const
        {
            return std::pow(elements[0], 2) + std::pow(elements[1], 2) + std::pow(elements[2], 2);
        }
    private:
        std::array<double, 3> elements = {};
    };

    struct Array3f
    {
        friend inline Array3f operator+(const Array3f&, const Array3f&);
        friend inline Array3f operator*(const double&, const Array3f&);
    public:
        Array3f() = default;

        Array3f(float x, float y, float z)
        {
            elements[0] = x;
            elements[1] = y;
            elements[2] = z;
        }
        ~Array3f() = default;

        float x() const { return elements[0]; }
        float y() const { return elements[1]; }
        float z() const { return elements[2]; }

        Array3f array()
        {
            return *this;
        }

        static Array3f Zero()
        {
            return Array3f();
        }
    private:
        std::array<float, 3> elements = {};
    };

    inline Array3f operator+(const Array3f& a, const Array3f& b)
    {
        return Array3f(a.elements[0] + b.elements[0],
                       a.elements[1] + b.elements[1],
                       a.elements[2] + b.elements[2]);
    }

    inline Array3f operator+=(Array3f& a, const Array3f& b)
    {
        a = a + b;
        return a;
    }

    inline Array3f operator*(const double& a, const Array3f& b)
    {
        return Array3f(a * b.elements[0], a * b.elements[1], a * b.elements[2]);
    }
}  // namespace Eigen