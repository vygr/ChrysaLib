SRC_DIR := ./src
OBJ_DIR := $(SRC_DIR)/obj

SRC_DIRS := $(shell find $(SRC_DIR) -type d | grep -v "/obj")
OBJ_DIRS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SRC_DIRS))

NODE_FILES := $(shell find $(SRC_DIR)/nodes -name "*.cpp")
NODE_FILES += $(shell find $(SRC_DIR)/nodes -name "*.c")

LIB_FILES := $(shell find $(SRC_DIR)/lib -name "*.cpp")
LIB_FILES += $(shell find $(SRC_DIR)/lib -name "*.c")
LIB_FILES += $(shell find $(SRC_DIR)/host -name "*.cpp")
LIB_FILES += $(shell find $(SRC_DIR)/host -name "*.c")

NODE_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(NODE_FILES))
NODE_OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(NODE_OBJ_FILES))

LIB_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_FILES))
LIB_OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_OBJ_FILES))

OBJ_FILES := $(NODE_OBJ_FILES)
OBJ_FILES += $(LIB_OBJ_FILES)

OS := $(shell uname)
CPPFLAGS := -std=c++17
CFLAGS += -MMD -D ASIO_STANDALONE
HGUI := $(shell echo $(GUI) | tr '[:upper:]' '[:lower:]')

HOST_GUI := 0
ifeq ($(HGUI),sdl)
	HOST_GUI := 0
endif
ifeq ($(HGUI),fb)
	HOST_GUI := 1
endif
ifeq ($(HGUI),raw)
	HOST_GUI := 2
endif

all:	CFLAGS += -O2
all:	hostenv hub_node gui_node files_node
debug:	CFLAGS += -DDEBUG -g -O2
debug:	hostenv hub_node gui_node files_node
debugo2:	CFLAGS += -g -O2
debugo2:	hostenv hub_node gui_node files_node

hostenv:
ifeq ($(HOST_GUI),0)
	@echo Building sdl GUI driver.
endif
ifeq ($(HOST_GUI),1)
	@echo Building fb GUI driver.
endif
ifeq ($(HOST_GUI),2)
	@echo Building raw GUI driver.
endif
	@mkdir -p $(OBJ_DIRS)

hub_node:	$(LIB_OBJ_FILES) $(OBJ_DIR)/nodes/hub/hub.o
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		-framework CoreFoundation \
		-framework IOKit \
		-framework Security \
		$(shell sdl2-config --libs) \
		$(shell brew --prefix libusb)/lib/libusb-1.0.a
else
	c++ -o $@ $^ \
		-pthread \
		$(shell sdl2-config --libs) \
		-L/usr/local/lib -lusb-1.0
endif

gui_node:	$(LIB_OBJ_FILES) $(OBJ_DIR)/nodes/gui/gui.o
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		-framework CoreFoundation \
		-framework IOKit \
		-framework Security \
		$(shell sdl2-config --libs) \
		$(shell brew --prefix libusb)/lib/libusb-1.0.a
else
	c++ -o $@ $^ \
		-pthread \
		$(shell sdl2-config --libs) \
		-L/usr/local/lib -lusb-1.0
endif

files_node:	$(LIB_OBJ_FILES) $(OBJ_DIR)/nodes/files/files.o
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		-framework CoreFoundation \
		-framework IOKit \
		-framework Security \
		$(shell sdl2-config --libs) \
		$(shell brew --prefix libusb)/lib/libusb-1.0.a
else
	c++ -o $@ $^ \
		-pthread \
		$(shell sdl2-config --libs) \
		-L/usr/local/lib -lusb-1.0
endif

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
ifeq ($(OS),Darwin)
ifeq ($(GUI),fb)
	c++ -c -o $@ $< $(CFLAGS) $(CPPFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I$(shell brew --prefix)/include/libusb-1.0/
else
	c++ -c -o $@ $< $(CFLAGS) $(CPPFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I$(shell brew --prefix)/include/libusb-1.0/ \
		$(shell sdl2-config --cflags)
endif
else
ifeq ($(GUI),fb)
	c++ -c -o $@ $< $(CFLAGS) $(CPPFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I/usr/local/include/libusb-1.0/
else
	c++ -c -o $@ $< $(CFLAGS) $(CPPFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I/usr/local/include/libusb-1.0/ \
		$(shell sdl2-config --cflags)
endif
endif

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
ifeq ($(OS),Darwin)
ifeq ($(GUI),fb)
	cc -c -o $@ $< $(CFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I$(shell brew --prefix)/include/libusb-1.0/
else
	cc -c -o $@ $< $(CFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I$(shell brew --prefix)/include/libusb-1.0/ \
		$(shell sdl2-config --cflags)
endif
else
ifeq ($(GUI),fb)
	cc -c -o $@ $< $(CFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I/usr/local/include/libusb-1.0/
else
	cc -c -o $@ $< $(CFLAGS) -D_HOST_GUI=$(HOST_GUI) \
		-I/usr/local/include/libusb-1.0/ \
		$(shell sdl2-config --cflags)
endif
endif

clean:
	@rm -f hub_node gui_node files_node
	@rm -rf $(OBJ_DIR)/

-include $(OBJ_FILES:.o=.d)
