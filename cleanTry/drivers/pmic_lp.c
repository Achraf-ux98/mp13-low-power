#include <errno.h>
#include <zephyr/sys/printk.h>

#include "stpmic1.h"
#include "pmic_lp.h"

struct pmic_lp_regval {
	const char *name;
	uint16_t millivolts;
	uint8_t hplp;
	uint8_t enable;
	uint8_t set_voltage;
	uint8_t set_hplp;
	uint8_t set_enable;
};

/*
 * Only rails that differ from "copy HP to LP" need to be listed here.
 * Everything else is copied from current HP register first.
 *
 * hplp:
 *   0 = high-load mode
 *   1 = low-load / standby mode
 */

/* lp-stop: mostly just copy current HP to LP */
static const struct pmic_lp_regval lp_stop_cfg[] = {
	/* no overrides */
};

/* lplv-stop */
static const struct pmic_lp_regval lplv_stop_cfg[] = {
	/* buck1 -> 0.9V, low-power load mode, still enabled */
	{
		.name = "buck1",
		.millivolts = 900,
		.hplp = 1,
		.enable = 1,
		.set_voltage = 1,
		.set_hplp = 1,
		.set_enable = 1,
	},
	/* buck4 -> 0.9V, low-power load mode, still enabled */
	{
		.name = "buck4",
		.millivolts = 900,
		.hplp = 1,
		.enable = 1,
		.set_voltage = 1,
		.set_hplp = 1,
		.set_enable = 1,
	},
};

/* lplv-stop2 */
static const struct pmic_lp_regval lplv_stop2_cfg[] = {
	/* buck1 off */
	{
		.name = "buck1",
		.enable = 0,
		.set_enable = 1,
	},
	/* buck4 -> 0.9V, low-power load mode, still enabled */
	{
		.name = "buck4",
		.millivolts = 900,
		.hplp = 1,
		.enable = 1,
		.set_voltage = 1,
		.set_hplp = 1,
		.set_enable = 1,
	},
};

/* standby-ddr-sr */
static const struct pmic_lp_regval standby_ddr_sr_cfg[] = {
	/* buck1 off */
	{
		.name = "buck1",
		.enable = 0,
		.set_enable = 1,
	},
	/* buck4 off */
	{
		.name = "buck4",
		.enable = 0,
		.set_enable = 1,
	},
	/* board rails off */
	{
		.name = "ldo1",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "ldo4",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "ldo5",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "ldo6",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "vref_ddr",
		.enable = 0,
		.set_enable = 1,
	},
};

/* standby-ddr-off */
static const struct pmic_lp_regval standby_ddr_off_cfg[] = {
	/* buck1 off */
	{
		.name = "buck1",
		.enable = 0,
		.set_enable = 1,
	},
	/* buck2 off (DDR supply off) */
	{
		.name = "buck2",
		.enable = 0,
		.set_enable = 1,
	},
	/* buck4 off */
	{
		.name = "buck4",
		.enable = 0,
		.set_enable = 1,
	},
	/* board rails off */
	{
		.name = "ldo1",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "ldo4",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "ldo5",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "ldo6",
		.enable = 0,
		.set_enable = 1,
	},
	{
		.name = "vref_ddr",
		.enable = 0,
		.set_enable = 1,
	},
};

struct pmic_lp_profile {
	enum pmic_lp_state state;
	const char *name;
	const struct pmic_lp_regval *cfg;
	size_t count;
};

#define ARRAY_SIZE_LOCAL(x) (sizeof(x) / sizeof((x)[0]))

static const struct pmic_lp_profile profiles[] = {
	{
		.state = PMIC_LP_STOP,
		.name = "lp-stop",
		.cfg = lp_stop_cfg,
		.count = ARRAY_SIZE_LOCAL(lp_stop_cfg),
	},
	{
		.state = PMIC_LPLV_STOP,
		.name = "lplv-stop",
		.cfg = lplv_stop_cfg,
		.count = ARRAY_SIZE_LOCAL(lplv_stop_cfg),
	},
	{
		.state = PMIC_LPLV_STOP2,
		.name = "lplv-stop2",
		.cfg = lplv_stop2_cfg,
		.count = ARRAY_SIZE_LOCAL(lplv_stop2_cfg),
	},
	{
		.state = PMIC_STANDBY_DDR_SR,
		.name = "standby-ddr-sr",
		.cfg = standby_ddr_sr_cfg,
		.count = ARRAY_SIZE_LOCAL(standby_ddr_sr_cfg),
	},
	{
		.state = PMIC_STANDBY_DDR_OFF,
		.name = "standby-ddr-off",
		.cfg = standby_ddr_off_cfg,
		.count = ARRAY_SIZE_LOCAL(standby_ddr_off_cfg),
	},
};

static const char * const lp_regulators_to_copy[] = {
	"buck1",
	"buck2",
	"buck3",
	"buck4",
	"ldo1",
	"ldo2",
	"ldo3",
	"ldo4",
	"ldo5",
	"ldo6",
	"vref_ddr",
};

static const struct pmic_lp_profile *get_profile(enum pmic_lp_state state)
{
	for (size_t i = 0; i < ARRAY_SIZE_LOCAL(profiles); i++) {
		if (profiles[i].state == state) {
			return &profiles[i];
		}
	}

	return NULL;
}

const char *pmic_lp_state_name(enum pmic_lp_state state)
{
	const struct pmic_lp_profile *p = get_profile(state);

	if (!p) {
		return "unknown";
	}

	return p->name;
}

static int pmic_lp_copy_all(void)
{
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE_LOCAL(lp_regulators_to_copy); i++) {
		const char *name = lp_regulators_to_copy[i];

		if (!stpmic1_regu_has_lp_cfg(name)) {
			continue;
		}

		ret = stpmic1_lp_copy_reg(name);
		if (ret) {
			printk("pmic_lp: lp_copy_reg(%s) failed (%d)\n", name, ret);
			return ret;
		}
	}

	return 0;
}

static int pmic_lp_apply_one(const struct pmic_lp_regval *rv)
{
	int ret;

	if (rv->set_voltage) {
		ret = stpmic1_lp_set_voltage(rv->name, rv->millivolts);
		if (ret) {
			printk("pmic_lp: lp_set_voltage(%s, %u) failed (%d)\n",
			       rv->name, rv->millivolts, ret);
			return ret;
		}
	}

	if (rv->set_hplp) {
		ret = stpmic1_lp_set_mode(rv->name, rv->hplp);
		if (ret) {
			printk("pmic_lp: lp_set_mode(%s, %u) failed (%d)\n",
			       rv->name, rv->hplp, ret);
			return ret;
		}
	}

	if (rv->set_enable) {
		ret = stpmic1_lp_reg_on_off(rv->name, rv->enable);
		if (ret) {
			printk("pmic_lp: lp_reg_on_off(%s, %u) failed (%d)\n",
			       rv->name, rv->enable, ret);
			return ret;
		}
	}

	return 0;
}

int pmic_lp_apply(enum pmic_lp_state state)
{
	const struct pmic_lp_profile *p;
	int ret;

	p = get_profile(state);
	if (!p) {
		return -EINVAL;
	}

	ret = stpmic1_powerctrl_on();
	if (ret) {
		printk("pmic_lp: stpmic1_powerctrl_on() failed (%d)\n", ret);
		return ret;
	}

	ret = pmic_lp_copy_all();
	if (ret) {
		return ret;
	}

	for (size_t i = 0; i < p->count; i++) {
		ret = pmic_lp_apply_one(&p->cfg[i]);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

static void dump_one(uint8_t reg)
{
	uint8_t val;
	int ret = stpmic1_register_read(reg, &val);

	if (ret) {
		printk("LP reg 0x%02x read failed (%d)\n", reg, ret);
		return;
	}

	printk("LP reg 0x%02x = 0x%02x\n", reg, val);
}

void pmic_lp_dump_regs(void)
{
	printk("---- STPMIC1 LP regs ----\n");
	dump_one(BUCK1_PWRCTRL_REG);
	dump_one(BUCK2_PWRCTRL_REG);
	dump_one(BUCK3_PWRCTRL_REG);
	dump_one(BUCK4_PWRCTRL_REG);
	dump_one(VREF_DDR_PWRCTRL_REG);
	dump_one(LDO1_PWRCTRL_REG);
	dump_one(LDO2_PWRCTRL_REG);
	dump_one(LDO3_PWRCTRL_REG);
	dump_one(LDO4_PWRCTRL_REG);
	dump_one(LDO5_PWRCTRL_REG);
	dump_one(LDO6_PWRCTRL_REG);
	dump_one(MAIN_CONTROL_REG);
}