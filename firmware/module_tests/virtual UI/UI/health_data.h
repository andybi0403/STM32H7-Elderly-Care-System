/**
 * health_data.h — 全局健康数据结构体
 * 所有 UI 页面和传感器任务共享此结构体
 */
#ifndef __HEALTH_DATA_H
#define __HEALTH_DATA_H

#include <stdint.h>

#define MED_SLOTS 4

typedef struct {
    char  name[12];      /* 药品名   */
    int16_t remaining;   /* 剩余片数  */
    int16_t threshold;   /* 低量阈值  */
} MedSlot;

typedef struct {
    /* 生命体征 */
    int16_t heart_rate;
    int16_t blood_oxygen;
    float   temperature;
    uint16_t steps;
    uint8_t fall_detected;
    uint8_t heart_abnormal;
    /* 药盒 */
    MedSlot med[MED_SLOTS];
    uint8_t med_slot_idx;
    uint8_t med_alarm;
    /* 连接状态 */
    uint8_t wifi_on;
    uint8_t bt_on;
} HealthData;

extern HealthData g_health_data;

void HealthData_Init(void);

#endif
