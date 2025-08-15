# Compile Shaders
sokol-shdc -i terrain.glsl -l metal_macos -f sokol -o terrain_shader.h

# Build Application
cd build
cmake --build . && ./sokol-dirt-jam