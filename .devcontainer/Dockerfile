FROM mcr.microsoft.com/devcontainers/base:jammy

USER root

RUN apt update && \
    apt-get install -y \
        git wget flex bison gperf \
        python3 python3-venv cmake ninja-build ccache \
        libffi-dev libssl-dev dfu-util libusb-1.0-0 \
        python3-pip && \
    rm -rf /var/cache/apt

USER vscode

RUN mkdir -p ~/esp
RUN git clone --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
RUN ~/esp/esp-idf/install.sh esp32

RUN python3 -m pip install mbed-cli

USER root
RUN mkdir /opt/mbed
RUN cd /opt/mbed; sudo $(which mbed-studio) --appimage-extract
RUN mv /opt/mbed/squashfs-root /opt/mbed
RUN chmod 755 /opt/mbed