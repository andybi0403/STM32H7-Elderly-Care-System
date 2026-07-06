#include "stm32h7rsxx.h"
#include "main.h"
#include "lcd_ui.h"

int main(void) {
    SystemInit();

    UI_FirmwareRun();

    return 0;
}
