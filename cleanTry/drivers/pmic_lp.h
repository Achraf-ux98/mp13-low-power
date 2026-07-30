#ifndef PMIC_LP_H
#define PMIC_LP_H

#include <stdint.h>

enum pmic_lp_state {
	PMIC_LP_STOP = 0,
	PMIC_LPLV_STOP,
	PMIC_LPLV_STOP2,
	PMIC_STANDBY_DDR_SR,
	PMIC_STANDBY_DDR_OFF,
};

int pmic_lp_apply(enum pmic_lp_state state);
const char *pmic_lp_state_name(enum pmic_lp_state state);
void pmic_lp_dump_regs(void);

#endif /* PMIC_LP_H */