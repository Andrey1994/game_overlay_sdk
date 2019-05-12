rmdir /S /Q build
mkdir build

cd build

cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_SYSTEM_VERSION=10.0.15063.0 ..
cmake --build . --config Release
cd ..
