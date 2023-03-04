#include "qb.h"

#include <stdio.h>
#include <stdlib.h>

void LoadVoxelModel(const char* file, VoxelModel* model) {
	u32 version;
	u32 colorFormat;
	u32 zAxisOrientation;
	u32 compressed;
	u32 visibilityMaskEncoded;
	u32 numMatrices;
	// constexpr u32 CODEFLAG = 2;
	// constexpr u32 NEXTSLICEFLAG = 6;

    FILE* f = fopen(file, "rb");

	fread(&version, sizeof(version), 1, f);
	fread(&colorFormat, sizeof(colorFormat), 1, f);
	fread(&zAxisOrientation, sizeof(zAxisOrientation), 1, f);
	fread(&compressed, sizeof(compressed), 1, f);
	fread(&visibilityMaskEncoded, sizeof(visibilityMaskEncoded), 1, f);
	fread(&numMatrices, sizeof(numMatrices), 1, f);
 
	for (u32 i = 0; i < numMatrices; i++) // for each matrix stored in file
	{
		// read matrix name
		u8 nameLength;
        fread(&nameLength, sizeof(nameLength), 1, f);
        char name[nameLength];
        fread(name, nameLength, 1, f);

        s32 posX;
        s32 posY;
        s32 posZ;
        u32 sizeX;
        u32 sizeY;
        u32 sizeZ;

		// read matrix size 
		fread(&sizeX, sizeof(sizeX), 1, f);
		fread(&sizeY, sizeof(sizeY), 1, f);
		fread(&sizeZ, sizeof(sizeZ), 1, f);

		// read matrix position (in this example the position is irrelevant)
		fread(&posX, sizeof(posX), 1, f);
		fread(&posY, sizeof(posY), 1, f);
		fread(&posZ, sizeof(posZ), 1, f);

        model->sizeX = sizeX;
        model->sizeY = sizeY;
        model->sizeZ = sizeZ;
        model->voxels = (u32*)malloc(sizeof(u32) * sizeX * sizeY * sizeZ);
		if (compressed == 0) // if uncompressd
		{
			for (u32 z = 0; z < sizeZ; z++)
				for (u32 y = 0; y < sizeY; y++)
					for (u32 x = 0; x < sizeX; x++) {
						fread(&model->voxels[x + y*sizeX + z*sizeX*sizeY], sizeof(u32), 1, f);
                    }
		}
		else // if compressed
		{ 
			// z = 0;

			// while (z < sizeZ) 
			// {
			// 	z++;
			// 	index = 0;

			// 	while (true) 
			// 	{
			// 		data = fread_uint32(file);

			// 		if (data == NEXTSLICEFLAG)
			// 			break;
			// 		else if (data == CODEFLAG) 
			// 		{
			// 			count = fread_uint32(file);
			// 			data = fread_uint32(file);

			// 			for (j = 0; j < count; j++) 
			// 			{
			// 				x = index mod sizeX + 1; // mod = modulo e.g. 12 mod 8 = 4
			// 				y = index div sizeX + 1; // div = integer division e.g. 12 div 8 = 1
			// 				index++;
			// 				matrix[x + y*sizeX + z*sizeX*sizeY] = data;
			// 			}
			// 		}
			// 		else 
			// 		{
			// 			x = index mod sizex + 1;
			// 			y = index div sizex + 1;
			// 			index++;
			// 			matrix[x + y*sizeX + z*sizeX*sizeY] = data;
			// 		}
			// 	}
			// }
            printf("Crapskis!\n");
		}
	}
}