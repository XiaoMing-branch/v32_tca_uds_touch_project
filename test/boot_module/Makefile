CC = gcc

#颜色代码
GREEN = \033[0;32m
COLOR_RESET = \033[0m

CC_SYMBOLS =
#头文件位置
INCLUDES = -I../FFF \
           -Iexclusive_fff \
           -I../Unity/src \
           -I../../boot/examples/tcpl03x/demo_apps/bootloader_seres/include \
           -I../../boot/lib/lin/inc \
           -I../../boot/middleware/lin_manager/inc \
           -I../../boot/middleware/log_manager/inc \
           -I../../boot/middleware/os_manager/inc \
           -I../../boot/middleware/store_manager/inc \
           -I../../boot/middleware/task_manager/inc \
           -I../../boot/platform/CMSIS/Core/Include \
           -I../../boot/platform/devices/tcpl03x/inc \
           -I../../boot/platform/drivers/tcpl03x_ll_driver/inc \
           -I../../boot/platform/drivers/tcpl03x_ll_driver/inc/reg \
           -I../../boot/platform/pal/multiplexing/inc \
           -I../../boot/platform/pal/pal_gpio/inc \
           -I../../boot/platform/pal/pal_led/inc \
           -I../../boot/platform/pal/pal_lin/inc \
           -I../../boot/platform/pal/pal_log/inc \
           -I../../boot/platform/pal/pal_meas/inc \
           -I../../boot/platform/pal/pal_pmu/inc \
           -I../../boot/platform/pal/pal_store/inc \
           -I../../boot/platform/pal/pal_systick/inc \
           -I../../boot/platform/pal/pal_timer/inc \
           -I../../boot/platform/pal/pal_wdg/inc \
           -I../../boot/platform/pal/utilities/inc

#源文件位置
SRC += $(shell find ../../boot/examples/tcpl03x/demo_apps/bootloader_seres/source/dfu_uds_manager.c) \
       $(shell find ../../boot/examples/tcpl03x/demo_apps/bootloader_seres/source/aes_cmac.c) \
	   $(shell find ../../boot/platform/pal/utilities/utilities.c)\
       $(shell find ../Unity/src -name "*.c") \
       $(shell find exclusive_fff -name "*.c")

#将SRC下的.c转为.o并放入TEST_OBJDIR
VPATH = $(sort $(dir $(SRC)))
OBJS = $(addprefix $(TEST_OBJDIR)/, $(notdir $(SRC:.c=.o)))

# 1. 找出所有以 fff_ 开头的桩文件对象
MOCK_OBJS = $(filter $(TEST_OBJDIR)/fff_%.o, $(OBJS))
# 2. 找出这些桩文件对应的“真货”文件名（例如 fff_aes_cmac.o 对应 aes_cmac.o）
REALS_THAT_HAVE_MOCKS = $(patsubst fff_%,%,$(notdir $(MOCK_OBJS)))

DEFS = -DENABLE_TEST_MODE \
       -D__TCPL03X__ \
       -DTCAE10X \
       -DLIN_TX_QUEUE_SIZE=8

#CFLAGS = $(INCLUDES) $(DEFS) -c -Wall -Werror -g -ggdb -std=gnu99 -coverage
CFLAGS = $(INCLUDES) $(DEFS) -c -Wall -g -ggdb -std=gnu99 -coverage

LDFLAGS = -coverage -lm

LDLIBS = -lm -lpthread

#单元测试文件路径
TEST_DIR = tests
#编译文件存放路径
TEST_OBJDIR = build

TEST_SET_OPTS =
ifeq ($(OSTYPE),Cygwin)
    TEST_SET_OPTS = igncr
endif

TEST_SRC = $(wildcard $(TEST_DIR)/test_*.c)

TESTS=$(patsubst %.c,$(TEST_OBJDIR)/%.bin,$(TEST_SRC))

# 测试支持文件（仅当 common.c 存在时才生效）
TEST_SUPPORT_SRC = $(wildcard $(TEST_DIR)/common.c)  # 若文件不存在，变量为空
TEST_SUPPORT_OBJS = $(patsubst %,$(TEST_OBJDIR)/%,$(TEST_SUPPORT_SRC:.c=.o))  # 源文件不存在时，目标文件也为空

COVERAGE_INFO_FILENAME = coverage.info
COVERAGE_INFO_PATH = $(TEST_OBJDIR)/$(COVERAGE_INFO_FILENAME)

#编译规则
$(TEST_OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CC_SYMBOLS) $(INCLUDES) -o $@ $<
#链接规则
$(TEST_OBJDIR)/%.bin: $(TEST_OBJDIR)/%.o $(OBJS) $(TEST_SUPPORT_OBJS)
	@mkdir -p $(dir $@)
	$(eval CUR_TEST_NAME := $(notdir $(basename $@)))
	$(eval MODULE_UNDER_TEST := $(patsubst test_%,%,$(CUR_TEST_NAME)))
	$(eval EXCLUDE_MOCK := $(TEST_OBJDIR)/fff_$(MODULE_UNDER_TEST).o)
	$(eval ALL_OTHER_REALS := $(filter-out $(TEST_OBJDIR)/$(MODULE_UNDER_TEST).o, $(addprefix $(TEST_OBJDIR)/, $(REALS_THAT_HAVE_MOCKS))))
	@echo "$(GREEN)Linking $@...$(COLOR_RESET)"
	$(CC) $(LDFLAGS) $(CC_SYMBOLS) $(INCLUDES) -o $@ \
		$(filter-out $(EXCLUDE_MOCK) $(ALL_OTHER_REALS), $^) \
		$(LDLIBS)

all: $(OBJS)

test: $(TESTS)
	@echo "开始进行单元测试"
	@set -o $(TEST_SET_OPTS) >/dev/null 2>&1
	@export SHELLOPTS
	@sh ../runtests.sh $(TEST_OBJDIR)/$(TEST_DIR)
	@echo "单元测试完成"

clean:
	@echo "正在清理测试目录$(TEST_OBJDIR)"
	@rm -rf $(TEST_OBJDIR)
	@echo "测试目录清理完成"

coverage:
	@echo "正在进行覆盖测试并生成报告"
	@make clean
	@make test
	@lcov --rc lcov_branch_coverage=1 --base-directory . --directory $(TEST_OBJDIR) -c -o $(TEST_OBJDIR)/coverage.info
	@lcov --rc lcov_branch_coverage=1 --remove $(COVERAGE_INFO_PATH) "*/Unity/src/*" "*/tests/*" "*/exclusive_fff/*" -o $(COVERAGE_INFO_PATH)
	@genhtml --rc genhtml_branch_coverage=1 -o $(TEST_OBJDIR)/coverage -t "isotp-c test coverage" --num-spaces 4 $(COVERAGE_INFO_PATH)
	@echo "$(GREEN)Coverage information generated in $(TEST_OBJDIR)/coverage/index.html.$(COLOR_RESET)"
	@echo "执行完成"

help:
	@echo "==================== 支持的 Make 指令 ===================="
	@echo "  make                ：默认执行 all 目标，编译所有核心 .o 文件（项目基础编译）"
	@echo "  make test           ：执行单元测试（运行所有 .bin 测试用例，不生成报告）"
	@echo "  make coverage       ：执行覆盖测试并生成 HTML 报告"
	@echo "  make clean          ：清理测试目录（删除 build 目录下所有编译产物和报告）"
	@echo "  make help           ：显示当前帮助信息"
	@echo "=========================================================="
