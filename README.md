# libnvds_mqtt_proto

A protocol library to get NVIDIA Deepstream detections output through MQTT.

This repo holds a few pieces of code taken from the web plus a tested NVDS pipeline configuration, saving precious time of lots of trial and error.

## Installation

- Install MQTT broker `sudo apt install mosquitto`
- Install `paho.mqtt.c` & `paho.mqtt.cpp` with dependencies:
    - Run `mkdir build-deps; bash mqtt_install.sh` for an auto-install
- Compile `libnvds_mqtt_proto.cpp` by running `make`.
    - This generates `libnvds_mqtt_proto.so`, and we're good to go.


## Usage

- Configure your pipeline's `nvmsgbroker`:
    - Link the protocol library `proto-lib="libnvds_mqtt_proto.so"`
    - Configure your connection string to your liking `conn-str="127.0.0.1;9998[;NAME]"`
    - Configure the publishing topic: `topic="hello"`
- Link the sample config file to `nvmsgconv`: `config="msgconv_config.txt"`
- Run Mosquitto to host the MQTT server
- Run your deepstream pipeline

Sample below:

```
gst-launch-1.0                                                                               \
    v4l2src device=/dev/video0                                                               \
        ! videoconvert ! 'video/x-raw,format=I420'                                           \
        ! queue leaky=2 max-size-buffers=100 max-size-time=0 max-size-bytes=1                \
        ! nvvideoconvert                                                                     \
        ! 'video/x-raw(memory:NVMM),format=(string)I420,width=640,height=480,framerate=30/1' \
        ! m.sink_0                                                                           \
    nvstreammux name=m batch-size=1 width=640 height=480 nvbuf-memory-type=0                 \
    ! nvvideoconvert                                                                         \
    ! nvinfer config-file-path="config_infer_primary.txt"                                    \
    ! tee name=teee                                                                          \
    teee.                                                                                    \
        ! nvmultistreamtiler width=640 height=480                                            \
        ! nvvideoconvert                                                                     \
        ! nvdsosd                                                                            \
        ! nveglglessink async=0 sync=0                                                       \
    teee.                                                                                    \
        ! queue leaky=1                                                                      \
        ! nvmsgconv msg2p-newapi=1                                                           \
            config="msgconv_config.txt"                                                      \
            payload-type=1                                                                   \
            frame-interval=1                                                                 \
        ! nvmsgbroker conn-str="127.0.0.1;9998"                                              \
            proto-lib="libnvds_mqtt_proto.so"                                                \
            topic="hello"                                                                    ;
```

## Acknowledgements

- Paho install script taken [from here](https://github.com/eclipse/paho.mqtt.cpp/issues/136)
- Library main script taken [from here](`https://forums.developer.nvidia.com/t/customize-gst-nvmsgbroker-adaptor-for-mqtt-using-paho-mqtt-c/163694/3`)