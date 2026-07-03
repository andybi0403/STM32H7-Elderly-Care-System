/**
 * health_data.c — 全局健康数据实例 & 初始化
 */
#include "health_data.h"
#include <string.h>

HealthData g_health_data;

void HealthData_Init(void)
{
    memset(&g_health_data, 0, sizeof(HealthData));

    g_health_data.heart_rate   = 72;
    g_health_data.blood_oxygen = 98;
    g_health_data.temperature  = 36.5f;
    g_health_data.wifi_on      = 1;
    g_health_data.bt_on        = 1;

    const char *names[MED_SLOTS] = {"阿司匹林","降压药","降糖药","钙片"};
    for (int i = 0; i < MED_SLOTS; i++) {
        strncpy(g_health_data.med[i].name, names[i], 11);
        g_health_data.med[i].remaining = 80;
        g_health_data.med[i].threshold = 10;
    }
}
