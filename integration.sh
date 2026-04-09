sudo apt-get install -y libsqlite3-dev libspdlog-dev
cmake --preset Release-shared
cmake --build --preset Release-shared
cmake --install build/Release --prefix /tmp/soar-staging
cmake -S tests/Integration \
                -B /tmp/soar_integration_build \
                -Dsoar_DIR=/tmp/soar-staging/lib/cmake/soar \
                -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/soar_integration_build
ctest --test-dir /tmp/soar_integration_build --output-on-failure
