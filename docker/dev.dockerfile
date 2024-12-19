FROM gcc:9

RUN apt update
RUN apt-get -y install cmake git

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

RUN git clone --branch 2.5.13 https://github.com/adafruit/Adafruit_SSD1306.git ssd1306
ENV SSD1306_PATH=${EXTERNAL_PATH}/ssd1306

# server sources should be mapped to this path during container run
WORKDIR /usr/src/app

CMD ["/bin/bash"]
