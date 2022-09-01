# this Makefile is to be used to build the mqtt_proto protocol adaptor .so

DS_INC:=/opt/nvidia/deepstream/deepstream/sources/includes
DS_LIB:=/opt/nvidia/deepstream/deepstream/lib

SRCS:= onenet_msgconv.c
TARGET_LIB:= libnvds_onenet_msgconv.so

CFLAGS:= -fPIC -Wall

PKGS:= glib-2.0 json-glib-1.0

LDFLAGS:= -shared

INC_PATHS:= -I $(DS_INC)
CFLAGS+= $(INC_PATHS)

LIBPATHS = -L /usr/local/lib -L $(DS_LIB)
LIBS+= -lc

CFLAGS+= `pkg-config --cflags $(PKGS)`
LIBS:= `pkg-config --libs $(PKGS)`

all: $(TARGET_LIB)

$(TARGET_DEBUG_LIB) : $(SRCS)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LDFLAGS) $(INC_PATHS) $(LIBPATHS) $(LIBS)

$(TARGET_LIB) : $(SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(INC_PATHS) $(LIBPATHS) $(LIBS)


clean:
	rm -rf $(TARGET_LIB) $(TARGET_DEBUG_LIB)

