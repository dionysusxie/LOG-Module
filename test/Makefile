TARGET = testApp

LOG_MODEL_DIR = ../output

OBJ_FILES = test.o

CXXFLAGS = -Wall -g -c
CXXFLAGS += -I$(LOG_MODEL_DIR)

LIB_DIR = /usr/local/lib

LDFLAGS = -L$(LIB_DIR)
LDFLAGS += -lboost_thread -lboost_filesystem -lboost_system

STATIC_ARCHIVES = $(LOG_MODEL_DIR)/liballyes-log.a

CC = g++

.PHONY: clean

$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(STATIC_ARCHIVES) $(LDFLAGS) -o $(TARGET)
	@echo "Test build successfully!"

%.o : %.cpp
	$(CC) $(CXXFLAGS) $*.cpp -o $*.o
	$(CC) $(CXXFLAGS) -MM $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

-include $(OBJECT_FILES:.o=.d)

clean:
	rm -f *.o *.d $(TARGET)
	