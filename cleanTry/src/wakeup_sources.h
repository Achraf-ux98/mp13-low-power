#pragma once

#include <stdbool.h>

int wakeup_sources_init(void);
int wakeup_sources_arm_rtc_alarm(unsigned int seconds);
void wakeup_sources_clear_rtc_alarm(void);
void wakeup_sources_mask_known_irqs(void);
void wakeup_sources_unmask_known_irqs(void);
int wakeup_sources_button_set_enabled(bool enabled);
bool wakeup_sources_button_fired(void);
bool wakeup_sources_rtc_fired(void);
bool wakeup_sources_rtc_pending(void);