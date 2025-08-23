# Compile Shaders
sokol-shdc -i terrain.glsl -l metal_macos:glsl430:hlsl5  -f sokol -o terrain_shader.h

# Build Application in Release mode
cd build
if [[ $1 == "release" ]];
then
    echo "Building Release Version..."
    cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
else
    echo "Building Development Version..."
fi
cmake --build . && ./sokol-dirt-jam
