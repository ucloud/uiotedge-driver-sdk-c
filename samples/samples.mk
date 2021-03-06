include ../make.settings
CFLAGS  = -g -Wall -O2 -lpthread

INCLUDE = -I../build/cjson -I../build/nats -I../build/utils -I../build/edge
LIBS    = ../build/edge/libedge.a ../build/nats/libnats_static.a ../build/utils/libutils.a ../build/cjson/libcjson.a

src    = uiot_edge_test.c
target = uiot_edge_test

all : $(target)

$(target) :
	$(CC) -o $(target) $(src) $(LIBS) $(INCLUDE) $(CFLAGS) -lrt

clean:
	-$(RM) $(target) $(target).o
