FROM ubuntu:22.04

RUN apt-get update

RUN apt-get install -y \
    build-essential \
    cmake \
    gdb \
    libswt-gtk-4-java \
    openjdk-11-jdk \
    pkgconf \
    python3-dev \
    python3-pip \
    swig

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y tcl-dev

RUN pip install \
    cpplint \
    scons

ENV LD_LIBRARY_PATH=/usr/local/lib/:/workspaces/Soar/out:$LD_LIBRARY_PATH