/*
 * STPMIC1 wrapper implementation.
 * Logic mirrors TF-A / OP-TEE stpmic1.c with Zephyr I2C backend.
 */

#define DT_DRV_COMPAT st_stpmic1

#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "stpmic1.h"

/* Internal state */
static const struct device *pmic_i2c_dev;
static uint16_t pmic_i2c_addr;
static bool ldo3_special_mode;

struct stpmic1_config {
	struct i2c_dt_spec i2c;
};

/* Voltage tables in mV (copied from TF-A / OP-TEE stpmic1.c) */
static const uint16_t buck1_voltage_table[] = {
	725, 725, 725, 725, 725, 725,
	750, 775, 800, 825, 850, 875, 900, 925, 950, 975,
	1000, 1025, 1050, 1075, 1100, 1125, 1150, 1175,
	1200, 1225, 1250, 1275, 1300, 1325, 1350, 1375,
	1400, 1425, 1450, 1475, 1500,
	1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
	1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
	1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
};

static const uint16_t buck2_voltage_table[] = {
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	1000, 1000,
	1050, 1050, 1100, 1100, 1150, 1150,
	1200, 1200, 1250, 1250, 1300, 1300,
	1350, 1350, 1400, 1400, 1450, 1450, 1500,
};

static const uint16_t buck3_voltage_table[] = {
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	1000, 1000, 1000, 1000,
	1100, 1100, 1100, 1100, 1200, 1200, 1200, 1200,
	1300, 1300, 1300, 1300, 1400, 1400, 1400, 1400,
	1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200,
	2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000,
	3100, 3200, 3300, 3400,
};

static const uint16_t buck4_voltage_table[] = {
	600, 625, 650, 675, 700, 725, 750, 775,
	800, 825, 850, 875, 900, 925, 950, 975,
	1000, 1025, 1050, 1075, 1100, 1125, 1150, 1175,
	1200, 1225, 1250, 1275, 1300, 1300, 1350, 1350,
	1400, 1400, 1450, 1450, 1500,
	1600, 1700, 1800, 1900, 2000, 2100, 2200,
	2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000,
	3100, 3200, 3300, 3400, 3500, 3600, 3700, 3800, 3900,
};

static const uint16_t ldo1_voltage_table[] = {
	1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700,
	1800, 1900, 2000, 2100, 2200, 2300, 2400,
	2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200, 3300,
};

static const uint16_t ldo2_voltage_table[] = {
	1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700,
	1800, 1900, 2000, 2100, 2200, 2300, 2400,
	2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200, 3300,
};

static const uint16_t ldo3_voltage_table[] = {
	1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700,
	1800, 1900, 2000, 2100, 2200, 2300, 2400,
	2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200,
	3300, 3300, 3300, 3300, 3300, 3300, 3300,
};

static const uint16_t ldo3_special_mode_table[] = { 0 };
static const uint16_t ldo4_voltage_table[] = { 3300 };

static const uint16_t ldo5_voltage_table[] = {
	1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700,
	1800, 1900, 2000, 2100, 2200, 2300, 2400,
	2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200,
	3300, 3400, 3500, 3600, 3700, 3800, 3900,
};

static const uint16_t ldo6_voltage_table[] = {
	900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700,
	1800, 1900, 2000, 2100, 2200, 2300, 2400,
	2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200, 3300,
};

static const uint16_t vref_ddr_voltage_table[] = { 3300 };
static const uint16_t fixed_5v_voltage_table[] = { 5000 };

/* Regulator descriptor */
struct regul_desc {
	const char *name;
	const uint16_t *vtable;
	uint8_t vtable_size;
	uint8_t ctrl_reg;
	uint8_t enable_mask;
	uint8_t lp_reg;
	uint8_t pd_reg;
	uint8_t pd_shift;
	uint8_t mask_reset_reg;
	uint8_t mask_reset;
	uint8_t icc_reg;
	uint8_t icc_mask;
};

#define ARRAY_SIZE_LOCAL(x) ((uint8_t)(sizeof(x) / sizeof((x)[0])))

static const struct regul_desc regulators[] = {
	{ "buck1", buck1_voltage_table, ARRAY_SIZE_LOCAL(buck1_voltage_table),
	  BUCK1_CONTROL_REG, LDO_BUCK_ENABLE_MASK, BUCK1_PWRCTRL_REG,
	  BUCK_PULL_DOWN_REG, BUCK1_PULL_DOWN_SHIFT,
	  MASK_RESET_BUCK_REG, BUCK1_MASK_RESET,
	  BUCK_ICC_TURNOFF_REG, BUCK1_ICC_SHIFT },

	{ "buck2", buck2_voltage_table, ARRAY_SIZE_LOCAL(buck2_voltage_table),
	  BUCK2_CONTROL_REG, LDO_BUCK_ENABLE_MASK, BUCK2_PWRCTRL_REG,
	  BUCK_PULL_DOWN_REG, BUCK2_PULL_DOWN_SHIFT,
	  MASK_RESET_BUCK_REG, BUCK2_MASK_RESET,
	  BUCK_ICC_TURNOFF_REG, BUCK2_ICC_SHIFT },

	{ "buck3", buck3_voltage_table, ARRAY_SIZE_LOCAL(buck3_voltage_table),
	  BUCK3_CONTROL_REG, LDO_BUCK_ENABLE_MASK, BUCK3_PWRCTRL_REG,
	  BUCK_PULL_DOWN_REG, BUCK3_PULL_DOWN_SHIFT,
	  MASK_RESET_BUCK_REG, BUCK3_MASK_RESET,
	  BUCK_ICC_TURNOFF_REG, BUCK3_ICC_SHIFT },

	{ "buck4", buck4_voltage_table, ARRAY_SIZE_LOCAL(buck4_voltage_table),
	  BUCK4_CONTROL_REG, LDO_BUCK_ENABLE_MASK, BUCK4_PWRCTRL_REG,
	  BUCK_PULL_DOWN_REG, BUCK4_PULL_DOWN_SHIFT,
	  MASK_RESET_BUCK_REG, BUCK4_MASK_RESET,
	  BUCK_ICC_TURNOFF_REG, BUCK4_ICC_SHIFT },

	{ "ldo1", ldo1_voltage_table, ARRAY_SIZE_LOCAL(ldo1_voltage_table),
	  LDO1_CONTROL_REG, LDO_BUCK_ENABLE_MASK, LDO1_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, LDO1_MASK_RESET,
	  LDO_ICC_TURNOFF_REG, LDO1_ICC_SHIFT },

	{ "ldo2", ldo2_voltage_table, ARRAY_SIZE_LOCAL(ldo2_voltage_table),
	  LDO2_CONTROL_REG, LDO_BUCK_ENABLE_MASK, LDO2_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, LDO2_MASK_RESET,
	  LDO_ICC_TURNOFF_REG, LDO2_ICC_SHIFT },

	{ "ldo3", ldo3_voltage_table, ARRAY_SIZE_LOCAL(ldo3_voltage_table),
	  LDO3_CONTROL_REG, LDO_BUCK_ENABLE_MASK, LDO3_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, LDO3_MASK_RESET,
	  LDO_ICC_TURNOFF_REG, LDO3_ICC_SHIFT },

	{ "ldo4", ldo4_voltage_table, ARRAY_SIZE_LOCAL(ldo4_voltage_table),
	  LDO4_CONTROL_REG, LDO_BUCK_ENABLE_MASK, LDO4_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, LDO4_MASK_RESET,
	  LDO_ICC_TURNOFF_REG, LDO4_ICC_SHIFT },

	{ "ldo5", ldo5_voltage_table, ARRAY_SIZE_LOCAL(ldo5_voltage_table),
	  LDO5_CONTROL_REG, LDO_BUCK_ENABLE_MASK, LDO5_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, LDO5_MASK_RESET,
	  LDO_ICC_TURNOFF_REG, LDO5_ICC_SHIFT },

	{ "ldo6", ldo6_voltage_table, ARRAY_SIZE_LOCAL(ldo6_voltage_table),
	  LDO6_CONTROL_REG, LDO_BUCK_ENABLE_MASK, LDO6_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, LDO6_MASK_RESET,
	  LDO_ICC_TURNOFF_REG, LDO6_ICC_SHIFT },

	{ "vref_ddr", vref_ddr_voltage_table, ARRAY_SIZE_LOCAL(vref_ddr_voltage_table),
	  VREF_DDR_CONTROL_REG, LDO_BUCK_ENABLE_MASK, VREF_DDR_PWRCTRL_REG,
	  0, 0, MASK_RESET_LDO_REG, VREF_DDR_MASK_RESET,
	  0, 0 },

	{ "boost", fixed_5v_voltage_table, ARRAY_SIZE_LOCAL(fixed_5v_voltage_table),
	  USB_CONTROL_REG, BOOST_ENABLED, 0,
	  0, 0, 0, 0,
	  BUCK_ICC_TURNOFF_REG, BOOST_ICC_SHIFT },

	{ "pwr_sw1", fixed_5v_voltage_table, ARRAY_SIZE_LOCAL(fixed_5v_voltage_table),
	  USB_CONTROL_REG, USBSW_OTG_SWITCH_ENABLED, 0,
	  0, 0, 0, 0,
	  BUCK_ICC_TURNOFF_REG, PWR_SW1_ICC_SHIFT },

	{ "pwr_sw2", fixed_5v_voltage_table, ARRAY_SIZE_LOCAL(fixed_5v_voltage_table),
	  USB_CONTROL_REG, SWIN_SWOUT_ENABLED, 0,
	  0, 0, 0, 0,
	  BUCK_ICC_TURNOFF_REG, PWR_SW2_ICC_SHIFT },
};

#define NUM_REGULATORS ((int)(sizeof(regulators) / sizeof(regulators[0])))

/* Helpers */
static const struct regul_desc *get_regul(const char *name)
{
	for (int i = 0; i < NUM_REGULATORS; i++) {
		if (strcmp(name, regulators[i].name) == 0) {
			return &regulators[i];
		}
	}

	return NULL;
}

static int voltage_to_index(const struct regul_desc *r, uint16_t mv)
{
	for (int i = 0; i < r->vtable_size; i++) {
		if (r->vtable[i] == mv) {
			return i;
		}
	}

	return -EINVAL;
}

/* Voltage can be set for buck<N> or ldo<N> (except ldo4) regulators */
static uint8_t find_plat_mask(const char *name)
{
	if (strncmp(name, "buck", 4) == 0) {
		return BUCK_VOLTAGE_MASK;
	}

	if ((strncmp(name, "ldo", 3) == 0) && (strcmp(name, "ldo4") != 0)) {
		return LDO_VOLTAGE_MASK;
	}

	return 0U;
}

/* Init */
void stpmic1_bind(const struct device *i2c_dev, uint16_t i2c_addr)
{
	pmic_i2c_dev = i2c_dev;
	pmic_i2c_addr = i2c_addr;
}

int stpmic1_is_ready(void)
{
	return (pmic_i2c_dev != NULL) && device_is_ready(pmic_i2c_dev);
}

/* Register access */
int stpmic1_register_read(uint8_t reg, uint8_t *value)
{
	if (!stpmic1_is_ready()) {
		return -ENODEV;
	}

	return i2c_reg_read_byte(pmic_i2c_dev, pmic_i2c_addr, reg, value);
}

int stpmic1_register_write(uint8_t reg, uint8_t value)
{
	if (!stpmic1_is_ready()) {
		return -ENODEV;
	}

	return i2c_reg_write_byte(pmic_i2c_dev, pmic_i2c_addr, reg, value);
}

int stpmic1_register_update(uint8_t reg, uint8_t value, uint8_t mask)
{
	uint8_t val;
	int ret;

	ret = stpmic1_register_read(reg, &val);
	if (ret != 0) {
		return ret;
	}

	val = (val & ~mask) | (value & mask);

	return stpmic1_register_write(reg, val);
}

/* Regulator API */
int stpmic1_regulator_enable(const char *name)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return -ENODEV;
	}

	return stpmic1_register_update(r->ctrl_reg, r->enable_mask,
				       r->enable_mask);
}

int stpmic1_regulator_disable(const char *name)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return -ENODEV;
	}

	return stpmic1_register_update(r->ctrl_reg, 0, r->enable_mask);
}

bool stpmic1_is_regulator_enabled(const char *name)
{
	const struct regul_desc *r = get_regul(name);
	uint8_t val;

	if (!r || stpmic1_register_read(r->ctrl_reg, &val) != 0) {
		return false;
	}

	return (val & r->enable_mask) == r->enable_mask;
}

int stpmic1_regulator_voltage_set(const char *name, uint16_t millivolts)
{
	const struct regul_desc *r = get_regul(name);
	uint8_t mask;
	int idx;

	if (!r) {
		return -ENODEV;
	}

	if ((strcmp(name, "ldo3") == 0) && ldo3_special_mode) {
		return 0;
	}

	mask = find_plat_mask(name);
	if (!mask) {
		return 0;
	}

	idx = voltage_to_index(r, millivolts);
	if (idx < 0) {
		return -EINVAL;
	}

	return stpmic1_register_update(r->ctrl_reg,
				       (uint8_t)(idx << LDO_BUCK_VOLTAGE_SHIFT),
				       mask);
}

int stpmic1_regulator_voltage_get(const char *name)
{
	const struct regul_desc *r = get_regul(name);
	uint8_t val;
	uint8_t mask;
	int ret;

	if (!r) {
		return -ENODEV;
	}

	if ((strcmp(name, "ldo3") == 0) && ldo3_special_mode) {
		return 0;
	}

	mask = find_plat_mask(name);
	if (!mask) {
		return 0;
	}

	ret = stpmic1_register_read(r->ctrl_reg, &val);
	if (ret < 0) {
		return ret;
	}

	val = (val & mask) >> LDO_BUCK_VOLTAGE_SHIFT;

	if (val >= r->vtable_size) {
		return -ERANGE;
	}

	return (int)r->vtable[val];
}

int stpmic1_regulator_levels_mv(const char *name, const uint16_t **levels,
				size_t *levels_count)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return -ENODEV;
	}

	if ((strcmp(name, "ldo3") == 0) && ldo3_special_mode) {
		*levels_count = 1;
		*levels = ldo3_special_mode_table;
	} else {
		*levels_count = r->vtable_size;
		*levels = r->vtable;
	}

	return 0;
}

int stpmic1_regulator_pull_down_set(const char *name)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return -ENODEV;
	}

	if (r->pd_reg != 0U) {
		return stpmic1_register_update(r->pd_reg,
					       BIT(r->pd_shift),
					       LDO_BUCK_PULL_DOWN_MASK << r->pd_shift);
	}

	return 0;
}

int stpmic1_regulator_mask_reset_set(const char *name)
{
	const struct regul_desc *r = get_regul(name);

	if (!r || r->mask_reset_reg == 0U) {
		return -EPERM;
	}

	return stpmic1_register_update(r->mask_reset_reg,
				       BIT(r->mask_reset),
				       LDO_BUCK_RESET_MASK << r->mask_reset);
}

int stpmic1_regulator_icc_set(const char *name)
{
	const struct regul_desc *r = get_regul(name);

	if (!r || r->icc_reg == 0U) {
		return -EPERM;
	}

	return stpmic1_register_update(r->icc_reg,
				       BIT(r->icc_mask),
				       BIT(r->icc_mask));
}

int stpmic1_regulator_sink_mode_set(const char *name)
{
	if (strcmp(name, "ldo3") != 0) {
		return -EPERM;
	}

	ldo3_special_mode = true;

	return stpmic1_register_update(LDO3_CONTROL_REG,
				       LDO3_DDR_SEL << LDO_BUCK_VOLTAGE_SHIFT,
				       LDO3_BYPASS | LDO_VOLTAGE_MASK);
}

int stpmic1_regulator_bypass_mode_set(const char *name)
{
	if (strcmp(name, "ldo3") != 0) {
		return -EPERM;
	}

	ldo3_special_mode = true;

	return stpmic1_register_update(LDO3_CONTROL_REG,
				       LDO3_BYPASS,
				       LDO3_BYPASS | LDO_VOLTAGE_MASK);
}

int stpmic1_active_discharge_mode_set(const char *name)
{
	if (strcmp(name, "pwr_sw1") == 0) {
		return stpmic1_register_update(USB_CONTROL_REG,
					       VBUS_OTG_DISCHARGE,
					       VBUS_OTG_DISCHARGE);
	}

	if (strcmp(name, "pwr_sw2") == 0) {
		return stpmic1_register_update(USB_CONTROL_REG,
					       SW_OUT_DISCHARGE,
					       SW_OUT_DISCHARGE);
	}

	return -EPERM;
}

/* Low-power helpers */
bool stpmic1_regu_has_lp_cfg(const char *name)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return false;
	}

	return r->lp_reg != 0U;
}

int stpmic1_lp_copy_reg(const char *name)
{
	const struct regul_desc *r = get_regul(name);
	uint8_t val;
	int ret;

	if (!r) {
		return -ENODEV;
	}

	if (r->lp_reg == 0U) {
		return -EPERM;
	}

	ret = stpmic1_register_read(r->ctrl_reg, &val);
	if (ret) {
		return ret;
	}

	return stpmic1_register_write(r->lp_reg, val);
}

int stpmic1_lp_reg_on_off(const char *name, uint8_t enable)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return -ENODEV;
	}

	if (r->lp_reg == 0U) {
		return -EPERM;
	}

	if (enable > 1U) {
		return -EINVAL;
	}

	return stpmic1_register_update(r->lp_reg, enable, LDO_BUCK_ENABLE_MASK);
}

int stpmic1_lp_set_mode(const char *name, uint8_t hplp)
{
	const struct regul_desc *r = get_regul(name);

	if (!r) {
		return -ENODEV;
	}

	if (r->lp_reg == 0U) {
		return -EPERM;
	}

	if (hplp > 1U) {
		return -EINVAL;
	}

	return stpmic1_register_update(r->lp_reg,
				       hplp << 1,
				       LDO_BUCK_HPLP_MASK);
}

int stpmic1_lp_set_voltage(const char *name, uint16_t millivolts)
{
	const struct regul_desc *r = get_regul(name);
	uint8_t mask;
	int idx;

	if (!r) {
		return -ENODEV;
	}

	if (r->lp_reg == 0U) {
		return -EPERM;
	}

	mask = find_plat_mask(name);
	if (!mask) {
		return 0;
	}

	idx = voltage_to_index(r, millivolts);
	if (idx < 0) {
		return -EINVAL;
	}

	return stpmic1_register_update(r->lp_reg,
				       (uint8_t)(idx << LDO_BUCK_VOLTAGE_SHIFT),
				       mask);
}

/* Misc */
int stpmic1_powerctrl_on(void)
{
	return stpmic1_register_update(MAIN_CONTROL_REG,
				       PWRCTRL_PIN_VALID,
				       PWRCTRL_PIN_VALID);
}

int stpmic1_switch_off(void)
{
	return stpmic1_register_update(MAIN_CONTROL_REG,
				       1,
				       SOFTWARE_SWITCH_OFF_ENABLED);
}

int stpmic1_get_version(unsigned long *version)
{
	uint8_t val;
	int ret;

	ret = stpmic1_register_read(VERSION_STATUS_REG, &val);
	if (ret < 0) {
		return ret;
	}

	*version = (unsigned long)val;
	return 0;
}

void stpmic1_dump_regulators(void)
{
	for (int i = 0; i < NUM_REGULATORS; i++) {
		const char *n = regulators[i].name;

		printk("PMIC regul %s: %sable, %dmV\n",
		       n,
		       stpmic1_is_regulator_enabled(n) ? "en" : "dis",
		       stpmic1_regulator_voltage_get(n));
	}
}

static int stpmic1_init(const struct device *dev)
{
	const struct stpmic1_config *cfg = dev->config;

	if (!i2c_is_ready_dt(&cfg->i2c)) {
		return -ENODEV;
	}

	stpmic1_bind(cfg->i2c.bus, cfg->i2c.addr);

	return 0;
}

#define STPMIC1_DEFINE(inst)                                                      \
	static const struct stpmic1_config stpmic1_cfg_##inst = {                 \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                \
	};                                                                       \
	DEVICE_DT_INST_DEFINE(inst, stpmic1_init, NULL, NULL, &stpmic1_cfg_##inst, \
			      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL)

DT_INST_FOREACH_STATUS_OKAY(STPMIC1_DEFINE);