// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

template<typename T>
class Vec3
{
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

    Vec3& normalize()
    {
        T nor2 = length2();
        if (nor2 > 0) {
            T invNor = 1 / sqrt(nor2);
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }
    
    Vec3<T> operator * (const T &f) const { return Vec3<T>(x * f, y * f, z * f); }
    Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
    Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
    Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
    Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
    
    T length2() const { return x * x + y * y + z * z; }
    T length() const { return sqrt(length2()); }
    friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v)
    {
        os << "[" << v.x << " " << v.y << " " << v.z << "]";
        return os;
    }
};

typedef Vec3<float> Vec3f;

Vec3f               vec_sub(Vec3f v1, Vec3f v2)
{
    Vec3f           ret;

    ret.x = v1.x - v2.x;
    ret.y = v1.y - v2.y;
    ret.z = v1.z - v2.z;
    return (ret);
}

Vec3f         vec_add(Vec3f v1, Vec3f v2)
{
    Vec3f     ret;

    ret.x = v1.x + v2.x;
    ret.y = v1.y + v2.y;
    ret.z = v1.z + v2.z;
    return (ret);
}

Vec3f         vec_mult_f(Vec3f v, float f)
{
    Vec3<float>     ret;

    ret.x = v.x * f;
    ret.y = v.y * f;
    ret.z = v.z * f;
    return (ret);
}

float               dot_product(Vec3f v1, Vec3f v2)
{
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}



float               vec_length(Vec3f vec)
{
    return (sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z));
}

Vec3f               vec_normalize(Vec3f vec)
{
    Vec3f           ret;
    float           len;

    len = vec_length(vec);
    ret.x = vec.x / len;
    ret.y = vec.y / len;
    ret.z = vec.z / len;
    return (ret);
}

float               max(float a, float b)
{
    if (a > b)
        return (a);
    return (b);
}

//END REPLACE GUNDZ



class Sphere
{
public:
    Vec3f pos;                           /// position of the sphere
    float radius, radius2;                  /// sphere radius and radius^2
    Vec3f surfaceColor, emissionColor;      /// surface color and emission (light)
    float transparency, reflection;         /// surface transparency and reflectivity
    Sphere(
        const Vec3f &c,
        const float &r,
        const Vec3f &sc,
        const float &refl = 0,
        const float &transp = 0,
        const Vec3f &ec = 0) :
        pos(c), radius(r), radius2(r * r), surfaceColor(sc), emissionColor(ec),
        transparency(transp), reflection(refl)
    { /* empty */ }
    //[comment]
    // Compute a ray-sphere intersection using the geometric solution
    //[/comment]
    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
    {
        Vec3f l = vec_sub(pos, rayorig);
        float tca = dot_product(l, raydir);
        if (tca < 0)
            return false;
        float d2 = dot_product(l, l) - tca * tca;
        if (d2 > (radius * radius))
            return false;
        float thc = sqrtf((radius * radius) - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        return true;
    }
};

float mix(const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

int
hitsphere(Vec3f rayorig, Vec3f raydir, Sphere sphere, float &t0, float &t1)
{
    Vec3f l = vec_sub(sphere.pos, rayorig);
    float tca = dot_product(l, raydir);
    if (tca < 0)
        return 0;
    float d2 = dot_product(l, l) - tca * tca;
    if (d2 > (sphere.radius * sphere.radius))
        return 0;
    float thc = sqrtf((sphere.radius * sphere.radius) - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    return (1);
}

Vec3f trace(
    const Vec3f &rayorig,
    const Vec3f &raydir,
    const std::vector<Sphere> &spheres,
    const int &depth)
{
    //if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
    float tnear = INFINITY;
    const Sphere* sphere = NULL;
    // find intersection of this ray with the sphere in the scene
    for (unsigned i = 0; i < spheres.size(); i++)
    {
        float t0 = INFINITY, t1 = INFINITY;
        //if (spheres[i].intersect(rayorig, raydir, t0, t1))
        if (hitsphere(rayorig, raydir, spheres[i], t0, t1) == 1)
        {
            if (t0 < 0)
                t0 = t1;
            if (t0 < tnear)
            {
                tnear = t0;
                sphere = &spheres[i];
            }
        }
    }
    // if there's no intersection return black or background color
    if (!sphere)
        return Vec3f(2.0f);
    Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray

    Vec3f phit = vec_add(rayorig, vec_mult_f(raydir, tnear));
    Vec3f nhit = vec_sub(phit, sphere->pos);
    nhit = vec_normalize(nhit);
    //printf("%f %f %f\n", nhit.x, nhit.y, nhit.z);
    // If the normal and the view direction are not opposite to each other
    // reverse the normal direction. That also means we are inside the sphere so set
    // the inside bool to true. Finally reverse the sign of IdotN which we want
    // positive.



    // this is a light

    // it's a diffuse object, no need to raytrace any further
    for (unsigned i = 0; i < spheres.size(); i++)
    {
        if (spheres[i].emissionColor.x > 0)
        {
            Vec3f transmission = 1;
            Vec3f lightDirection = spheres[i].pos - phit;
            lightDirection.normalize();
            for (unsigned j = 0; j < spheres.size(); j++)
            {
                if (i != j)
                {
                    float t0, t1;
                    if (spheres[j].intersect(phit + nhit, lightDirection, t0, t1))
                    {
                        transmission = 0;
                        break;
                    }
                }
            }
            surfaceColor += sphere->surfaceColor * transmission *
            std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
        }
    }

    return vec_add(surfaceColor, sphere->emissionColor);
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void render(const std::vector<Sphere> &spheres)
{
    unsigned width = 1920.0f, height = 1080.0f;
    Vec3f *image = new Vec3f[width * height], *pixel = image;
    float invWidth = 1.0f / float(width), invHeight = 1.0f / float(height);
    float fov = 30.0f, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5f * fov / 180.0f);
    // Trace rays
    for (unsigned y = 0; y < height; ++y)
    {
        for (unsigned x = 0; x < width; ++x, ++pixel)
        {
            float xx = (2.0f * ((x + 0.5f) * invWidth) - 1.0f) * angle * aspectratio;
            float yy = (1.0f - 2.0f * ((y + 0.5f) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1.0f);
            raydir = vec_normalize(raydir);


            *pixel = trace(Vec3f(0), raydir, spheres, 0);
        }
    }
    // Save result to a PPM image (keep these flags if you compile under Windows)
    std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (unsigned i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
               (unsigned char)(std::min(float(1), image[i].y) * 255) <<
               (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image;
}

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char **argv)
{
    srand48(13);
    std::vector<Sphere> spheres;
    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere(Vec3f( 0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
    spheres.push_back(Sphere(Vec3f( 0.0,      0, -20),     4, Vec3f(1.00, 0.32, 0.36), 1, 0.5));
    spheres.push_back(Sphere(Vec3f( 5.0,     -1, -15),     2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
    spheres.push_back(Sphere(Vec3f( 5.0,      0, -25),     3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
    spheres.push_back(Sphere(Vec3f(-5.5,      0, -15),     3, Vec3f(0.90, 0.90, 0.90), 1, 0.0));
    // light
    spheres.push_back(Sphere(Vec3f( 0.0,     20, -30),     3, Vec3f(0.00, 0.00, 0.00), 0, 0.0, Vec3f(3)));
    render(spheres);

    return 0;
}