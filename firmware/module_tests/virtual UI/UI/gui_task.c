/**
 * gui_task.c — FreeRTOS GUI_Task (emWin 版本)
 *
 * 每 50ms 执行一次:
 *   1. GUI_Delay(50) — emWin 消息处理 + 触发 WM_PAINT
 *   2. 读取 g_health_data — 更新主页 TEXT Widget
 *
 * 离线测试:
 *   定义 ENABLE_SIMULATION → 每秒随机更新心率/血氧
 *   真机编译时注释掉此宏
 */
#include "gui_task.h"
#include "health_data.h"
#include "GUI.h"
#include "TEXT.h"
#include "WM.h"

/* ★ 真机编译时注释掉此行 */
#define ENABLE_SIMULATION

#ifdef ENABLE_SIMULATION
#include <stdlib.h>
#endif

/*---------------------------------------------------------------------------*/
/* 主页 Widget ID (需与 GUI_CreateHomePage 中创建时一致)                      */
/*---------------------------------------------------------------------------*/
#define ID_HOME_TIME        10
#define ID_HOME_HR_VALUE    12
#define ID_HOME_SPO2_VALUE  13
#define ID_HOME_TEMP_VALUE  14
#define ID_HOME_STEPS_VALUE 17
#define ID_HOME_ALARM_TEXT  18
#define ID_HOME_WIFI_ICON   15
#define ID_HOME_BT_ICON     16

extern WM_HWIN hPageHome;  /* 主页窗口句柄 (gui_pages.c 中定义) */

/*---------------------------------------------------------------------------*/
/* 辅助: 更新指定 TEXT Widget 的文本                                          */
/*---------------------------------------------------------------------------*/
static void UpdateText(int widget_id, WM_HWIN hParent, const char *text)
{
    WM_HWIN hItem = WM_GetDialogItem(hParent, widget_id);
    if (hItem) TEXT_SetText(hItem, text);
}

/*---------------------------------------------------------------------------*/
/* 虚拟传感器模拟 (仅 PC 离线测试)                                            */
/*---------------------------------------------------------------------------*/
#ifdef ENABLE_SIMULATION
static void Simulate_HealthData(void)
{
    static uint32_t last = 0;
    uint32_t now = HAL_GetTick();
    if (now - last < 1000) return;
    last = now;

    g_health_data.heart_rate   = 70 + (rand() % 11) - 5;
    g_health_data.blood_oxygen = 97 + (rand() % 5)  - 2;
    g_health_data.temperature  = 36.5f + ((rand() % 7) - 3) * 0.1f;
    g_health_data.steps       += rand() % 4;

    /* 整点触发服药提醒 */
    /* (真机: 由 RTC 闹钟任务设置 g_health_data.med_alarm) */
}
#endif

/*---------------------------------------------------------------------------*/
/* GUI_Task — FreeRTOS 任务入口                                              */
/*---------------------------------------------------------------------------*/
void GUI_Task(void *pvParameters)
{
    (void)pvParameters;

    GUI_Init();
    WM_SetDesktopColor(0x001A1A2E);

    /* 创建三个页面 (由 gui_pages.c 提供) */
    extern void GUI_CreateHomePage(void);
    extern void GUI_CreateHealthPage(void);
    extern void GUI_CreateMedicationPage(void);
    GUI_CreateHomePage();
    GUI_CreateHealthPage();
    GUI_CreateMedicationPage();

    while (1)
    {
#ifdef ENABLE_SIMULATION
        Simulate_HealthData();
#endif
        /* ★ 数据绑定: 读取 g_health_data → 更新屏幕 */
        {
            char buf[32];

            snprintf(buf, sizeof(buf), "%d", g_health_data.heart_rate);
            UpdateText(ID_HOME_HR_VALUE, hPageHome, buf);

            snprintf(buf, sizeof(buf), "%d%%", g_health_data.blood_oxygen);
            UpdateText(ID_HOME_SPO2_VALUE, hPageHome, buf);

            snprintf(buf, sizeof(buf), "%.1f C", g_health_data.temperature);
            UpdateText(ID_HOME_TEMP_VALUE, hPageHome, buf);

            snprintf(buf, sizeof(buf), "%d", g_health_data.steps);
            UpdateText(ID_HOME_STEPS_VALUE, hPageHome, buf);
        }

        /* emWin 消息处理 + 50ms 延时释放 CPU */
        GUI_Delay(GUI_REFRESH_MS);
    }
}
