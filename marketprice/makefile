TARGET  := marketprice
INCS = -I ../network -I ../utils

LIBS = -L ../utils -L ../network -L ../depends/hiredis \
       -Wl,-Bstatic \
       -lutils -lnetwork -lev -ljansson -lmpdec \
       -lrdkafka -lz -llz4 -lssl -lcrypto \
       -lhiredis -lzstd -llz4 \
       -Wl,-Bdynamic \
       -lcurl -lsasl2 \
       -lstdc++ -lm -lpthread -ldl

# 创建 obj 目录
OBJ_DIR = obj

# 获取所有 .c 源文件
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

# 添加头文件依赖
DEPS = $(OBJS:.o=.d)
-include $(DEPS)

# 默认目标
.PHONY: all
all: $(TARGET)

# 创建 obj 目录
$(shell mkdir -p $(OBJ_DIR))

# 编译规则
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -MMD -MP -c $< -o $@

# 链接规则
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

# 清理规则
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# 包含通用 makefile
include ../makefile.inc
