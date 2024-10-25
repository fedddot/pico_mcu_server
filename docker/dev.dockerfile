FROM gcc:9

RUN apt update
RUN apt-get -y install cmake git curl

# install externals
WORKDIR /usr/src/external

ARG PICO_SDK_VERSION=2.0.0
RUN curl https://github.com/raspberrypi/pico-sdk/archive/refs/tags/${PICO_SDK_VERSION}.tar.gz


WORKDIR /usr/src/app

CMD ["/bin/bash"]
