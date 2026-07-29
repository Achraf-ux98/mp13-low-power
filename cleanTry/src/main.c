#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "sysram.h"

#define SYSRAM_START 0x2FFE0000U
#define SYSRAM_END   0x30000000U

int main(void)
{
	const struct ddr_sr_result *result;
	uintptr_t stub_addr;
	uintptr_t result_addr;
	uintptr_t stack_addr;
	uint32_t t0_ms;
	uint32_t t1_ms;
	unsigned int irq_key;

	printk("\n=== SYSRAM copy+execute test (module method) ===\n");
	printk("sysram init              = begin\n");
	sysram_init();
	printk("sysram init              = done\n");

	stub_addr = sysram_get_stub_addr();
	result_addr = sysram_get_result_addr();
	stack_addr = sysram_get_stack_addr();

	printk("SYSRAM range              = 0x%08x - 0x%08x\n",
	       (uint32_t)SYSRAM_START,
	       (uint32_t)(SYSRAM_END - 1U));
	printk("sysram_text size          = %u bytes\n",
	       (unsigned int)sysram_get_text_size());
	printk("stub addr                 = 0x%08x\n", (uint32_t)stub_addr);
	printk("result addr               = 0x%08x\n", (uint32_t)result_addr);
	printk("sysram stack addr         = 0x%08x\n", (uint32_t)stack_addr);

	if ((stub_addr < SYSRAM_START) || (stub_addr >= SYSRAM_END)) {
		printk("ERROR: stub is not in SYSRAM\n");
	}
	if ((result_addr < SYSRAM_START) || (result_addr >= SYSRAM_END)) {
		printk("ERROR: result is not in SYSRAM\n");
	}
	if ((stack_addr < SYSRAM_START) || (stack_addr >= SYSRAM_END)) {
		printk("ERROR: stack is not in SYSRAM\n");
	}

	printk("\n=== Running SYSRAM DDR self-refresh with LED blinking (5s) ===\n");
	t0_ms = k_uptime_get_32();
	printk("timestamp before run      = %u ms\n", t0_ms);

	irq_key = irq_lock();
	sysram_run();
	irq_unlock(irq_key);

	t1_ms = k_uptime_get_32();
	printk("timestamp after run       = %u ms\n", t1_ms);
	printk("elapsed                   = %u ms\n", t1_ms - t0_ms);

	result = sysram_get_result();
	printk("result: started=%u phase=%u finished=%u entry_ok=%u exit_ok=%u\n",
	       result->started,
	       result->phase,
	       result->finished,
	       result->entry_ok,
	       result->exit_ok);
	printk("result: entry_stat=0x%08x exit_stat=0x%08x zdata=0x%08x\n",
	       result->entry_stat,
	       result->exit_stat,
	       result->zdata);

	printk("\n=== Test completed! Continuing normal execution... ===\n");

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
