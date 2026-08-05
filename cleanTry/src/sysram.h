#pragma once

#include <stddef.h>
#include <stdint.h>

struct ddr_sr_result {
	uint32_t started;
	uint32_t phase;
	uint32_t finished;
	uint32_t zdata;
	uint32_t entry_stat;
	uint32_t exit_stat;
	uint32_t entry_ok;
	uint32_t exit_ok;
};

void sysram_init(void);
void sysram_run(void);
void sysram_configure_lpstop(void);

const struct ddr_sr_result *sysram_get_result(void);
uintptr_t sysram_get_stub_addr(void);
uintptr_t sysram_get_result_addr(void);
uintptr_t sysram_get_stack_addr(void);
size_t sysram_get_text_size(void);
