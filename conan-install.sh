pip install conan
conan profile detect --force
conan install . --build=missing
conan install . --build=missing -s build_type=Debug