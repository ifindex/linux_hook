OUT_BIN        = test_proj

all : $(OUT_BIN)

$(OUT_BIN): test.c 
	gcc -o $(OUT_BIN) test.c -lpthread -m32 -ldl -lhk

.PHONY: clean
clean c:
	@rm -rvf $(OUT_BIN)

