static void outb(unsigned char value, unsigned short port) {
  asm volatile("outb %b0, %w1" : : "a"(value), "d"(port));
}

static void io_wait(void) { outb(0x80, 0); }

// 8259a = 一主一丛各自8个 IR, 4个ICW，三个OCW

// 8259a 分主和从 各自有 7 个 IRQ(IRQ0~IRQ7)
// 从芯片的 IRQ1 级连到主芯片的 IRQ2
// 8259A寄存器 = ICW*4 + OCW*3
// ICW用于初始化 8259A
#define _8259A_M_CMD 0x20  // 主片控制端口
#define _8259A_M_DATA 0x21 // 主片数据端口
#define _8259A_S_CMD 0xA0  // 从片控制端口
#define _8259A_S_DATA 0xA1 // 从片数据端口
#define _8259A_M_IRQ_OFF 0x20
#define _8259A_S_IRQ_OFF 0x28
#define _8259A_VEC_CLOCK (_8259A_M_IRQ_OFF + 0) // 时钟中断 的中断向量号
#define _8259A_VEC_SATA0 (_8259A_S_IRQ_OFF + 6) // sata0 的中断向量号
#define _8259A_VEC_SATA1 (_8259A_S_IRQ_OFF + 7) // sata1 的中断向量号

static void _8259A_EOI() {
  outb(0x20, _8259A_S_CMD);
  outb(0x20, _8259A_M_CMD);
}

void _8259A_reset(char ocw_m, char ocw_s) {
  // 初始化 ICW1~ICW4 必须按照 ICW1~ICW4 的顺序进行
  // 主片:
  // ICW1 OCW2 OCW3 映射到 0x20 端口
  // ICW2~4 OCW1 映射到 0x21 端口
  // 从片:
  // ICW1 OCW2 OCW3 映射到 0xa0 端口
  // ICW2~4 OCW1 映射到 0xa1 端口

  // ICW1:
  // 5~7 位 pc 机必须位 0
  // 4 ICW 必须为 1
  // 3 触发模式忽略
  // 2 忽略
  // 1 true=单片 false=级联
  // 0 true=使用 ICW4 false= 不使用 ICW4
  outb(0b00010001, _8259A_M_CMD);
  io_wait();
  outb(0b00010001, _8259A_S_CMD);
  io_wait();

  // ICW2:
  // 我们设置主片的中断向量从 0x20 开始, 从片从 0x28 开始
  // 主0x20~0x27 从 0x28~0x2f
  outb(_8259A_M_IRQ_OFF, _8259A_M_DATA);
  io_wait();
  outb(_8259A_S_IRQ_OFF, _8259A_S_DATA);
  io_wait();

  // ICW3:
  // 主片 IRQ2 级联到从片
  outb(1 << 2, _8259A_M_DATA);
  io_wait();
  // 从片 IRQ1 级联到从片
  outb(1 << 1, _8259A_S_DATA);
  io_wait();

  // ICW4:
  // 这里通常不会设置的过于复杂
  // 主片 从片 开启 8086 模式
  // 这样后面中断处理中收到 0x20~0x2f 的中断后要向中断处理器发送 EOI 指令
  // 中断请求优先级按 IRQ0~IRQ7 从高到低排序
  outb(1, _8259A_M_DATA);
  io_wait();
  outb(1, _8259A_M_DATA);
  io_wait();

  // 暂时关闭所有中断
  outb(ocw_s, _8259A_S_DATA);
  io_wait();
  outb(ocw_m, _8259A_M_DATA);
  io_wait();
}
