/*
 * mp135dk_lp_full_demo.c
 *
 * STM32MP135F-DK low-power Zephyr demo:
 * - Wake source 1: RTC Alarm A (via EXTI line 19)
 * - Wake source 2: WKUP1 pin (GPIOF8)
 * - Enter CStop, wake on either source
 * - Poll-based wake attribution in app path (no app-level IRQ ownership)
 *
 * Important:
 * - Must run in privileged secure context if secure firmware owns these regs.
 * - If code executes from DDR during CStop, add DDR self-refresh hooks.
 */

#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>

#include "sysram.h"

/* ============================================================
 * Basic helpers
 * ============================================================ */
typedef uint32_t u32;

static inline u32 mmio_read(u32 addr) { return *(volatile u32 *)addr; }
static inline void mmio_write(u32 addr, u32 v) { *(volatile u32 *)addr = v; }
static inline void mmio_setbits(u32 addr, u32 mask) { mmio_write(addr, mmio_read(addr) | mask); }
static inline void mmio_clrbits(u32 addr, u32 mask) { mmio_write(addr, mmio_read(addr) & ~mask); }
static inline uint8_t mmio_read8(u32 addr) { return *(volatile uint8_t *)addr; }
static inline void mmio_write8(u32 addr, uint8_t v) { *(volatile uint8_t *)addr = v; }

static inline void mmio_clrset(u32 addr, u32 clr_mask, u32 set_mask)
{
    u32 v = mmio_read(addr);
    v &= ~clr_mask;
    v |= set_mask;
    mmio_write(addr, v);
}

static inline void cpu_dsb_isb(void)
{
    __asm__ volatile ("dsb sy\nisb\n" : : : "memory");
}

static inline void cpu_wfi(void)
{
    __asm__ volatile ("wfi");
}

static inline void cpu_enable_irq(void)
{
    __asm__ volatile ("cpsie i");
}

static inline void cpu_disable_irq(void)
{
    __asm__ volatile ("cpsid i");
}

/* ============================================================
 * MP13 base addresses used on MP135-DK
 * Guarded with #ifndef so STM32 HAL header definitions take precedence.
 * ============================================================ */
#ifndef RCC_BASE
#define RCC_BASE              0x50000000u
#endif
#ifndef PWR_BASE
#define PWR_BASE              0x50001000u
#endif
#ifndef EXTI_BASE
#define EXTI_BASE             0x5000D000u
#endif
#ifndef RTC_BASE
#define RTC_BASE              0x5C004000u
#endif
#ifndef GPIOA_BASE
#define GPIOA_BASE            0x50002000u
#endif

/* RTC IRQ index in this build (owned by Zephyr RTC driver, not app). */
#define IRQ_RTC_DTS           DT_IRQN(DT_NODELABEL(rtc))

/* GPIOA LD3 (active-low) on STM32MP135-DK for post-wake proof-of-life */
#define GPIO_MODER            0x00u
#define GPIO_BSRR             0x18u
#define LED_PIN               14u
#define LED_MASK              BIT(LED_PIN)

/* ============================================================
 * RCC (MP13) — register offsets (not base addresses)
 * ============================================================ */
#define RCC_MP_SREQSETR       0x100u
#define RCC_MP_SREQCLRR       0x104u
#define RCC_PWRLPDLYCR        0x110u
#define RCC_MP_CIER           0x200u
#define RCC_MP_CIFR           0x204u

#ifndef RCC_MP_SREQSETR_STPREQ_P0
#define RCC_MP_SREQSETR_STPREQ_P0    BIT(0)
#endif
#ifndef RCC_MP_SREQCLRR_STPREQ_P0
#define RCC_MP_SREQCLRR_STPREQ_P0    BIT(0)
#endif
#ifndef RCC_MP_CIER_WKUPIE
#define RCC_MP_CIER_WKUPIE           BIT(20)
#endif
#ifndef RCC_MP_CIFR_WKUPF
#define RCC_MP_CIFR_WKUPF            BIT(20)
#endif
#define RCC_PWRLPDLYCR_PWRLP_DLY_MASK 0x003FFFFFu

/* ============================================================
 * PWR (MP13) — register offsets and bit masks
 * ============================================================ */
#define PWR_CR1_OFF           0x00u
#ifndef PWR_CR1_LPDS
#define PWR_CR1_LPDS          BIT(0)
#endif
#ifndef PWR_CR1_LPCFG
#define PWR_CR1_LPCFG         BIT(1)
#endif
#ifndef PWR_CR1_LVDS
#define PWR_CR1_LVDS          BIT(2)
#endif
#ifndef PWR_CR1_STOP2
#define PWR_CR1_STOP2         BIT(3)
#endif
#ifndef PWR_CR1_DBP
#define PWR_CR1_DBP           BIT(8)
#endif

#define PWR_MPUCR_OFF         0x10u
#ifndef PWR_MPUCR_PDDS
#define PWR_MPUCR_PDDS        BIT(0)
#endif
#ifndef PWR_MPUCR_CSSF
#define PWR_MPUCR_CSSF        BIT(9)
#endif

#define PWR_WKUPCR_OFF        0x20u
#define PWR_WKUPFR_OFF        0x24u
#define PWR_MPUWKUPENR_OFF    0x28u

#define WKUP_EDGE_SHIFT       8u
#define WKUP_PULL_SHIFT       16u

enum wkup_pull {
    WKUP_NO_PULL   = 0u,
    WKUP_PULL_UP   = 1u,
    WKUP_PULL_DOWN = 2u,
};

/* ============================================================
 * EXTI (MP13 bank1 for lines 0..31)
 * ============================================================ */
#define EXTI_RTSR1            0x00u
#define EXTI_FTSR1            0x04u
#define EXTI_RPR1             0x0Cu
#define EXTI_FPR1             0x10u
#define EXTI_IMR1             0x80u

/* ============================================================
 * RTC register offsets and bit masks
 * ============================================================ */
#define RTC_TR                0x00u
#define RTC_DR                0x04u
#define RTC_CR                0x18u
#define RTC_WPR               0x24u
#define RTC_ALRMAR            0x40u
#define RTC_SR                0x50u
#define RTC_SCR               0x5Cu

#define RTC_WPR_KEY1          0xCAu
#define RTC_WPR_KEY2          0x53u
#define RTC_WPR_KEY_LOCK      0xFFu

#ifndef RTC_CR_ALRAE
#define RTC_CR_ALRAE          BIT(8)
#endif
#ifndef RTC_CR_ALRAIE
#define RTC_CR_ALRAIE         BIT(12)
#endif
#ifndef RTC_SR_ALRAF
#define RTC_SR_ALRAF          BIT(0)
#endif
#ifndef RTC_SCR_CALRAF
#define RTC_SCR_CALRAF        BIT(0)
#endif

/* TR decode fields */
#define RTC_TR_SU_MASK        0x0000000Fu
#define RTC_TR_ST_MASK        0x00000070u
#define RTC_TR_MNU_MASK       0x00000F00u
#define RTC_TR_MNT_MASK       0x00007000u
#define RTC_TR_HU_MASK        0x000F0000u
#define RTC_TR_HT_MASK        0x00300000u

/* DR decode fields */
#define RTC_DR_DU_MASK        0x0000000Fu
#define RTC_DR_DT_MASK        0x00000030u

/*********************
 * Global wake flags
 *********************/
static volatile u32 g_wake_flags = 0;
#define WAKEF_RTC   BIT(0)
#define WAKEF_WKUP1 BIT(1)
#define WAKEF_RCC   BIT(2)

static const struct device *rtc_dev = DEVICE_DT_GET(DT_NODELABEL(rtc));

static void rtc_alarm_cb(const struct device *dev, uint16_t id, void *user_data)
{
    (void)dev;
    (void)id;
    (void)user_data;
    g_wake_flags |= WAKEF_RTC;
}

/* ============================================================
 * GIC helpers
 * ============================================================ */

/* ============================================================
 * RTC helpers
 * ============================================================ */
static inline u32 bcd_to_bin(u32 tens, u32 units)
{
    return tens * 10u + units;
}

static inline void rtc_unlock(void)
{
    mmio_write(RTC_BASE + RTC_WPR, RTC_WPR_KEY1);
    mmio_write(RTC_BASE + RTC_WPR, RTC_WPR_KEY2);
}

static inline void rtc_lock(void)
{
    mmio_write(RTC_BASE + RTC_WPR, RTC_WPR_KEY_LOCK);
}

static void rtc_enable_backup_domain_write(void)
{
    u32 tries = 100000u;

    mmio_setbits(PWR_BASE + PWR_CR1_OFF, PWR_CR1_DBP);
    while (((mmio_read(PWR_BASE + PWR_CR1_OFF) & PWR_CR1_DBP) == 0u) && (tries-- > 0u)) {
        ;
    }
}

static void led_init(void)
{
    u32 moder = mmio_read(GPIOA_BASE + GPIO_MODER);
    moder &= ~(0x3u << (LED_PIN * 2u));
    moder |=  (0x1u << (LED_PIN * 2u));
    mmio_write(GPIOA_BASE + GPIO_MODER, moder);
}

static void led_on(void)
{
    /* Active-low LED: reset bit drives output low (ON). */
    mmio_write(GPIOA_BASE + GPIO_BSRR, LED_MASK << 16u);
}

static void led_off(void)
{
    mmio_write(GPIOA_BASE + GPIO_BSRR, LED_MASK);
}

static void led_blink_postwake(void)
{
    for (u32 i = 0; i < 3u; i++) {
        led_on();
        for (volatile u32 d = 0; d < 350000u; d++) { }
        led_off();
        for (volatile u32 d = 0; d < 350000u; d++) { }
    }
}

static void rtc_alarm_a_disable(void)
{
    u32 cr;

    rtc_enable_backup_domain_write();
    rtc_unlock();
    cr = mmio_read(RTC_BASE + RTC_CR);
    cr &= ~(RTC_CR_ALRAE | RTC_CR_ALRAIE);
    mmio_write(RTC_BASE + RTC_CR, cr);
    mmio_write(RTC_BASE + RTC_SCR, RTC_SCR_CALRAF);
    rtc_lock();
}

static int rtc_driver_alarm_after_seconds(u32 delta_s)
{
    struct rtc_time now;
    struct rtc_time alarm;
    u32 total;
    int ret;

    if (!device_is_ready(rtc_dev)) {
        return -ENODEV;
    }

    ret = rtc_get_time(rtc_dev, &now);
    if (ret < 0) {
        return ret;
    }

    alarm = now;
    total = (u32)now.tm_hour * 3600u + (u32)now.tm_min * 60u + (u32)now.tm_sec + delta_s;

    alarm.tm_hour = (int)((total / 3600u) % 24u);
    alarm.tm_min = (int)((total % 3600u) / 60u);
    alarm.tm_sec = (int)(total % 60u);

    ret = rtc_alarm_set_callback(rtc_dev, 0u, rtc_alarm_cb, NULL);
    if (ret < 0) {
        return ret;
    }

    ret = rtc_alarm_set_time(rtc_dev, 0u,
        RTC_ALARM_TIME_MASK_SECOND |
        RTC_ALARM_TIME_MASK_MINUTE |
        RTC_ALARM_TIME_MASK_HOUR,
        &alarm);
    return ret;
}

/*
 * Set Alarm A relative to current time by delta seconds.
 * Simplified demo: handles same-day rollover and day increment 1..31.
 */
static void rtc_set_alarm_after_seconds(u32 delta_s)
{
    u32 tr = mmio_read(RTC_BASE + RTC_TR);
    u32 dr = mmio_read(RTC_BASE + RTC_DR);

    u32 sec = bcd_to_bin((tr & RTC_TR_ST_MASK) >> 4, (tr & RTC_TR_SU_MASK));
    u32 min = bcd_to_bin((tr & RTC_TR_MNT_MASK) >> 12, (tr & RTC_TR_MNU_MASK) >> 8);
    u32 hour = bcd_to_bin((tr & RTC_TR_HT_MASK) >> 20, (tr & RTC_TR_HU_MASK) >> 16);
    u32 day = bcd_to_bin((dr & RTC_DR_DT_MASK) >> 4, (dr & RTC_DR_DU_MASK));

    u32 total = hour * 3600u + min * 60u + sec + delta_s;
    u32 day_inc = total / 86400u;
    u32 rem = total % 86400u;

    u32 ah = rem / 3600u;
    u32 am = (rem % 3600u) / 60u;
    u32 as = rem % 60u;
    u32 ad = day + day_inc;
    while (ad > 31u) ad -= 31u;

    u32 alrmar = 0u;

    /* sec */
    alrmar |= ((as % 10u) << 0);
    alrmar |= ((as / 10u) << 4);

    /* min */
    alrmar |= ((am % 10u) << 8);
    alrmar |= ((am / 10u) << 12);

    /* hour */
    alrmar |= ((ah % 10u) << 16);
    alrmar |= ((ah / 10u) << 20);

    /* date */
    alrmar |= ((ad % 10u) << 24);
    alrmar |= ((ad / 10u) << 28);

    rtc_enable_backup_domain_write();
    rtc_unlock();

    /* Disable, program, clear stale flag, re-enable */
    mmio_clrbits(RTC_BASE + RTC_CR, RTC_CR_ALRAE | RTC_CR_ALRAIE);
    mmio_write(RTC_BASE + RTC_ALRMAR, alrmar);
    mmio_write(RTC_BASE + RTC_SCR, RTC_SCR_CALRAF);
    mmio_setbits(RTC_BASE + RTC_CR, RTC_CR_ALRAE | RTC_CR_ALRAIE);

    rtc_lock();

    printk("rtc arm: CR=0x%08lx SR=0x%08lx ALRMAR=0x%08lx\n",
        (unsigned long)mmio_read(RTC_BASE + RTC_CR),
        (unsigned long)mmio_read(RTC_BASE + RTC_SR),
        (unsigned long)mmio_read(RTC_BASE + RTC_ALRMAR));
}

/* ============================================================
 * EXTI19 for RTC alarm
 * ============================================================ */
static void exti19_rtc_arm(void)
{
    u32 bit = BIT(19);

    /* Rising trigger */
    mmio_setbits(EXTI_BASE + EXTI_RTSR1, bit);
    mmio_clrbits(EXTI_BASE + EXTI_FTSR1, bit);

    /* Clear pending */
    mmio_write(EXTI_BASE + EXTI_RPR1, bit);
    mmio_write(EXTI_BASE + EXTI_FPR1, bit);

    /* Unmask */
    mmio_setbits(EXTI_BASE + EXTI_IMR1, bit);
}

/* ============================================================
 * WKUP1 setup on MP135-DK (GPIOF8)
 * ============================================================ */
static void wkup1_arm_rising(enum wkup_pull pull)
{
    /* WKUP1 is index 0 */
    const u32 idx = 0u;
    const u32 wkup_bit = BIT(idx);
    const u32 edge_bit = BIT(WKUP_EDGE_SHIFT + idx);
    const u32 pull_shift = WKUP_PULL_SHIFT + (idx * 2u);
    const u32 pull_mask = (0x3u << pull_shift);

    /* disable before changing */
    mmio_clrbits(PWR_BASE + PWR_MPUWKUPENR_OFF, wkup_bit);

    /* Rising edge => clear edge bit for MP13 WKUPCR semantics. */
    mmio_clrbits(PWR_BASE + PWR_WKUPCR_OFF, edge_bit);

    /* Pull config */
    mmio_clrset(PWR_BASE + PWR_WKUPCR_OFF, pull_mask, ((u32)pull << pull_shift));

    /* clear stale flag then enable */
    mmio_setbits(PWR_BASE + PWR_WKUPCR_OFF, wkup_bit);
    mmio_setbits(PWR_BASE + PWR_MPUWKUPENR_OFF, wkup_bit);
}

static void cstop_cleanup_after_wake(void)
{
    /* Disable stop request */
    mmio_setbits(RCC_BASE + RCC_MP_SREQCLRR, RCC_MP_SREQCLRR_STPREQ_P0);

    /* Disable and clear RCC wake flag */
    mmio_clrbits(RCC_BASE + RCC_MP_CIER, RCC_MP_CIER_WKUPIE);
    mmio_setbits(RCC_BASE + RCC_MP_CIFR, RCC_MP_CIFR_WKUPF);

    /* Clear source flags */
    mmio_write(RTC_BASE + RTC_SCR, RTC_SCR_CALRAF);
    mmio_setbits(PWR_BASE + PWR_WKUPCR_OFF, BIT(0));
}

/* ============================================================
 * Wake source implementations live in the Zephyr ISR callbacks above.
 * The bare-metal irq_handler (GIC IAR/EOIR) is removed to avoid conflicting
 * with Zephyr's interrupt controller management.
 * ============================================================ */

/* ============================================================
 * Optional board stubs
 * ============================================================ */
__attribute__((weak)) void board_clock_init(void) {}
__attribute__((weak)) void board_uart_init(void) {}
__attribute__((weak)) void board_gpio_init(void) {}
__attribute__((weak)) void board_log(const char *s)
{
    printk("%s\n", s);
}

/* ============================================================
 * Main sequence requested
 * ============================================================ */
int main(void)
{
    uintptr_t stub_addr;
    uintptr_t stack_addr;
    uintptr_t result_addr;
    const struct ddr_sr_result *sr;
    u32 rtc_sr_post;
    u32 exti_rpr1_post;
    u32 pwr_wkupfr_post;
    u32 rcc_cifr_post;
    u32 cyc_before;
    u32 cyc_after;
    u32 pwr_mpucr_before;
    u32 pwr_mpucr_after;
    u32 pwr_cr1_before;
    u32 pwr_cr1_after;
    u32 sreq_set_before;
    u32 sreq_set_after;

    printk("main reached\n");

    sysram_init();
    led_init();
    stub_addr = sysram_get_stub_addr();
    stack_addr = sysram_get_stack_addr();
    result_addr = sysram_get_result_addr();
    printk("sysram: stub=0x%08lx stack=0x%08lx result=0x%08lx text_size=%lu\n",
           (unsigned long)stub_addr,
           (unsigned long)stack_addr,
           (unsigned long)result_addr,
           (unsigned long)sysram_get_text_size());

    printk("skip gic_init for poll-wake test\n");

    printk("before rtc setup\n");
        printk("irq map: rtc=%u (owned by rtc driver), pwr=125 (platform-owned)\n",
            (unsigned int)IRQ_RTC_DTS);

    /* RTC IRQ is already owned by Zephyr RTC driver on this target.
     * Do not re-register IRQ 35 here; use post-wake register polling. */
    /* IRQ_PWR_WAKEUP (125) is also platform-owned in this app path. */

    rtc_alarm_a_disable();
    exti19_rtc_arm();
    wkup1_arm_rising(WKUP_PULL_DOWN);

    /* Clear any stale wake status right before stop entry. */
    mmio_write(EXTI_BASE + EXTI_RPR1, BIT(19));
    mmio_write(EXTI_BASE + EXTI_FPR1, BIT(19));
    mmio_write(RTC_BASE + RTC_SCR, RTC_SCR_CALRAF);
    mmio_setbits(PWR_BASE + PWR_WKUPCR_OFF, BIT(0));
    mmio_setbits(RCC_BASE + RCC_MP_CIFR, RCC_MP_CIFR_WKUPF);

    printk("after rtc setup\n");

    rtc_sr_post = 0u;
    exti_rpr1_post = 0u;
    pwr_wkupfr_post = 0u;
    rcc_cifr_post = 0u;
    pwr_mpucr_before = 0u;
    pwr_mpucr_after = 0u;
    pwr_cr1_before = 0u;
    pwr_cr1_after = 0u;
    sreq_set_before = 0u;
    sreq_set_after = 0u;
    cyc_before = 0u;
    cyc_after = 0u;

    /* Single attempt; arm through Zephyr RTC driver path. */
    if (rtc_driver_alarm_after_seconds(3u) < 0) {
        /* Fallback to direct programming if driver alarm path is unavailable. */
        rtc_set_alarm_after_seconds(3u);
    }

    mmio_write(EXTI_BASE + EXTI_RPR1, BIT(19));
    mmio_write(EXTI_BASE + EXTI_FPR1, BIT(19));
    mmio_write(RTC_BASE + RTC_SCR, RTC_SCR_CALRAF);
    mmio_setbits(PWR_BASE + PWR_WKUPCR_OFF, BIT(0));
    mmio_setbits(RCC_BASE + RCC_MP_CIFR, RCC_MP_CIFR_WKUPF);

    pwr_mpucr_before = mmio_read(PWR_BASE + PWR_MPUCR_OFF);
    pwr_cr1_before = mmio_read(PWR_BASE + PWR_CR1_OFF);
    sreq_set_before = mmio_read(RCC_BASE + RCC_MP_SREQSETR);
    cyc_before = k_cycle_get_32();

    /* Pre-WFI sanity snapshot. */
    printk("pre-wfi: CIER=0x%08lx CIFR=0x%08lx SREQSETR=0x%08lx\n",
        (unsigned long)mmio_read(RCC_BASE + RCC_MP_CIER),
        (unsigned long)mmio_read(RCC_BASE + RCC_MP_CIFR),
        (unsigned long)mmio_read(RCC_BASE + RCC_MP_SREQSETR));
    printk("pre-wfi: MPUWKUPENR=0x%08lx WKUPCR=0x%08lx WKUPFR=0x%08lx\n",
        (unsigned long)mmio_read(PWR_BASE + PWR_MPUWKUPENR_OFF),
        (unsigned long)mmio_read(PWR_BASE + PWR_WKUPCR_OFF),
        (unsigned long)mmio_read(PWR_BASE + PWR_WKUPFR_OFF));
    printk("pre-wfi: EXTI_IMR1=0x%08lx RTSR1=0x%08lx RPR1=0x%08lx\n",
        (unsigned long)mmio_read(EXTI_BASE + EXTI_IMR1),
        (unsigned long)mmio_read(EXTI_BASE + EXTI_RTSR1),
        (unsigned long)mmio_read(EXTI_BASE + EXTI_RPR1));
    printk("pre-wfi: RTC_CR=0x%08lx RTC_SR=0x%08lx PWR_CR1=0x%08lx MPUCR=0x%08lx\n",
        (unsigned long)mmio_read(RTC_BASE + RTC_CR),
        (unsigned long)mmio_read(RTC_BASE + RTC_SR),
        (unsigned long)mmio_read(PWR_BASE + PWR_CR1_OFF),
        (unsigned long)mmio_read(PWR_BASE + PWR_MPUCR_OFF));

    cpu_enable_irq();
    printk("before cstop enter (RTC in ~3s or WKUP1 pin)\n");
    sysram_cstop_enter_lp_stop(0U);

    cyc_after = k_cycle_get_32();
    pwr_mpucr_after = mmio_read(PWR_BASE + PWR_MPUCR_OFF);
    pwr_cr1_after = mmio_read(PWR_BASE + PWR_CR1_OFF);
    sreq_set_after = mmio_read(RCC_BASE + RCC_MP_SREQSETR);

    rtc_sr_post = mmio_read(RTC_BASE + RTC_SR);
    exti_rpr1_post = mmio_read(EXTI_BASE + EXTI_RPR1);
    pwr_wkupfr_post = mmio_read(PWR_BASE + PWR_WKUPFR_OFF);
    rcc_cifr_post = mmio_read(RCC_BASE + RCC_MP_CIFR);

    g_wake_flags = 0u;
    if ((rtc_sr_post & RTC_SR_ALRAF) != 0u) {
        g_wake_flags |= WAKEF_RTC;
    }
    if ((exti_rpr1_post & BIT(19)) != 0u) {
        g_wake_flags |= WAKEF_RTC;
    }
    if ((pwr_wkupfr_post & BIT(0)) != 0u) {
        g_wake_flags |= WAKEF_WKUP1;
    }
    if ((rcc_cifr_post & RCC_MP_CIFR_WKUPF) != 0u) {
        g_wake_flags |= WAKEF_RCC;
    }

    /* Proof-of-life independent of UART after wake/return path. */
    led_blink_postwake();

        sr = sysram_get_result();
        board_uart_init();
    printk("after cstop enter\n");
        printk("wake attempts: 1 real=%lu\n",
            (unsigned long)(g_wake_flags != 0u ? 1u : 0u));
        printk("stop diag: dcyc=%lu PWR_MPUCR pre=0x%08lx post=0x%08lx PWR_CR1 pre=0x%08lx post=0x%08lx SREQSETR pre=0x%08lx post=0x%08lx\n",
            (unsigned long)(cyc_after - cyc_before),
            (unsigned long)pwr_mpucr_before,
            (unsigned long)pwr_mpucr_after,
            (unsigned long)pwr_cr1_before,
            (unsigned long)pwr_cr1_after,
            (unsigned long)sreq_set_before,
            (unsigned long)sreq_set_after);
        printk("sysram wake: started=0x%08lx phase=0x%08lx finished=0x%08lx wake_flags=0x%08lx\n",
            (unsigned long)sr->started,
            (unsigned long)sr->phase,
            (unsigned long)sr->finished,
            (unsigned long)g_wake_flags);
        printk("sysram internals: entry_stat=0x%08lx exit_stat=0x%08lx tries=%lu wake_seen=%lu\n",
            (unsigned long)sr->entry_stat,
            (unsigned long)sr->exit_stat,
            (unsigned long)sr->entry_ok,
            (unsigned long)sr->exit_ok);
        printk("wake regs: RTC_SR=0x%08lx EXTI_RPR1=0x%08lx PWR_WKUPFR=0x%08lx RCC_CIFR=0x%08lx\n",
            (unsigned long)rtc_sr_post,
            (unsigned long)exti_rpr1_post,
            (unsigned long)pwr_wkupfr_post,
            (unsigned long)rcc_cifr_post);

    cstop_cleanup_after_wake();
    printk("after cstop cleanup\n");

    while (1) {
        cpu_wfi();
    }
    return 0;
}