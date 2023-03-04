# galileo3d

This is a 3D renderer project I worked on a little while ago. It does realtime voxel pathtracing on the GPU, with 1 bounce of global illumination.

The denoising filter is a cut-down version of Spatiotemporal Variance-Guided Filtering, without the variance history buffer (only spatio-temporal).

# usage

run by calling 

```bash
bin/galileo3d
```

on windows using prebuilt binaries: (built w/ clang)
```
winbin/galileo3d
```
