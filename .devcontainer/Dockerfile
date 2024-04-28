FROM ubuntu:22.04

RUN apt-get update

RUN apt-get install -y \
    build-essential \
    pkgconf \
    openjdk-11-jdk \
    swig \
    python3-dev \
    python3-pip \
    libswt-gtk-4-java \
    gdb

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y tcl-dev

RUN pip install scons

ENV LD_LIBRARY_PATH=/workspaces/Soar/out:$LD_LIBRARY_PATH