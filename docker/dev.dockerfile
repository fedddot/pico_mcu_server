FROM alpine:latest AS base_image

RUN apk update
RUN apk add git make cmake clang-extra-tools
RUN apk add gcc-arm-none-eabi g++-arm-none-eabi newlib-arm-none-eabi gdb-multiarch
RUN apk add python3 python3-dev py3-pip
RUN pip install --upgrade --break-system-packages protobuf grpcio-tools

ENV PICO_SDK_PATH=/usr/app/deps/pico-sdk
WORKDIR ${PICO_SDK_PATH}
RUN git clone --branch=2.2.0 https://github.com/raspberrypi/pico-sdk.git ${PICO_SDK_PATH}
RUN git submodule update --init

WORKDIR /usr/app/src

ENTRYPOINT ["sh"]