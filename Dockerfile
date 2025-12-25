FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    nasm \
    gcc \
    gcc-multilib \
    g++ \
    g++-multilib \
    make \
    xorriso \
    mtools \
    grub-pc-bin \
    grub-common \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /os

CMD ["/bin/bash"]
