rmdir /S /Q build32
mkdir build32

cd build32

cmake -G "Visual Studio 15 2017" -DCMAKE_SYSTEM_VERSION=10.0.15063.0 ..
cmake --build . --config Release
cd ..

"%VK_SDK_PATH%"Bin\glslangValidator.exe -V src\GameOverlay\vulkan\src\shader.vert -o python\game_overlay_sdk\lib\vert.spv
"%VK_SDK_PATH%"Bin\glslangValidator.exe -V src\GameOverlay\vulkan\src\shader.frag -o python\game_overlay_sdk\lib\frag.spv
"%VK_SDK_PATH%"Bin\glslangValidator.exe -V src\GameOverlay\vulkan\src\shader.comp -o python\game_overlay_sdk\lib\comp.spv

echo F | xcopy /Y src\GameOverlay\vulkan\src\shader.vert python\game_overlay_sdk\lib\shader.vert
echo F | xcopy /Y src\GameOverlay\vulkan\src\shader.frag python\game_overlay_sdk\lib\shader.frag
echo F | xcopy /Y src\GameOverlay\vulkan\src\shader.comp python\game_overlay_sdk\lib\shader.comp