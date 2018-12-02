TARGET_ARCH = x86
ARCH = x86

ifeq ($(TARGET_ARCH), x86)
    ARCH = x86
else ifeq ($(TARGET_ARCH), x8664)
    ARCH = x8664
endif

SRC_DIR     = ../src
OBJ_DIR     = ../obj
ARCH_DIR    = ../arch/$(ARCH)
ARCH_OBJ_DIR    = ../arch_obj/$(ARCH)
INC_DIR     = ../inc

SRC_FILE    = $(shell find $(SRC_DIR) -name "*.c")
OBJ_FILE    = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILE))
ARCH_FILE   = $(shell find $(ARCH_DIR) -name "*.c")
ARCH_OBJ_FILE   = $(patsubst $(ARCH_DIR)/%.c, $(ARCH_OBJ_DIR)/%.o, $(ARCH_FILE))

CC          = gcc
DEF         =
FLAGS       = -g -m32 -fno-pie
LINK        = -lpthread

OUT_SO      = ../lib/libhk.so

all : $(OUT_SO)

$(OUT_SO): $(OBJ_FILE) $(ARCH_OBJ_FILE)
	$(CC) -o $(OUT_SO) $(OBJ_FILE) $(ARCH_OBJ_FILE) -shared -fPIC $(LINK) $(FLAGS) $(DEF)

$(OBJ_FILE): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -I$(INC_DIR) -c $< -o $@ $(FLAGS) $(DEF)

$(ARCH_OBJ_FILE): $(ARCH_OBJ_DIR)/%.o : $(ARCH_DIR)/%.c
	$(CC) -I$(INC_DIR) -c $< -o $@ $(FLAGS) $(DEF)

.PHONY: clean
clean c:
	@rm -rvf $(OBJ_DIR)/*.o
	@rm -rvf $(ARCH_OBJ_DIR)/*.o
	@rm -rvf $(OUT_SO)


