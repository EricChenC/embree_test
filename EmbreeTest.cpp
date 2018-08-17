#include "EmbreeTest.h"


#include "../common/algorithms/parallel_for.h"
#include "../common/tasking/taskschedulertbb.h"



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
    : v1_(0.0f, 2.0f, 6.0f)
    , v2_(0.0f, 0.0f, 0.0f)
{
    rtcore_ = "benchmark=1,start_threads=1";
}

EmbreeTest::~EmbreeTest()
{
}

void EmbreeTest::InitWindow()
{

    if (glfwInit() != GLFW_TRUE) return;

    std::cout << "glfw Init success!\n";
    

    if(glewInit() != GLFW_TRUE) return;

    std::cout << "glew Init success!\n";


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window_ = glfwCreateWindow(width_, height_, "embree3 test", nullptr, nullptr);
    /*glfwSetKeyCallback(window, embree::keyboardFunc);
    glfwSetCursorPosCallback(window, embree::motionFunc);
    glfwSetMouseButtonCallback(window, embree::clickFunc);
    glfwSetWindowSizeCallback(window, embree::reshapeFunc);*/
    resize(width_, height_);

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    reshapeFunc(window_, 0, 0);

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

void EmbreeTest::Init()
{
    InitWindow();
    InitDevice();
    InitScene();
    InitGeometry();

    rtcCommitScene(scene_);


    while (!glfwWindowShouldClose(window_))
    {
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        Render();
    }

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

void EmbreeTest::Render()
{

    const int numTilesX = (width_ + TILE_SIZE_X - 1) / TILE_SIZE_X;
    const int numTilesY = (height_ + TILE_SIZE_Y - 1) / TILE_SIZE_Y;
    parallel_for(size_t(0), size_t(numTilesX*numTilesY), [&](const range<size_t>& range) {
        const int threadIndex = (int)TaskScheduler::threadIndex();
        for (size_t i = range.begin(); i<range.end(); i++)
            renderTileStandard((int)i, threadIndex, (int*)pixels_, width_, height_, v1_, v2_, numTilesX, numTilesY);
    });

    glDrawPixels(width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels_);

    glfwSwapBuffers(window_);

}

void EmbreeTest::renderTileStandard(int taskIndex, int threadIndex, int * pixels, const unsigned int width, const unsigned int height, const Vec3fa & v1, const Vec3fa & v2, const int numTilesX, const int numTilesY)
{

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


        /* initialize ray */
        Ray& ray = rays[N++];
        bool mask = 1; { // invalidates inactive rays
            ray.tnear() = mask ? 0.0f : (float)(1000000);
            ray.tfar = mask ? (float)(1000000) : (float)(-100000);
        }
        init_Ray(ray, Vec3fa(4.18, 2.806, -1.19), Vec3fa(normalize((float)x*Vec3fa(-0.09, 0.0, -0.99) + (float)y*Vec3fa(-0.437, 0.89, 0.04) + Vec3fa(-92.36, -342.51, 266.00))), ray.tnear(), ray.tfar, y*x);

    }


    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    /* trace stream of rays */
    rtcIntersect1M(scene_, &context, (RTCRayHit*)&rays[0], N, sizeof(Ray));


    /* shade stream of rays */
    N = 0;
    for (unsigned int y = y0; y<y1; y++) for (unsigned int x = x0; x<x1; x++)
    {
        /* ISPC workaround for mask == 0 */

        Ray& ray = rays[N++];

        /* eyelight shading */
        Vec3fa color = Vec3fa(0.0f);
        if (ray.geomID != RTC_INVALID_GEOMETRY_ID)
        {
            Vec3fa diffuse = face_colors[ray.primID];
            color = color + diffuse * 0.5f;
            Vec3fa lightDir = normalize(Vec3fa(1, -1, -1));

            /* initialize shadow ray */
            Ray shadow(ray.org + ray.tfar*ray.dir, -lightDir, 0.001f, 1000000.0f, 0.0f);

            /* trace shadow ray */
            rtcOccluded1(scene_, &context, (RTCRay*)&shadow);

            /* add light contribution */
            if (shadow.tfar >= 0.0f)
                color = color + diffuse * clamp(-dot(lightDir, normalize(ray.Ng)), 0.0f, 1.0f);

        }

        /* write color to framebuffer */
        unsigned int r = (unsigned int)(255.0f * clamp(color.x, 0.0f, 1.0f));
        unsigned int g = (unsigned int)(255.0f * clamp(color.y, 0.0f, 1.0f));
        unsigned int b = (unsigned int)(255.0f * clamp(color.z, 0.0f, 1.0f));
        pixels[y*width + x] = (b << 16) + (g << 8) + r;
    }

}

void EmbreeTest::resize(unsigned width, unsigned height)
{

    //if (pixels_) _aligned_free(pixels_);
    width_ = width;
    height_ = height;
    pixels_ = (unsigned*)alignedMalloc(width*height * sizeof(unsigned), 64);
}


