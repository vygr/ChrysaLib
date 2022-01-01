SRC_DIR := ./src
OBJ_DIR := ./obj
NODE_DIR := $(SRC_DIR)/nodes
NODE_OBJ_DIR := $(OBJ_DIR)/nodes
LIB_DIR := $(SRC_DIR)/lib
LIB_OBJ_DIR := $(OBJ_DIR)/lib

NODE_FILES := $(shell find $(NODE_DIR) -name "*.cpp")
NODE_DIRS := $(shell find $(NODE_DIR) -type d)
LIB_FILES := $(shell find $(LIB_DIR) -name "*.cpp")
LIB_DIRS := $(shell find $(LIB_DIR) -type d)

NODE_OBJ_FILES := $(patsubst $(NODE_DIR)/%.cpp,$(NODE_OBJ_DIR)/%.o,$(NODE_FILES))
NODE_OBJ_DIRS := $(patsubst $(NODE_DIR)/%,$(NODE_OBJ_DIR)/%,$(NODE_DIRS))
LIB_OBJ_FILES := $(patsubst $(LIB_DIR)/%.cpp,$(LIB_OBJ_DIR)/%.o,$(LIB_FILES))
LIB_OBJ_DIRS := $(patsubst $(LIB_DIR)/%,$(LIB_OBJ_DIR)/%,$(LIB_DIRS))

OBJ_DIRS_CREATE := $(shell mkdir -p $(NODE_OBJ_DIRS) $(LIB_OBJ_DIRS))
OS := $(shell uname)

CPPFLAGS := -std=c++14 -D ASIO_STANDALONE
CXXFLAGS += -MMD

all:	CXXFLAGS += -O3
all:	hub_node files_node gui_node
debug:	CXXFLAGS += -DDEBUG -g -O0
debug:	hub_node files_node gui_node

hub_node:	$(LIB_OBJ_FILES) $(NODE_OBJ_DIR)/hub/hub.o
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		-framework CoreFoundation \
		-framework IOKit \
		$(shell sdl2-config --libs) \
		/usr/local/Cellar/libusb/1.0.24/lib/libusb-1.0.a
endif
ifeq ($(OS),Linux)
	c++ -o $@ $^ \
		-pthread \
		$(shell sdl2-config --libs) \
		-L/usr/local/lib -lusb-1.0
endif

gui_node:	$(LIB_OBJ_FILES) $(NODE_OBJ_DIR)/gui/gui.o
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		-framework CoreFoundation \
		-framework IOKit \
		$(shell sdl2-config --libs) \
		/usr/local/Cellar/libusb/1.0.24/lib/libusb-1.0.a
endif
ifeq ($(OS),Linux)
	c++ -o $@ $^ \
		-pthread \
		$(shell sdl2-config --libs) \
		-L/usr/local/lib -lusb-1.0
endif

files_node:	$(LIB_OBJ_FILES) $(NODE_OBJ_DIR)/files/files.o
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		-framework CoreFoundation \
		-framework IOKit \
		$(shell sdl2-config --libs) \
		/usr/local/Cellar/libusb/1.0.24/lib/libusb-1.0.a
endif
ifeq ($(OS),Linux)
	c++ -o $@ $^ \
		-pthread \
		$(shell sdl2-config --libs) \
		-L/usr/local/lib -lusb-1.0
endif

$(LIB_OBJ_DIR)/%.o : $(LIB_DIR)/%.cpp
ifeq ($(OS),Darwin)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c \
		-I/usr/local/include/libusb-1.0/ \
		$(shell sdl2-config --cflags) \
		-o $@ $<
endif
ifeq ($(OS),Linux)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c \
		-I/usr/local/include/libusb-1.0/ \
		$(shell sdl2-config --cflags) \
		-o $@ $<
endif

$(NODE_OBJ_DIR)/%.o : $(NODE_DIR)/%.cpp
ifeq ($(OS),Darwin)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c \
		-I/usr/local/include/libusb-1.0/ \
		$(shell sdl2-config --cflags) \
		-o $@ $<
endif
ifeq ($(OS),Linux)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c \
		-I/usr/local/include/libusb-1.0/ \
		$(shell sdl2-config --cflags) \
		-o $@ $<
endif

clean:
	rm -f hub_node files_node gui_node $(shell find . -name "*.o") $(shell find . -name "*.d")
	rm -rf $(OBJ_DIR)/

-include $(LIB_OBJ_FILES:.o=.d)
-include $(NODE_OBJ_FILES:.o=.d)
