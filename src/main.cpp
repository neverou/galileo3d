#include "std.h"

#include <sdl/SDL.h>
#include <glad/glad.h>

#include "maths.h"

#include "qb.h"

struct {
    bool running;
} engineState;

#include <string.h>
// #include <signal.h>
#include <stdio.h>
#define ASSERT(x) { if (!(x)) { printf("Assert failed! Ln [%ul] in file %s\n", __LINE__, __FILE__); /* raise(SIGTRAP); */ } }


struct Blob {
    u8* data;
    size_t size;
};

Blob allocBlob(size_t size) {
    Blob blob;
    blob.size = size;
    blob.data = (u8*)malloc(size);
    return blob;
}

void freeBlob(Blob blob) {
    free(blob.data);
}

// normally id pass an allocator
Blob readFile(const char* filePath) {
    FILE* file = fopen(filePath, "rb");
    ASSERT(file);

    Blob blob;

    size_t fileSize; 
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    blob = allocBlob(fileSize);
    fseek(file, 0, SEEK_SET);
    fread(blob.data, 1, fileSize, file);

    fclose(file);

    return blob;
}

struct Camera {
    Vec3 position;
    Vec3 rotation;
};

struct {
    Camera camera;
    float time;
} appData;

struct UniformPassData {
    Mat4 inCameraOrientation;
    Mat4 inLastCameraOrientation;
    Vec2 inScreenSize; 
    Vec2 inBlurDir;
    float inTime;
};

struct Voxel {
    Vec4 color;
    Vec4 emission;
};

struct WorldRawData {
    int xSize;
    int ySize;
    int zSize;
    
    int _padding;
};

struct WorldData {
    int xSize;
    int ySize;
    int zSize;
    
    int _padding;
};

void CreateWorldData(int xSize, int ySize, int zSize, WorldData** worldData) {
    WorldData* worldDataMem = (WorldData*)malloc(sizeof(WorldData) + xSize * ySize * zSize * sizeof(Voxel));
    worldDataMem->xSize = xSize;
    worldDataMem->ySize = ySize;
    worldDataMem->zSize = zSize;
    *worldData = worldDataMem;
}

void FreeWorldData(WorldData* worldData) {
    free(worldData);
}

Voxel* GetVoxelData(WorldData* worldData) {
    return (Voxel*)(worldData + 1);
}

size_t GetWorldDataSize(WorldData* worldData) {
    return sizeof(WorldData) + worldData->xSize * worldData->ySize * worldData->zSize * sizeof(Voxel);
}

void UploadUniformData(GLuint ubo, UniformPassData data) {
    void* bufferData = glMapNamedBuffer(ubo, GL_WRITE_ONLY);
    memcpy(bufferData, &data, sizeof(UniformPassData)); 
    glUnmapNamedBuffer(ubo);
}

GLuint CreateShader(const char* vertexPath, const char* pixelPath) {
    GLuint shaderProgram = glCreateProgram();

    // load shader sources
    Blob vertexShaderData = readFile(vertexPath);
    Blob pixelShaderData = readFile(pixelPath);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &vertexShader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, vertexShaderData.data, vertexShaderData.size);
    glShaderBinary(1, &pixelShader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, pixelShaderData.data, pixelShaderData.size);

    glSpecializeShader(vertexShader, "main", 0, 0, 0);
    glSpecializeShader(pixelShader, "main", 0, 0, 0);

    {
        s32 success = false;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            constexpr size_t LOG_SIZE = 4096;
            char log[LOG_SIZE];
            glGetShaderInfoLog(vertexShader, LOG_SIZE, NULL, log);
            printf("Vertex Shader Error: %s\n", log);
        }
    }

    {
        s32 success = false;
        glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            constexpr size_t LOG_SIZE = 4096;
            char log[LOG_SIZE];
            glGetShaderInfoLog(pixelShader, LOG_SIZE, NULL, log);
            printf("Pixel Shader Error: %s\n", log);
        }
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, pixelShader);
    glLinkProgram(shaderProgram);

        {
        s32 success = false;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            constexpr size_t LOG_SIZE = 4096;
            char log[LOG_SIZE];
            glGetProgramInfoLog(shaderProgram, LOG_SIZE, NULL, log);
            printf("Program Error: %s\n", log);
        }
    }


    glDeleteShader(vertexShader);
    glDeleteShader(pixelShader);

    return shaderProgram;
}

struct RenderTarget {
    GLuint fbo;

    u32 attachments;
    GLuint* textures;
    GLuint sampler; // Sampler shouldnt go here but im too lazy to move it for this project lol
};

RenderTarget CreateRenderTarget(u32 width, u32 height, u32 attachments) {
    RenderTarget target;
    glCreateFramebuffers(1, &target.fbo);
    
    target.attachments = attachments;
    target.textures = (GLuint*)malloc(sizeof(GLuint) * attachments);
    

    glCreateTextures(GL_TEXTURE_2D, attachments, target.textures);
    glCreateSamplers(1, &target.sampler);

    glSamplerParameteri(target.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(target.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    for (u32 i = 0; i < attachments; i++) {
        glTextureStorage2D(target.textures[i], 1, GL_RGBA16F, width, height);

        glNamedFramebufferTexture(target.fbo, GL_COLOR_ATTACHMENT0 + i, target.textures[i], 0);
    }
    return target;
}

void BindRenderTarget(RenderTarget target) {
    // for (u32 i = 0; i < target.attachments; i++) {
    //     glBindSampler(i, target.sampler);
    //     glBindTextureUnit(i, target.textures[i]);
    // }

    GLenum buffers[target.attachments];
    for (u32 i = 0; i < target.attachments; i++) {
        buffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
    glDrawBuffers(target.attachments, buffers);
}

void UnbindRenderTarget() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FreeRenderTarget(RenderTarget target) {
    glDeleteTextures(target.attachments, target.textures);
    free(target.textures);
    glDeleteSamplers(1, &target.sampler);
    glDeleteFramebuffers(1, &target.fbo);
}

#define BIND_TEX_UNIT(unit, fbo, idx) glBindSampler(unit, fbo.sampler); glBindTextureUnit(unit, fbo.textures[idx]);

s32 main(int argc, char** argv) {
    u32 windowWidth = 1920;
    u32 windowHeight = 1080;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_Window* window = SDL_CreateWindow("Galileo3D", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        windowWidth, windowHeight, 
        SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE*/);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    // create gl context
    ASSERT(gladLoadGLLoader(SDL_GL_GetProcAddress));

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));




    // lololololol
    GLuint globalVao;
    glCreateVertexArrays(1, &globalVao);
    glBindVertexArray(globalVao);

    // vbo
    GLuint quadVbo;
    {
        float vertices[] = {
            -1, -1,
            -1,  1,
            1,  1,
            
            -1, -1,
            1,  1,
            1, -1,
        };

        glCreateBuffers(1, &quadVbo);
        glNamedBufferStorage(quadVbo, sizeof(vertices), vertices, 0);
    }


    // mega shader!! lol
    GLuint renderShader = CreateShader("data/shaders/render.vert.spv", "data/shaders/render.pix.spv");
    GLuint temporalShader = CreateShader("data/shaders/filter_temporal.vert.spv", "data/shaders/filter_temporal.pix.spv");
    GLuint spatialShader = CreateShader("data/shaders/filter_spatial.vert.spv", "data/shaders/filter_spatial.pix.spv");
    GLuint blitShader = CreateShader("data/shaders/blit.vert.spv", "data/shaders/blit.pix.spv");
    GLuint compositeShader = CreateShader("data/shaders/composite.vert.spv", "data/shaders/composite.pix.spv");
  
    GLuint ubo;
    {
        glCreateBuffers(1, &ubo);
        glNamedBufferStorage(ubo, sizeof(UniformPassData), NULL, GL_MAP_WRITE_BIT);
    }

    RenderTarget fboGbuffer = CreateRenderTarget(windowWidth, windowHeight, 4);

    RenderTarget fboTemporalIndirect = CreateRenderTarget(windowWidth, windowHeight, 1);
    RenderTarget fboLastIndirect = CreateRenderTarget(windowWidth, windowHeight, 1);
    RenderTarget fboSpatialIndirectInterm = CreateRenderTarget(windowWidth, windowHeight, 1);
    RenderTarget fboSpatialIndirect = CreateRenderTarget(windowWidth, windowHeight, 1);



    GLuint worldDataBuffer;
    WorldData* worldData;
    Voxel* worldDataVoxels;
    {
        // CreateWorldData(256, 256, 256, &worldData);
        // worldDataVoxels = GetVoxelData(worldData);

        // // Jank world data upload
        // for (s32 z = 0; z < worldData->zSize; z++) 
        //     for (s32 y = 0; y < worldData->ySize; y++)  
        //         for (s32 x = 0; x < worldData->xSize; x++)  {
        //             Voxel voxel;

        //             bool h = (x < 32 && z < 32); 
        //             if (!h && y > m_sinf(x + z) * 10 + 10) {
        //                 voxel.color = Vec4 { 0, 0, 0, 0 };
        //             }
        //             else {
        //                 voxel.color = Vec4 { rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1 };
        //             }
                    

        //             worldDataVoxels[x + y * worldData->ySize + z * worldData->xSize * worldData->ySize] = voxel;
                    
        //             int a = 123+ 123;
        //         }

        VoxelModel vmdl;
        
        char* modelPath;
        if (argc <= 1)
        {
        	printf("Missing scene argument, any qb file will work. Hint: there are some pre-existing files in data/models/\n");
        	return 1;
        }
         
        
        modelPath = argv[1];
        LoadVoxelModel(modelPath, &vmdl);

		int sX = 1;
		int sZ = 1;
        CreateWorldData(vmdl.sizeX * sX, vmdl.sizeY, vmdl.sizeZ * sZ, &worldData);
        printf("%f MB\n", worldData->xSize * worldData->ySize * worldData->zSize * sizeof(Voxel) / 1000000.0F);
        worldDataVoxels = GetVoxelData(worldData);
        for (s32 z = 0; z < vmdl.sizeZ; z++) 
            for (s32 y = 0; y < vmdl.sizeY; y++)  
                for (s32 x = 0; x < vmdl.sizeX; x++)  {
                    // u32 i = x + y * worldData->xSize + z * worldData->xSize * worldData->ySize;
                    u32 vox = vmdl.voxels[x + y * vmdl.sizeX + z * vmdl.sizeX * vmdl.sizeY];
                    Voxel voxel = {};

                    voxel.color = Vec4 { 
                        ((vox) & 0xFF) / 255.0F,
                        ((vox >> 8) & 0xFF) / 255.0F,
                        ((vox >> 16) & 0xFF) / 255.0F,
                        ((vox >> 24) & 0xFF) / 255.0F,
                    };
                    if (voxel.color.w > 0 && voxel.color.x > voxel.color.y)
                        voxel.emission = voxel.color;

                    for (int j = 0; j < sZ; j++)
                        for (int i = 0; i < sX; i++)
                            worldDataVoxels[(x + i * vmdl.sizeX) + y * worldData->xSize + (z + j * vmdl.sizeZ) * worldData->xSize * worldData->ySize] = voxel;
                }

        // for (s32 z = 0; z < worldData->zSize; z++) 
        //     for (s32 y = 0; y < worldData->ySize; y++)  
        //         for (s32 x = 0; x < worldData->xSize; x++)  {
        //             Voxel voxel;

        //             if (y > rand() / (float)RAND_MAX * 50 - 10) {
        //                 voxel.color = Vec4 { 0, 0, 0, 0 };
        //             }
        //             else {
        //                 voxel.color = Vec4 { rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1 };
        //                 voxel.color = vec_mul(v4(((float)y / 40)), voxel.color);
        //                 if (vec_length(voxel.color) > 1)
        //                     voxel.emission = voxel.color;

        //             }
                    
        //             worldDataVoxels[x + y * worldData->ySize + z * worldData->xSize * worldData->ySize] = voxel;
        //         }
    


        glCreateBuffers(1, &worldDataBuffer);
        glNamedBufferStorage(worldDataBuffer, GetWorldDataSize(worldData), worldData, GL_MAP_WRITE_BIT);
    }

    SDL_Joystick *joystick;
    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);
    struct {
        Vec2 leftStick;
        Vec2 rightStick;
        float leftTrigger;
        float rightTrigger;
    } joystickState;

    Mat4 lastCameraOrientation;

    engineState.running = true;
    float deltaTime = 0.0f;
    while (engineState.running) {
        u32 startTime = SDL_GetTicks();


        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    engineState.running = false;
                    break;
                }

                case SDL_KEYDOWN: {
                    if (event.key.keysym.sym == SDLK_q)
                        appData.camera.position = v3(0, 3, 0);

					if (event.key.keysym.sym == SDLK_w) {
						joystickState.leftStick.y = 1;
					}
					if (event.key.keysym.sym == SDLK_s) {
						joystickState.leftStick.y = -1;
					}
					if (event.key.keysym.sym == SDLK_a) {
						joystickState.leftStick.x = -1;
					}
					if (event.key.keysym.sym == SDLK_d) {
						joystickState.leftStick.x = 1;
					}

					if (event.key.keysym.sym == SDLK_UP) {
						joystickState.rightStick.y = 1;
					}
					if (event.key.keysym.sym == SDLK_DOWN) {
						joystickState.rightStick.y = -1;
					}
					if (event.key.keysym.sym == SDLK_LEFT) {
						joystickState.rightStick.x = -1;
					}
					if (event.key.keysym.sym == SDLK_RIGHT) {
						joystickState.rightStick.x = 1;
					}

					
                    break;
                }

				case SDL_KEYUP: {
                    if (event.key.keysym.sym == SDLK_q)
                        appData.camera.position = v3(0, 3, 0);

					if (event.key.keysym.sym == SDLK_w) {
						joystickState.leftStick.y = 0;
					}
					if (event.key.keysym.sym == SDLK_s) {
						joystickState.leftStick.y = 0;
					}
					if (event.key.keysym.sym == SDLK_a) {
						joystickState.leftStick.x = 0;
					}
					if (event.key.keysym.sym == SDLK_d) {
						joystickState.leftStick.x = 0;
					}

					if (event.key.keysym.sym == SDLK_UP) {
						joystickState.rightStick.y = 0;
					}
					if (event.key.keysym.sym == SDLK_DOWN) {
						joystickState.rightStick.y = 0;
					}
					if (event.key.keysym.sym == SDLK_LEFT) {
						joystickState.rightStick.x = 0;
					}
					if (event.key.keysym.sym == SDLK_RIGHT) {
						joystickState.rightStick.x = 0;
					}

					
                    break;
                }

					
//                 case SDL_JOYAXISMOTION: {
//                     constexpr u8 AXIS_LEFT_X = 0;
//                     constexpr u8 AXIS_LEFT_Y = 1;
// 
//                     constexpr u8 AXIS_RIGHT_X = 3;
//                     constexpr u8 AXIS_RIGHT_Y = 4;
// 
//                     constexpr u8 AXIS_TRIGGER_LEFT = 2;
//                     constexpr u8 AXIS_TRIGGER_RIGHT = 5;
// 
//                     if (event.jaxis.axis == AXIS_LEFT_X) joystickState.leftStick.x = event.jaxis.value / 32767.0F;
//                     if (event.jaxis.axis == AXIS_LEFT_Y) joystickState.leftStick.y = -event.jaxis.value / 32767.0F;
// 
//                     if (event.jaxis.axis == AXIS_RIGHT_X) joystickState.rightStick.x = event.jaxis.value / 32767.0F;
//                     if (event.jaxis.axis == AXIS_RIGHT_Y) joystickState.rightStick.y = -event.jaxis.value / 32767.0F;
// 
//                     if (event.jaxis.axis == AXIS_TRIGGER_LEFT) joystickState.leftTrigger = event.jaxis.value / 32767.0F;
//                     if (event.jaxis.axis == AXIS_TRIGGER_RIGHT) joystickState.rightTrigger = event.jaxis.value / 32767.0F;
// 
//                     printf("Joypad%d-> id=%d\n", event.jaxis.which, event.jaxis.axis);
//                     break;
//                 }
            }
        }

        Mat4 cameraOrientation = mat_mul(
                    mat_translation(appData.camera.position),
                    mat_rotation_euler(appData.camera.rotation)
                    );

        // Input()
        {
            Vec4 forwardRaw = mat_column(cameraOrientation, 2);
            Vec4 upRaw = mat_column(cameraOrientation, 1);
            Vec4 rightRaw = mat_column(cameraOrientation, 0);

            Vec3 forward = v3(forwardRaw.x, forwardRaw.y, forwardRaw.z);
            Vec3 up = v3(upRaw.x, upRaw.y, upRaw.z);
            Vec3 right = v3(rightRaw.x, rightRaw.y, rightRaw.z);

            constexpr float CAMERA_SPEED = 32; // 5;
            constexpr float CAMERA_SENSE = 100;
            constexpr float DEADZONE = 0.2F;
        
            if (vec_length(joystickState.leftStick) > DEADZONE) {
                appData.camera.position = vec_add(appData.camera.position, vec_mul(forward, v3(CAMERA_SPEED * deltaTime * joystickState.leftStick.y)));
                appData.camera.position = vec_add(appData.camera.position, vec_mul(right, v3(CAMERA_SPEED * deltaTime * joystickState.leftStick.x)));
            }

            if (vec_length(joystickState.rightStick) > DEADZONE) {
                appData.camera.rotation.y += CAMERA_SENSE * deltaTime * joystickState.rightStick.x;
                appData.camera.rotation.x += -CAMERA_SENSE * deltaTime * joystickState.rightStick.y;
            }
        }



        // Render()
        {
            s32 screenWidth, screenHeight;
            SDL_GetWindowSize(window, &screenWidth, &screenHeight);
            glViewport(0, 0, screenWidth, screenHeight);

            UniformPassData passData;
            {
                passData.inCameraOrientation = cameraOrientation;
                passData.inLastCameraOrientation = lastCameraOrientation;
                passData.inScreenSize = v2(screenWidth, screenHeight);
                passData.inTime = appData.time;
                UploadUniformData(ubo, passData);

                lastCameraOrientation = cameraOrientation;
            }

            // {
            //     void* data = glMapNamedBuffer(worldDataBuffer, GL_WRITE_ONLY);
            //     memcpy(data, worldData, GetWorldDataSize(worldData));
            //     glUnmapNamedBuffer(worldDataBuffer);

            //     for (s32 z = 0; z < worldData->zSize; z++) 
            //         for (s32 y = 0; y < worldData->ySize; y++)  
            //             for (s32 x = 0; x < worldData->xSize; x++)  {
            //                 Voxel voxel;

            //                 if (y > rand() / (float)RAND_MAX * 50 - 10) {
            //                     voxel.color = Vec4 { 0, 0, 0, 0 };
            //                 }
            //                 else {
            //                     voxel.color = Vec4 { rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1 };
            //                     voxel.color = vec_mul(v4(((float)y / 40)), voxel.color);
            //                 }
                            
            //                 worldDataVoxels[x + y * worldData->ySize + z * worldData->xSize * worldData->ySize] = voxel;
                            
            //                 int a = 123+ 123;
            //             }
            // }


            // glClearColor(1, 0, 64.0F/255.0F, 1);
            // glClear(GL_COLOR_BUFFER_BIT);

            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, worldDataBuffer);
            
            glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
            
            BindRenderTarget(fboGbuffer);
            glUseProgram(renderShader);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            UnbindRenderTarget();

            glUseProgram(spatialShader);

            
            {
                passData.inBlurDir = Vec2 { 1, 0 };
                UploadUniformData(ubo, passData);
                BindRenderTarget(fboSpatialIndirectInterm);
                BIND_TEX_UNIT(0, fboGbuffer, 2); // indirect
                BIND_TEX_UNIT(1, fboGbuffer, 3); // normals
                glDrawArrays(GL_TRIANGLES, 0, 6);
                UnbindRenderTarget();
            }

            {
                passData.inBlurDir = Vec2 { 0, 1 };
                UploadUniformData(ubo, passData);
                BindRenderTarget(fboSpatialIndirect);
                BIND_TEX_UNIT(0, fboSpatialIndirectInterm, 0); // indirect
                BIND_TEX_UNIT(1, fboGbuffer, 3); // normals
                glDrawArrays(GL_TRIANGLES, 0, 6);
                UnbindRenderTarget();
            }


            glUseProgram(temporalShader);
            BindRenderTarget(fboTemporalIndirect);
            BIND_TEX_UNIT(0, fboSpatialIndirect, 0); // indirect
            BIND_TEX_UNIT(1, fboLastIndirect, 0); // last indirect
            glDrawArrays(GL_TRIANGLES, 0, 6);
            UnbindRenderTarget();

            // copy indirect into last indirect buffer 
            {
                glUseProgram(blitShader);
                BindRenderTarget(fboLastIndirect);
                BIND_TEX_UNIT(0, fboTemporalIndirect, 0);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                UnbindRenderTarget();
            }

            glUseProgram(compositeShader);
            BIND_TEX_UNIT(0, fboGbuffer, 0); // color
            BIND_TEX_UNIT(1, fboGbuffer, 1); // direct
            BIND_TEX_UNIT(2, fboTemporalIndirect, 0); // indirect
            glDrawArrays(GL_TRIANGLES, 0, 6);



            SDL_GL_SwapWindow(window);
        }


        u32 endTime = SDL_GetTicks();
        deltaTime = (endTime - startTime) / 1000.0F;
        appData.time += deltaTime;
    }

    FreeRenderTarget(fboGbuffer);

    glDeleteProgram(renderShader);
    glDeleteBuffers(1, &quadVbo);
    glDeleteVertexArrays(1, &globalVao);

    return 0;
}
