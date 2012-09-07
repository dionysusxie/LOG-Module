
OUT_DIR = output

LIB_INSTALL_DIR = /usr/local/lib
HEAD_INSTALL_DIR = /usr/local/include

LIB_SO_NAME = liballyes-log.so
LIB_A_NAME  = liballyes-log.a

LIB_SO_PATH = $(OUT_DIR)/$(LIB_SO_NAME)
LIB_A_PATH  = $(OUT_DIR)/$(LIB_A_NAME)

# the head file to be included by other APPs
EXTERNAL_INCLUDED_HEAD_FILE = allyes-log.h

CPP_FILES = log.cpp log_config.cpp LogSys.cpp Logger.cpp

CXXFLAGS = -Wall -g

LIB_DIR = /usr/local/lib

LDFLAGS = -L$(LIB_DIR) 
LDFLAGS += -lboost_filesystem

CC = g++

.PHONY: all clean install uninstall

all: $(LIB_SO_PATH) $(LIB_A_PATH)
	cp $(EXTERNAL_INCLUDED_HEAD_FILE) $(OUT_DIR)/
	@echo "Build successfully!"

$(LIB_SO_PATH): $(CPP_FILES)
	@echo "Build .so file ..."
	$(CC) $(CXXFLAGS) -fPIC $(CPP_FILES) $(LDFLAGS) -shared -o $(LIB_SO_PATH)
	
$(LIB_A_PATH): $(CPP_FILES)
	@echo "Build .a file ..."
	$(CC) $(CXXFLAGS) -c $(CPP_FILES)
	ar crv $(LIB_A_PATH) *.o

clean:
	rm -f $(OUT_DIR)/*.h $(OUT_DIR)/*.so $(OUT_DIR)/*.a
	rm -f *.o

install:
	cp $(OUT_DIR)/$(LIB_SO_NAME) $(OUT_DIR)/$(LIB_A_NAME) $(LIB_INSTALL_DIR)
	cp $(OUT_DIR)/$(EXTERNAL_INCLUDED_HEAD_FILE) $(HEAD_INSTALL_DIR)
	
uninstall:
	rm $(LIB_INSTALL_DIR)/$(LIB_SO_NAME) $(LIB_INSTALL_DIR)/$(LIB_A_NAME)
	rm $(HEAD_INSTALL_DIR)/$(EXTERNAL_INCLUDED_HEAD_FILE)
	