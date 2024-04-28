FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG USERNAME="developer"
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN apt-get update && apt-get install -y \
    astyle \
    build-essential \
    cppcheck \
    gdb \
    git \
    libswt-gtk-4-java \
    mono-complete \
    openjdk-11-jdk \
    pkgconf \
    python3-dev \
    python3-pip \
    sudo \
    swig \
    tcl-dev

# TODO Visual Soar
# TODO C#: mono not found by scons

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME



USER ${USERNAME}

RUN pip install \
    scons

ENV LD_LIBRARY_PATH=/workspaces/Soar/out:$LD_LIBRARY_PATH
