FROM mcr.microsoft.com/devcontainers/base:jammy

USER root

RUN apt-get update && \
    apt-get install -y \
    git wget flex bison gperf \
    python3 python3-venv cmake ninja-build ccache \
    libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    python3-pip mercurial&& \
    rm -rf /var/cache/apt/*

USER vscode

RUN mkdir -p ~/esp
RUN git clone --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
RUN ~/esp/esp-idf/install.sh esp32 --enable-gdbgui
RUN python3 ~/esp/esp-idf/tools/idf_tools.py install qemu-xtensa

RUN python3 -m pip install mbed-cli

USER root
WORKDIR /tmp

RUN wget "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2?rev=78196d3461ba4c9089a67b5f33edf82a&hash=D484B37FF37D6FC3597EBE2877FB666A41D5253B" -O gcc-arm-none-eabi.tar.bz2
RUN tar -xvf gcc-arm-none-eabi.tar.bz2
RUN sudo mv gcc-arm-none-eabi-10.3-2021.10 /opt/gcc-arm-none-eabi-10.3-2021.10
RUN sudo rm -rf /tmp/*

# RUN mbed config -G GCC_ARM_PATH /opt/gcc-arm-none-eabi-10.3-2021.10/bin
# RUN mbed toolchain -G GCC_ARM

WORKDIR /workspaces/esp-stm32