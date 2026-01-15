# 编译器设置
CXX = g++
# 基础编译选项 (对应你的 -std=c++11 -O3)
CXXFLAGS = -std=c++11 -O3 #-Wall

# 目标文件名默认值
TARGET = ReduceColor
SRC = ReduceColor.cpp

# --- 核心逻辑：检测命令行是否传入了 DEBUG=1 ---
ifdef DEBUG
    # 如果是调试模式：
    # 1. 追加 ASan 和 -g 选项
    CXXFLAGS += -fsanitize=address -g
    # 2. 修改输出文件名，避免覆盖正式版 (例如变成 ReduceColor_debug)
    TARGET := $(TARGET)_debug
else
    # 如果是正式模式 (默认)：
    # 这里可以额外加 -DNDEBUG 关闭 assert (可选)
    CXXFLAGS += -DNDEBUG
endif

# --- 编译规则 ---

# 默认目标
all: $(TARGET)

# 编译命令
$(TARGET): $(SRC) ReduceColor.h basic.h util.h
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# 清理命令
clean:
	rm -f ReduceColor ReduceColor_debug