#pragma once

#include "std.h"

struct VoxelModel {
    u32 sizeX, sizeY, sizeZ;
    u32* voxels;
};

void LoadVoxelModel(const char* file, VoxelModel* model);
void FreeVoxelModel(VoxelModel* model);