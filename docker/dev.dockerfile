FROM mcu-server-dev:latest AS builder

RUN apt update

# pico-sdk deps
RUN apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib gdb-multiarch

# install externals
ARG EXTERNAL_PATH=/usr/src/external
WORKDIR ${EXTERNAL_PATH} 

RUN git clone --branch=2.1.1 https://github.com/raspberrypi/pico-sdk.git pico-sdk
RUN cd pico-sdk && git submodule update --init
ENV PICO_SDK_PATH=${EXTERNAL_PATH}/pico-sdk

RUN git clone --branch=main https://github.com/fedddot/mcu_server.git mcu_server
ENV MCU_SERVER_PATH=${EXTERNAL_PATH}/mcu_server

RUN git clone --branch=nanopb-0.4.9.1 https://github.com/nanopb/nanopb.git nanopb
ENV NANOPB_PATH=${EXTERNAL_PATH}/nanopb

# server sources should be mapped to this path during container run
WORKDIR /usr/src/app

CMD ["/bin/bash"]
