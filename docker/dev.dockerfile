FROM gcc:9

RUN apt update
RUN apt-get -y install cmake git

# pico-sdk deps
RUN apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib

# install externals
ARG EXTERNAL_PATH=/usr/src/external
WORKDIR ${EXTERNAL_PATH} 

RUN git clone https://github.com/raspberrypi/pico-sdk.git pico-sdk
ENV PICO_SDK_PATH=${EXTERNAL_PATH}/pico-sdk

RUN git clone https://github.com/fedddot/mcu_server.git mcu_server
ENV MCU_SERVER_PATH=${EXTERNAL_PATH}/mcu_server

# server sources should be mapped to this path during container run
WORKDIR /usr/src/app

CMD ["/bin/bash"]
