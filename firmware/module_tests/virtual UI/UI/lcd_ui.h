#ifndef LCD_UI_H
#define LCD_UI_H

#include <stdint.h>

typedef struct {
    uint8_t heart_rate;
    uint8_t spo2;
    int16_t temperature_c10;
    int16_t pitch_deg;
    int16_t roll_deg;
    uint8_t fall_detected;
    uint8_t pillbox_slot;
    uint16_t pill_weight_g;
    uint8_t servo_open;
    uint8_t alarm_active;
    uint8_t wifi_connected;
} UI_Data_t;

typedef struct {
    uint8_t magic;
    uint8_t version;
    UI_Data_t data;
    uint8_t checksum;
} UI_CaptureFrame_t;

void UI_Init(void);
void UI_Update(const UI_Data_t *data);
void UI_OnTouch(uint16_t x, uint16_t y, uint8_t pressed);
uint16_t UI_GetCaptureFrame(uint8_t *buffer, uint16_t buffer_size);
void UI_FirmwareRun(void);

#endif
