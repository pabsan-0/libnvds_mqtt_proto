
all:
	sudo g++ libnvds_mqtt_proto.cpp -I/opt/nvidia/deepstream/deepstream/sources/includes \
								 	-I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/glib-2.0/include \
								 	-Ibuild-deps/paho.mqtt.c/src \
								 	 /opt/nvidia/deepstream/deepstream/lib/libnvds_logger.so \
								 	 /tools/build-deps/paho.mqtt.c/src/libpaho-mqtt3c.so \
								 	-std=c++11 -fPIC -shared -o ./libnvds_mqtt_proto.so
