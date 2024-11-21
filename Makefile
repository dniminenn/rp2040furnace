BUILD_DIR = build
PICO_BOARD = pico_w

all:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DPICO_BOARD=$(PICO_BOARD) .. && make -j$$(nproc)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

.PHONY: all clean rebuild
