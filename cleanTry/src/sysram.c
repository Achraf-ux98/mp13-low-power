#include <stdint.h>
#include <string.h>

#include "sysram.h"

#define SYSRAM_START 0x2FFE0000U
#define SYSRAM_END   0x30000000U

#define DDRCTRL_BASE_ADDR  0x5A003000U
#define DDRPHYC_BASE_ADDR  0x5A004000U
#define PWR_BASE_ADDR      0x50001000U

#define DDRCTRL_STAT_OFFSET    0x004U
#define DDRCTRL_PWRCTL_OFFSET  0x030U
#define DDRPHYC_ZQ0CR0_OFFSET  0x180U
#define PWR_CR3_OFFSET         0x00CU

#define DDRCTRL_STAT_OPERATING_MODE_NML  0x00000001U
#define DDRCTRL_STAT_OPERATING_MODE_SR   0x00000003U
#define DDRCTRL_STAT_SELFREF_TYPE_SW     0x00000020U

#ifndef DDRCTRL_STAT_OPERATING_MODE_Msk
#define DDRCTRL_STAT_OPERATING_MODE_Msk  0x00000007U
#endif

#ifndef DDRCTRL_STAT_SELFREF_TYPE_Msk
#define DDRCTRL_STAT_SELFREF_TYPE_Msk    0x00000030U
#endif

#ifndef DDRCTRL_PWRCTL_SELFREF_SW
#define DDRCTRL_PWRCTL_SELFREF_SW        0x00000020U
#endif

#ifndef DDRCTRL_PWRCTL_SELFREF_EN
#define DDRCTRL_PWRCTL_SELFREF_EN        0x00000001U
#endif

#ifndef DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE
#define DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE 0x00000008U
#endif

#ifndef DDRPHYC_ZQ0CR0_ZDATA_Msk
#define DDRPHYC_ZQ0CR0_ZDATA_Msk         0x000FFFFFU
#endif

#ifndef PWR_CR3_DDRSREN
#define PWR_CR3_DDRSREN                  0x00010000U
#endif

#ifndef PWR_CR3_DDRRETEN
#define PWR_CR3_DDRRETEN                 0x00020000U
#endif

/* GPIO A LED control (LD3 is active-low on pin 14). */
#define GPIOA_BASE_ADDR   0x50002000U
#define GPIO_MODER_OFFSET 0x00U
#define GPIO_OTYPER_OFFSET 0x04U
#define GPIO_PUPDR_OFFSET 0x0CU
#define GPIO_BSRR_OFFSET  0x18U
#define LED_PIN           14U
#define LED_PIN_MASK      (1U << LED_PIN)

#define STUB_WAIT_TIMEOUT      50000000U
#define BLINK_HALF_PERIOD_NOPS 200000U

typedef enum {
	HAL_OK = 0x00U,
	HAL_ERROR = 0x01U,
	HAL_TIMEOUT = 0x03U,
} HAL_StatusTypeDef;

typedef enum {
	HAL_DDR_SW_SELF_REFRESH_MODE = 0x0U,
	HAL_DDR_AUTO_SELF_REFRESH_MODE = 0x1U,
	HAL_DDR_HW_SELF_REFRESH_MODE = 0x2U,
	HAL_DDR_INVALID_MODE = 0x3U,
} HAL_DDR_SelfRefreshModeTypeDef;

extern uint8_t __sysram_text_start[];
extern uint8_t __sysram_text_end[];
extern uint8_t __sysram_text_load_start[];

__attribute__((section(".sysram_bss")))
static struct ddr_sr_result sysram_result;

__attribute__((section(".sysram_bss"), aligned(8)))
static uint8_t sysram_stack[1024];

static inline uint32_t mmio_read32(uint32_t addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void mmio_write32(uint32_t addr, uint32_t val)
{
	*(volatile uint32_t *)addr = val;
}

static inline void mmio_set_bits32(uint32_t addr, uint32_t mask)
{
	mmio_write32(addr, mmio_read32(addr) | mask);
}

static inline void mmio_clear_bits32(uint32_t addr, uint32_t mask)
{
	mmio_write32(addr, mmio_read32(addr) & ~mask);
}

static HAL_StatusTypeDef ddr_sr_mode_ssr(void)
{
	const uint32_t pwrctl_addr = DDRCTRL_BASE_ADDR + DDRCTRL_PWRCTL_OFFSET;

	/* SW mode in HAL: no auto self-refresh and no clock-disable LP mode. */
	mmio_clear_bits32(pwrctl_addr,
			      DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
			      DDRCTRL_PWRCTL_SELFREF_EN);

	return HAL_OK;
}

static HAL_StatusTypeDef ddr_sr_mode_asr(void)
{
	const uint32_t pwrctl_addr = DDRCTRL_BASE_ADDR + DDRCTRL_PWRCTL_OFFSET;

	/* ASR mode in HAL: auto self-refresh + clock-disable LP mode. */
	mmio_set_bits32(pwrctl_addr,
		    DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
		    DDRCTRL_PWRCTL_SELFREF_EN);

	return HAL_OK;
}

static HAL_StatusTypeDef ddr_sr_mode_hsr(void)
{
	const uint32_t pwrctl_addr = DDRCTRL_BASE_ADDR + DDRCTRL_PWRCTL_OFFSET;

	/* HSR mode in HAL: clock-disable LP mode enabled, auto self-refresh disabled. */
	mmio_set_bits32(pwrctl_addr, DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE);
	mmio_clear_bits32(pwrctl_addr, DDRCTRL_PWRCTL_SELFREF_EN);

	return HAL_OK;
}

static HAL_DDR_SelfRefreshModeTypeDef ddr_sr_read_mode(void)
{
	const uint32_t pwrctl_addr = DDRCTRL_BASE_ADDR + DDRCTRL_PWRCTL_OFFSET;
	uint32_t pwrctl = mmio_read32(pwrctl_addr);

	switch (pwrctl & (DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
				 DDRCTRL_PWRCTL_SELFREF_EN)) {
	case 0U:
		return HAL_DDR_SW_SELF_REFRESH_MODE;
	case DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE:
		return HAL_DDR_HW_SELF_REFRESH_MODE;
	case DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE | DDRCTRL_PWRCTL_SELFREF_EN:
		return HAL_DDR_AUTO_SELF_REFRESH_MODE;
	default:
		return HAL_DDR_INVALID_MODE;
	}
}

/* HAL-like helper: enter software self-refresh and wait for status transition. */
static HAL_StatusTypeDef ddr_sw_self_refresh_in(void)
{
	uint32_t stat;
	uint32_t timeout;
	const uint32_t pwrctl_addr = DDRCTRL_BASE_ADDR + DDRCTRL_PWRCTL_OFFSET;
	const uint32_t stat_addr = DDRCTRL_BASE_ADDR + DDRCTRL_STAT_OFFSET;

	mmio_set_bits32(pwrctl_addr, DDRCTRL_PWRCTL_SELFREF_SW);

	timeout = STUB_WAIT_TIMEOUT;
	do {
		stat = mmio_read32(stat_addr);
		if (((stat & DDRCTRL_STAT_OPERATING_MODE_Msk) == DDRCTRL_STAT_OPERATING_MODE_SR) &&
		    ((stat & DDRCTRL_STAT_SELFREF_TYPE_Msk) == DDRCTRL_STAT_SELFREF_TYPE_SW)) {
			return HAL_OK;
		}
	} while (--timeout != 0U);

	return HAL_TIMEOUT;
}

/* HAL-like helper: clear software self-refresh request and wait for normal mode. */
static HAL_StatusTypeDef ddr_sw_self_refresh_out(void)
{
	uint32_t stat;
	uint32_t timeout;
	const uint32_t pwrctl_addr = DDRCTRL_BASE_ADDR + DDRCTRL_PWRCTL_OFFSET;
	const uint32_t stat_addr = DDRCTRL_BASE_ADDR + DDRCTRL_STAT_OFFSET;

	mmio_clear_bits32(pwrctl_addr, DDRCTRL_PWRCTL_SELFREF_SW);

	timeout = STUB_WAIT_TIMEOUT;
	do {
		stat = mmio_read32(stat_addr);
		if ((stat & DDRCTRL_STAT_OPERATING_MODE_Msk) == DDRCTRL_STAT_OPERATING_MODE_NML) {
			return HAL_OK;
		}
	} while (--timeout != 0U);

	return HAL_TIMEOUT;
}

/* Copied/adapted from HAL_DDR_SR_Entry. */
static HAL_StatusTypeDef HAL_DDR_SR_Entry(uint32_t *zq0cr0_zdata)
{
	if (zq0cr0_zdata != NULL) {
		*zq0cr0_zdata = mmio_read32(DDRPHYC_BASE_ADDR + DDRPHYC_ZQ0CR0_OFFSET) & DDRPHYC_ZQ0CR0_ZDATA_Msk;
	}

	if (ddr_sw_self_refresh_in() != HAL_OK) {
		return HAL_ERROR;
	}

	mmio_set_bits32(PWR_BASE_ADDR + PWR_CR3_OFFSET, PWR_CR3_DDRSREN);

	return HAL_OK;
}

/* Copied/adapted from HAL_DDR_SR_Exit. */
static HAL_StatusTypeDef HAL_DDR_SR_Exit(void)
{
	HAL_StatusTypeDef ret;

	ret = ddr_sw_self_refresh_out();
	if (ret != HAL_OK) {
		return ret;
	}

	mmio_clear_bits32(PWR_BASE_ADDR + PWR_CR3_OFFSET, PWR_CR3_DDRRETEN);

	return HAL_OK;
}

/* Copied/adapted from HAL_DDR_SR_SetMode. */
static HAL_StatusTypeDef HAL_DDR_SR_SetMode(HAL_DDR_SelfRefreshModeTypeDef mode)
{
	HAL_StatusTypeDef ret;

	switch (mode) {
	case HAL_DDR_SW_SELF_REFRESH_MODE:
		ret = ddr_sr_mode_ssr();
		break;
	case HAL_DDR_AUTO_SELF_REFRESH_MODE:
		ret = ddr_sr_mode_asr();
		break;
	case HAL_DDR_HW_SELF_REFRESH_MODE:
		ret = ddr_sr_mode_hsr();
		break;
	default:
		ret = HAL_ERROR;
		break;
	}

	return ret;
}

/* Copied/adapted from HAL_DDR_SR_ReadMode. */
static HAL_DDR_SelfRefreshModeTypeDef HAL_DDR_SR_ReadMode(void)
{
	return ddr_sr_read_mode();
}

static void led_init(void)
{
	uint32_t moder;
	uint32_t otyper;
	uint32_t pupdr;

	moder = mmio_read32(GPIOA_BASE_ADDR + GPIO_MODER_OFFSET);
	moder &= ~(0x3U << (LED_PIN * 2U));
	moder |= (0x1U << (LED_PIN * 2U));
	mmio_write32(GPIOA_BASE_ADDR + GPIO_MODER_OFFSET, moder);

	otyper = mmio_read32(GPIOA_BASE_ADDR + GPIO_OTYPER_OFFSET);
	otyper &= ~LED_PIN_MASK;
	mmio_write32(GPIOA_BASE_ADDR + GPIO_OTYPER_OFFSET, otyper);

	pupdr = mmio_read32(GPIOA_BASE_ADDR + GPIO_PUPDR_OFFSET);
	pupdr &= ~(0x3U << (LED_PIN * 2U));
	mmio_write32(GPIOA_BASE_ADDR + GPIO_PUPDR_OFFSET, pupdr);

	/* Active-low LED off. */
	mmio_write32(GPIOA_BASE_ADDR + GPIO_BSRR_OFFSET, LED_PIN_MASK);
}

static void copy_sysram_sections(void)
{
	size_t size;

	size = (size_t)(__sysram_text_end - __sysram_text_start);
	memcpy(__sysram_text_start, __sysram_text_load_start, size);
}

__attribute__((section(".sysram_text"), noinline, used))
static void ddr_sr_stub(struct ddr_sr_result *result)
{
	uint32_t stat;
	HAL_StatusTypeDef ret;
	const uint32_t stat_addr = DDRCTRL_BASE_ADDR + DDRCTRL_STAT_OFFSET;
	const uint32_t gpioa_bsrr_addr = GPIOA_BASE_ADDR + GPIO_BSRR_OFFSET;

	result->started = 1U;
	result->phase = 1U;

	if (HAL_DDR_SR_SetMode(HAL_DDR_SW_SELF_REFRESH_MODE) != HAL_OK) {
		result->phase = 0xE0U;
		return;
	}

	if (HAL_DDR_SR_ReadMode() != HAL_DDR_SW_SELF_REFRESH_MODE) {
		result->phase = 0xE4U;
		return;
	}

	ret = HAL_DDR_SR_Entry(&result->zdata);
	result->phase = 2U;
	stat = mmio_read32(stat_addr);
	result->entry_stat = stat;
	result->entry_ok = (ret == HAL_OK) ? 1U : 0U;
	if (!result->entry_ok) {
		result->phase = 0xE1U;
		return;
	}

	result->phase = 3U;

	for (uint32_t blink_count = 0U; blink_count < 10U; blink_count++) {
		mmio_write32(gpioa_bsrr_addr, LED_PIN_MASK << 16);
		for (volatile uint32_t delay = 0U; delay < BLINK_HALF_PERIOD_NOPS; delay++) {
			__asm__ volatile("nop");
		}

		mmio_write32(gpioa_bsrr_addr, LED_PIN_MASK);
		for (volatile uint32_t delay = 0U; delay < BLINK_HALF_PERIOD_NOPS; delay++) {
			__asm__ volatile("nop");
		}
	}

	ret = HAL_DDR_SR_Exit();
	result->phase = 4U;
	stat = mmio_read32(stat_addr);
	result->exit_stat = stat;
	result->exit_ok = (ret == HAL_OK) ? 1U : 0U;
	if (!result->exit_ok) {
		result->phase = 0xE2U;
		result->finished = 1U;
		return;
	}

	result->phase = 5U;
	result->finished = 1U;
}

void sysram_init(void)
{
	led_init();
	copy_sysram_sections();
	__asm__ volatile("dsb" ::: "memory");
	__asm__ volatile("isb" ::: "memory");
	memset(&sysram_result, 0, sizeof(sysram_result));
}

__attribute__((section(".sysram_text"), noinline, used))
void sysram_run(void)
{
	register uintptr_t old_sp;
	uintptr_t new_sp = (uintptr_t)(sysram_stack + sizeof(sysram_stack));

	__asm__ volatile("mov %0, sp" : "=r"(old_sp));
	__asm__ volatile("mov sp, %0" : : "r"(new_sp) : "memory");

	ddr_sr_stub(&sysram_result);

	__asm__ volatile("mov sp, %0" : : "r"(old_sp) : "memory");
}

const struct ddr_sr_result *sysram_get_result(void)
{
	return &sysram_result;
}

uintptr_t sysram_get_stub_addr(void)
{
	return (uintptr_t)ddr_sr_stub & ~(uintptr_t)0x1U;
}

uintptr_t sysram_get_result_addr(void)
{
	return (uintptr_t)&sysram_result;
}

uintptr_t sysram_get_stack_addr(void)
{
	return (uintptr_t)sysram_stack;
}

size_t sysram_get_text_size(void)
{
	return (size_t)(__sysram_text_end - __sysram_text_start);
}
