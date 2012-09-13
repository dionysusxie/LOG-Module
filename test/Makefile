TARGET = testApp

OBJ_FILES = test.o

CXXFLAGS = -Wall -g -c

LIB_DIR = /usr/local/lib

LDFLAGS = -L$(LIB_DIR)
LDFLAGS += -lboost_thread -lboost_filesystem -lboost_system -lallyes-log

STATIC_ARCHIVES = 

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
	