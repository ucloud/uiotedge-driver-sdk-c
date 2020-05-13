# 解压nats组件源码并完成编译和安装

LIB_NATS_TAR    = $(PWD)/deps/nats.c-master
LIB_CJSON_TAR   = $(PWD)/deps/cJSON-1.7.7
LIB_NATS_DIR    = $(PWD)/build/nats
LIB_CJSON_DIR   = $(PWD)/build/cjson

all :
	unzip $(LIB_NATS_TAR).zip
	mkdir -p ../build/nats
	cd $(LIB_NATS_TAR) && cmake . -DNATS_BUILD_WITH_TLS=OFF -DNATS_BUILD_STREAMING=OFF -DNATS_BUILD_NO_SPIN=ON && make
	cp $(PWD)/deps/nats.c-master/src/libnats_static.a $(LIB_NATS_DIR)
	cp $(PWD)/deps/nats.c-master/src/nats.h $(LIB_NATS_DIR)
	cp $(PWD)/deps/nats.c-master/src/status.h $(LIB_NATS_DIR)
	cp $(PWD)/deps/nats.c-master/src/version.h $(LIB_NATS_DIR)
	mkdir -p ../build/cjson
	tar xvfz $(LIB_CJSON_TAR).tar.gz
	cd $(LIB_CJSON_TAR) && make clean && make static
	cp $(LIB_CJSON_TAR)/libcjson.a $(LIB_CJSON_DIR)
	cp $(LIB_CJSON_TAR)/cJSON.h $(LIB_CJSON_DIR)

clean :	
	cd $(LIB_NATS_TAR) && make clean
	cd $(LIB_CJSON_TAR) && make clean
