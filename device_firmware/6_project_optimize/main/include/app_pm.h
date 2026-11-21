#ifndef APP_PM_H
#define APP_PM_H

#include "esp_err.h"

esp_err_t app_pm_init(void);
esp_err_t app_pm_lock_acquire(void);
esp_err_t app_pm_lock_release(void);
void app_pm_log_status(void);

#endif