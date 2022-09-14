#!/bin/sh
#
# Installs all required dependencies
#

PROJECT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_DEPS_DIR="$PROJECT_DIR/build-deps/"

# Create temp directory for building/pulling external libs
if [ ! -d "$BUILD_DEPS_DIR" ]; then
sudo mkdir ${PROJECT_DIR}/build-deps/
fi

## Install dependencies required via apt-get
apt-get update
apt-get install -y  build-essential                 \
                    libssl-dev                      \
                    gcc                             \
                    make                            \
                    cmake                           \
                    cmake-gui                       \
                    cmake-curses-gui                \
                    automake                        \
                    autoconf                        \
                    libtool                         \
                    doxygen                         \
                    graphviz                        \
                    git                             \
                    gcc-arm-linux-gnueabihf         \
                    g++-arm-linux-gnueabihf         \
                    pkg-config-arm-linux-gnueabihf

# Pull PAHO.MQTT.C from github
cd ${PROJECT_DIR}/build-deps/
git clone https://github.com/eclipse/paho.mqtt.c

# Build PAHO.MQTT.C & installing..
cd ${PROJECT_DIR}/build-deps/paho.mqtt.c/
cmake -DPAHO_WITH_SSL=TRUE -DPAHO_BUILD_DOCUMENTATION=FALSE -DPAHO_BUILD_STATIC=TRUE -DPAHO_BUILD_SAMPLES=TRUE ${CROSS_COMPILE_ARG}
make
sudo make install

# Pull PAHO.MQTT.CPP from github
cd ${PROJECT_DIR}/build-deps/
git clone https://github.com/eclipse/paho.mqtt.cpp

# Building PAHO.MQTT.CPP & installing..
cd ${PROJECT_DIR}/build-deps/paho.mqtt.cpp/
cmake -DPAHO_WITH_SSL=TRUE -DPAHO_BUILD_DOCUMENTATION=FALSE -DPAHO_BUILD_STATIC=TRUE -DPAHO_BUILD_SHARED=FALSE \
        -DPAHO_MQTT_C_PATH=../paho.mqtt.c/ -DPAHO_MQTT_C_LIB=../paho.mqtt.c/src/libpaho.mqtt3as-static.a ${CROSS_COMPILE_ARG}
make
make install

echo "Cleaning up temporary files.."
# sudo rm -r ${PROJECT_DIR}/build-deps
echo "Cleaning up done"

# Regenerate cache and links to libs
sudo ldconfig
echo "Finished"