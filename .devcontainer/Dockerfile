FROM mcr.microsoft.com/devcontainers/base:noble

RUN apt-get update && apt-get install -y \
    build-essential \
    clang-tidy \
    cmake \
    cppcheck \
    gdb \
    libswt-gtk-4-java \
    mono-complete \
    ninja-build \
    openjdk-11-jdk \
    python3-pip \
    swig \
    tcl-dev

RUN apt-get install -y qt6-base-dev qt6-base-dev-tools qt6-wayland-dev libqt6core6 libqt6gui6 libqt6widgets6 libqt6waylandclient6

RUN pip install --break-system-packages \
    scons \
    conan

ENV PATH="/workspaces/Soar/install/bin:${PATH}"
ENV LD_LIBRARY_PATH="/workspaces/Soar/install/lib:${LD_LIBRARY_PATH}"
ENV CPATH="/workspaces/Soar/install/include:${CPATH}"