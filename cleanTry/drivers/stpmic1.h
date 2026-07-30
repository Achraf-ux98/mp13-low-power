/*
 * STPMIC1 driver wrapper for this application.
 *
 * This keeps the TF-A / OP-TEE STPMIC1 API shape while using Zephyr I2C.
 */

#ifndef STPMIC1_DRIVER_H
#define STPMIC1_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <zephyr/sys/util.h> /* BIT, GENMASK */
#include <zephyr/drivers/i2c.h>

/*
 * STPMIC1 register map and bit definitions.
 *
 * Localized here so this driver does not depend on TF-A / OP-TEE headers.
 */

/* Register addresses */
#define VERSION_STATUS_REG               0x06U
#define MAIN_CONTROL_REG                 0x10U
#define BUCK_PULL_DOWN_REG               0x12U
#define MASK_RESET_BUCK_REG              0x18U
#define MASK_RESET_LDO_REG               0x1AU
#define BUCK_ICC_TURNOFF_REG             0x1DU
#define LDO_ICC_TURNOFF_REG              0x1EU
#define BUCK1_CONTROL_REG                0x20U
#define BUCK2_CONTROL_REG                0x21U
#define BUCK3_CONTROL_REG                0x22U
#define BUCK4_CONTROL_REG                0x23U
#define VREF_DDR_CONTROL_REG             0x24U
#define LDO1_CONTROL_REG                 0x25U
#define LDO2_CONTROL_REG                 0x26U
#define LDO3_CONTROL_REG                 0x27U
#define LDO4_CONTROL_REG                 0x28U
#define LDO5_CONTROL_REG                 0x29U
#define LDO6_CONTROL_REG                 0x2AU
#define BUCK1_PWRCTRL_REG                0x30U
#define BUCK2_PWRCTRL_REG                0x31U
#define BUCK3_PWRCTRL_REG                0x32U
#define BUCK4_PWRCTRL_REG                0x33U
#define VREF_DDR_PWRCTRL_REG             0x34U
#define LDO1_PWRCTRL_REG                 0x35U
#define LDO2_PWRCTRL_REG                 0x36U
#define LDO3_PWRCTRL_REG                 0x37U
#define LDO4_PWRCTRL_REG                 0x38U
#define LDO5_PWRCTRL_REG                 0x39U
#define LDO6_PWRCTRL_REG                 0x3AU
#define USB_CONTROL_REG                  0x40U

/* Register masks */
#define LDO_VOLTAGE_MASK                 GENMASK(6, 2)
#define BUCK_VOLTAGE_MASK                GENMASK(7, 2)
#define LDO_BUCK_VOLTAGE_SHIFT           2
#define LDO_BUCK_ENABLE_MASK             BIT(0)
#define LDO_BUCK_HPLP_MASK               BIT(1)
#define LDO_BUCK_RESET_MASK              BIT(0)
#define LDO_BUCK_PULL_DOWN_MASK          GENMASK(1, 0)

/* Pull-down shifts */
#define BUCK1_PULL_DOWN_SHIFT            0
#define BUCK2_PULL_DOWN_SHIFT            2
#define BUCK3_PULL_DOWN_SHIFT            4
#define BUCK4_PULL_DOWN_SHIFT            6

/* ICC shifts */
#define BUCK1_ICC_SHIFT                  0
#define BUCK2_ICC_SHIFT                  1
#define BUCK3_ICC_SHIFT                  2
#define BUCK4_ICC_SHIFT                  3
#define PWR_SW1_ICC_SHIFT                4
#define PWR_SW2_ICC_SHIFT                5
#define BOOST_ICC_SHIFT                  6
#define LDO1_ICC_SHIFT                   0
#define LDO2_ICC_SHIFT                   1
#define LDO3_ICC_SHIFT                   2
#define LDO4_ICC_SHIFT                   3
#define LDO5_ICC_SHIFT                   4
#define LDO6_ICC_SHIFT                   5

/* Mask reset bits */
#define BUCK1_MASK_RESET                 0
#define BUCK2_MASK_RESET                 1
#define BUCK3_MASK_RESET                 2
#define BUCK4_MASK_RESET                 3
#define LDO1_MASK_RESET                  0
#define LDO2_MASK_RESET                  1
#define LDO3_MASK_RESET                  2
#define LDO4_MASK_RESET                  3
#define LDO5_MASK_RESET                  4
#define LDO6_MASK_RESET                  5
#define VREF_DDR_MASK_RESET              6

/* LDO3 special modes */
#define LDO3_BYPASS                      BIT(7)
#define LDO3_DDR_SEL                     31U

/* Main PMIC control */
#define PWRCTRL_PIN_VALID                BIT(2)
#define SOFTWARE_SWITCH_OFF_ENABLED      BIT(0)

/* USB control */
#define SW_OUT_DISCHARGE                 BIT(5)
#define VBUS_OTG_DISCHARGE               BIT(4)
#define SWIN_SWOUT_ENABLED               BIT(2)
#define USBSW_OTG_SWITCH_ENABLED         BIT(1)
#define BOOST_ENABLED                    BIT(0)

/* Zephyr-backed API matching TF-A / OP-TEE naming. */
void stpmic1_bind(const struct device *i2c_dev, uint16_t i2c_addr);
int stpmic1_is_ready(void);

int stpmic1_register_read(uint8_t reg, uint8_t *value);
int stpmic1_register_write(uint8_t reg, uint8_t value);
int stpmic1_register_update(uint8_t reg, uint8_t value, uint8_t mask);

int  stpmic1_regulator_enable(const char *name);
int  stpmic1_regulator_disable(const char *name);
bool stpmic1_is_regulator_enabled(const char *name);
int  stpmic1_regulator_voltage_set(const char *name, uint16_t millivolts);
int  stpmic1_regulator_voltage_get(const char *name);
int  stpmic1_regulator_levels_mv(const char *name, const uint16_t **levels,
				 size_t *levels_count);
int  stpmic1_regulator_pull_down_set(const char *name);
int  stpmic1_regulator_mask_reset_set(const char *name);
int  stpmic1_regulator_icc_set(const char *name);
int  stpmic1_regulator_sink_mode_set(const char *name);
int  stpmic1_regulator_bypass_mode_set(const char *name);
int  stpmic1_active_discharge_mode_set(const char *name);

/* Low-power helpers mirrored from OP-TEE */
bool stpmic1_regu_has_lp_cfg(const char *name);
int  stpmic1_lp_copy_reg(const char *name);
int  stpmic1_lp_reg_on_off(const char *name, uint8_t enable);
int  stpmic1_lp_set_mode(const char *name, uint8_t hplp);
int  stpmic1_lp_set_voltage(const char *name, uint16_t millivolts);

int  stpmic1_powerctrl_on(void);
int  stpmic1_switch_off(void);
int  stpmic1_get_version(unsigned long *version);
void stpmic1_dump_regulators(void);

#endif /* STPMIC1_DRIVER_H */