#pragma once

#define TASKING_TBB
#define __SSE__


#include "embree3/rtcore.h"
#include "embree3/rtcore_scene.h"
#include "embree3/rtcore_buffer.h"
#include "embree3/rtcore_ray.h"
#include "embree3/rtcore_common.h"
#include "embree3/rtcore_geometry.h"
#include "embree3/rtcore_device.h"

#include "../common/math/vec3.h"
#include "../common/math/vec3fa.h"
#include "../common/math/constants.h"


#include "GL/glew.h"
#include <glfw/glfw3.h>

#include <string>
#include "glm/glm.hpp"


#define TILE_SIZE_X 8
#define TILE_SIZE_Y 8

using namespace embree;


class EmbreeTest {
public:
    struct Vertex { float x, y, z, r; };
    struct Triangle { int v0, v1, v2; };

    EmbreeTest();
    ~EmbreeTest();

    void Init();
    void Render();

public:
    void* alignedMalloc(size_t size, size_t align);

    unsigned int addCube(RTCScene scene_i);

    unsigned int addGroundPlane(RTCScene scene_i);

    void InitWindow();
    void InitDevice();
    void InitScene();
    void InitGeometry();

public:
    /*! Ray structure. */
    struct  Ray
    {
        /*! Default construction does nothing. */
        __forceinline Ray() {}

        /*! Constructs a ray from origin, direction, and ray segment. Near
        *  has to be smaller than far. */
        __forceinline Ray(const embree::Vec3fa& org,
            const embree::Vec3fa& dir,
            float tnear = 0,
            float tfar = embree::inf,
            float time = 0,
            int mask = -1,
            unsigned int geomID = RTC_INVALID_GEOMETRY_ID,
            unsigned int primID = RTC_INVALID_GEOMETRY_ID,
            unsigned int instID = RTC_INVALID_GEOMETRY_ID)
            : org(org, tnear), dir(dir, time), tfar(tfar), mask(mask), primID(primID), geomID(geomID), instID(instID) {}

        /*! Tests if we hit something. */
        __forceinline operator bool() const { return geomID != RTC_INVALID_GEOMETRY_ID; }

    public:
        embree::Vec3fa org;       //!< Ray origin + tnear
                                  //float tnear;              //!< Start of ray segment
        embree::Vec3fa dir;        //!< Ray direction + tfar
                                   //float time;               //!< Time of this ray for motion blur.
        float tfar;               //!< End of ray segment
        unsigned int mask;        //!< used to mask out objects during traversal
        unsigned int id;          //!< ray ID
        unsigned int flags;       //!< ray flags

    public:
        embree::Vec3f Ng;         //!< Not normalized geometry normal
        float u;                  //!< Barycentric u coordinate of hit
        float v;                  //!< Barycentric v coordinate of hit
        unsigned int primID;           //!< primitive ID
        unsigned int geomID;           //!< geometry ID
        unsigned int instID;           //!< instance ID

        __forceinline float &tnear() { return org.w; };
        __forceinline float &time() { return dir.w; };
        __forceinline float const &tnear() const { return org.w; };
        __forceinline float const &time()  const { return dir.w; };

    };


    __forceinline void init_Ray(Ray &ray,
        const embree::Vec3fa& org,
        const embree::Vec3fa& dir,
        float tnear = 0,
        float tfar = embree::inf,
        float time = 0,
        int mask = -1,
        unsigned int geomID = RTC_INVALID_GEOMETRY_ID,
        unsigned int primID = RTC_INVALID_GEOMETRY_ID,
        unsigned int instID = RTC_INVALID_GEOMETRY_ID)
    {
        ray = Ray(org, dir, tnear, tfar, time, mask, geomID, primID, instID);
    }


    void renderTileStandard(int taskIndex,
        int threadIndex,
        int* pixels,
        const unsigned int width,
        const unsigned int height,
        const Vec3fa& v1,
        const Vec3fa& v2,
        const int numTilesX,
        const int numTilesY);


    void resize(unsigned width, unsigned height);


    void reshapeFunc(GLFWwindow* window, int, int)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        resize(width, height);
        glViewport(0, 0, width, height);
        width_ = width; height_ = height;
    }


private:
    RTCDevice device_;

    RTCScene scene_;

    std::string rtcore_;

    Vec3fa* face_colors = nullptr;

    Vec3fa* vertex_colors = nullptr;

    Vec3fa v1_;

    Vec3fa v2_;

    unsigned int width_ = 800;

    unsigned int height_ = 600;

    unsigned* pixels_;

    GLFWwindow* window_;

};