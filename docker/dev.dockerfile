FROM mcu-server-dev:latest AS builder

RUN apt update

# pico-sdk deps
RUN apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib gdb-multiarch

# install externals
ARG EXTERNAL_PATH=/usr/src/external
WORKDIR ${EXTERNAL_PATH} 

RUN git clone https://github.com/fedddot/mcu_server.git mcu_server
ENV MCU_SERVER_PATH=${EXTERNAL_PATH}/mcu_server

RUN git clone https://github.com/raspberrypi/pico-sdk.git pico-sdk
RUN cd pico-sdk && git submodule update --init
ENV PICO_SDK_PATH=${EXTERNAL_PATH}/pico-sdk

# server sources should be mapped to this path during container run
WORKDIR /usr/src/app

CMD ["/bin/bash"]
