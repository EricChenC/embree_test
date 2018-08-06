#include "EmbreeTest.h"

#include <windows.h>
#include <gl/GL.h>




void error_handler(void* userPtr, const RTCError code, const char* str = nullptr)
{
    if (code == RTC_ERROR_NONE)
        return;

    printf("Embree: ");
    switch (code) {
    case RTC_ERROR_UNKNOWN: printf("RTC_ERROR_UNKNOWN"); break;
    case RTC_ERROR_INVALID_ARGUMENT: printf("RTC_ERROR_INVALID_ARGUMENT"); break;
    case RTC_ERROR_INVALID_OPERATION: printf("RTC_ERROR_INVALID_OPERATION"); break;
    case RTC_ERROR_OUT_OF_MEMORY: printf("RTC_ERROR_OUT_OF_MEMORY"); break;
    case RTC_ERROR_UNSUPPORTED_CPU: printf("RTC_ERROR_UNSUPPORTED_CPU"); break;
    case RTC_ERROR_CANCELLED: printf("RTC_ERROR_CANCELLED"); break;
    default: printf("invalid error code"); break;
    }
    if (str) {
        printf(" (");
        while (*str) putchar(*str++);
        printf(")\n");
    }
    exit(1);
}


EmbreeTest::EmbreeTest()
{
    rtcore_ = "benchmark=1,start_threads=1";
}

EmbreeTest::~EmbreeTest()
{
}

void EmbreeTest::InitDevice()
{
    device_ = rtcNewDevice(rtcore_.c_str());

    error_handler(nullptr, rtcGetDeviceError(device_));

    rtcSetDeviceErrorFunction(device_, error_handler, nullptr);
}

void EmbreeTest::InitScene()
{
    scene_ = rtcNewScene(device_);
}

void EmbreeTest::InitGeometry()
{
    /* add cube */
    addCube(scene_);

    /* add ground plane */
    addGroundPlane(scene_);
}

void EmbreeTest::Render()
{

    for (int h = 0; h < height_; h += TILE_SIZE_Y) {
        for (int w = 0; w < width_; w += TILE_SIZE_X) {

            const int numTilesX = (width_ + TILE_SIZE_X - 1) / TILE_SIZE_X;
            const int numTilesY = (height_ + TILE_SIZE_Y - 1) / TILE_SIZE_Y;

            const unsigned int tileY = taskIndex / numTilesX;
            const unsigned int tileX = taskIndex - tileY * numTilesX;
            const unsigned int x0 = tileX * TILE_SIZE_X;
            const unsigned int x1 = glm::min(x0 + TILE_SIZE_X, width_);
            const unsigned int y0 = tileY * TILE_SIZE_Y;
            const unsigned int y1 = min(y0 + TILE_SIZE_Y, height_);

            Ray rays[TILE_SIZE_X*TILE_SIZE_Y];

            /* generate stream of primary rays */
            int N = 0;
            for (unsigned int y = y0; y<y1; y++) for (unsigned int x = x0; x<x1; x++)
            {
                /* ISPC workaround for mask == 0 */


                RandomSampler sampler;
                RandomSampler_init(sampler, x, y, 0);

                /* initialize ray */
                Ray& ray = rays[N++];
                bool mask = 1; { // invalidates inactive rays
                    ray.tnear() = mask ? 0.0f : (float)(pos_inf);
                    ray.tfar = mask ? (float)(inf) : (float)(neg_inf);
                }
                init_Ray(ray, Vec3fa(camera.xfm.p), Vec3fa(normalize((float)x*camera.xfm.l.vx + (float)y*camera.xfm.l.vy + camera.xfm.l.vz)), ray.tnear(), ray.tfar, RandomSampler_get1D(sampler));

                RayStats_addRay(stats);
            }
            
            
        }
    }




#pragma omp parallel for





    glDrawPixels(width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

}

void EmbreeTest::Init()
{
    InitDevice();
    InitScene();
    InitGeometry();

    rtcCommitScene(scene_);
}

void* EmbreeTest::alignedMalloc(size_t size, size_t align)
{
    if (size == 0)
        return nullptr;

    assert((align & (align - 1)) == 0);
    void* ptr = _mm_malloc(size, align);

    if (size != 0 && ptr == nullptr)
        throw std::bad_alloc();

    return ptr;
}


/* adds a cube to the scene */
unsigned int EmbreeTest::addCube(RTCScene scene_i)
{
    /* create a triangulated cube with 12 triangles and 8 vertices */
    RTCGeometry mesh = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_TRIANGLE);

    /* create face and vertex color arrays */
    face_colors = (Vec3fa*)alignedMalloc(12 * sizeof(Vec3fa), 16);
    vertex_colors = (Vec3fa*)alignedMalloc(8 * sizeof(Vec3fa), 16);

    /* set vertices and vertex colors */
    Vertex* vertices = (Vertex*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 8);
    vertex_colors[0] = Vec3fa(0, 0, 0); vertices[0].x = -1; vertices[0].y = -1; vertices[0].z = -1;
    vertex_colors[1] = Vec3fa(0, 0, 1); vertices[1].x = -1; vertices[1].y = -1; vertices[1].z = +1;
    vertex_colors[2] = Vec3fa(0, 1, 0); vertices[2].x = -1; vertices[2].y = +1; vertices[2].z = -1;
    vertex_colors[3] = Vec3fa(0, 1, 1); vertices[3].x = -1; vertices[3].y = +1; vertices[3].z = +1;
    vertex_colors[4] = Vec3fa(1, 0, 0); vertices[4].x = +1; vertices[4].y = -1; vertices[4].z = -1;
    vertex_colors[5] = Vec3fa(1, 0, 1); vertices[5].x = +1; vertices[5].y = -1; vertices[5].z = +1;
    vertex_colors[6] = Vec3fa(1, 1, 0); vertices[6].x = +1; vertices[6].y = +1; vertices[6].z = -1;
    vertex_colors[7] = Vec3fa(1, 1, 1); vertices[7].x = +1; vertices[7].y = +1; vertices[7].z = +1;

    /* set triangles and face colors */
    int tri = 0;
    Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 12);

    // left side
    face_colors[tri] = Vec3fa(1, 0, 0); triangles[tri].v0 = 0; triangles[tri].v1 = 1; triangles[tri].v2 = 2; tri++;
    face_colors[tri] = Vec3fa(1, 0, 0); triangles[tri].v0 = 1; triangles[tri].v1 = 3; triangles[tri].v2 = 2; tri++;

    // right side
    face_colors[tri] = Vec3fa(0, 1, 0); triangles[tri].v0 = 4; triangles[tri].v1 = 6; triangles[tri].v2 = 5; tri++;
    face_colors[tri] = Vec3fa(0, 1, 0); triangles[tri].v0 = 5; triangles[tri].v1 = 6; triangles[tri].v2 = 7; tri++;

    // bottom side
    face_colors[tri] = Vec3fa(0.5f);  triangles[tri].v0 = 0; triangles[tri].v1 = 4; triangles[tri].v2 = 1; tri++;
    face_colors[tri] = Vec3fa(0.5f);  triangles[tri].v0 = 1; triangles[tri].v1 = 4; triangles[tri].v2 = 5; tri++;

    // top side
    face_colors[tri] = Vec3fa(1.0f);  triangles[tri].v0 = 2; triangles[tri].v1 = 3; triangles[tri].v2 = 6; tri++;
    face_colors[tri] = Vec3fa(1.0f);  triangles[tri].v0 = 3; triangles[tri].v1 = 7; triangles[tri].v2 = 6; tri++;

    // front side
    face_colors[tri] = Vec3fa(0, 0, 1); triangles[tri].v0 = 0; triangles[tri].v1 = 2; triangles[tri].v2 = 4; tri++;
    face_colors[tri] = Vec3fa(0, 0, 1); triangles[tri].v0 = 2; triangles[tri].v1 = 6; triangles[tri].v2 = 4; tri++;

    // back side
    face_colors[tri] = Vec3fa(1, 1, 0); triangles[tri].v0 = 1; triangles[tri].v1 = 5; triangles[tri].v2 = 3; tri++;
    face_colors[tri] = Vec3fa(1, 1, 0); triangles[tri].v0 = 3; triangles[tri].v1 = 5; triangles[tri].v2 = 7; tri++;

    rtcSetGeometryVertexAttributeCount(mesh, 1);
    rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, vertex_colors, 0, sizeof(Vec3fa), 8);

    rtcCommitGeometry(mesh);
    unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
    rtcReleaseGeometry(mesh);
    return geomID;
}

/* adds a ground plane to the scene */
unsigned int EmbreeTest::addGroundPlane(RTCScene scene_i)
{
    /* create a triangulated plane with 2 triangles and 4 vertices */
    RTCGeometry mesh = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_TRIANGLE);

    /* set vertices */
    Vertex* vertices = (Vertex*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 4);
    vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
    vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
    vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
    vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;

    /* set triangles */
    Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 2);
    triangles[0].v0 = 0; triangles[0].v1 = 1; triangles[0].v2 = 2;
    triangles[1].v0 = 1; triangles[1].v1 = 3; triangles[1].v2 = 2;

    rtcCommitGeometry(mesh);
    unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
    rtcReleaseGeometry(mesh);
    return geomID;
}


