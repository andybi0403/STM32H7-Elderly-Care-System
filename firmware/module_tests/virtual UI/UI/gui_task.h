/**
 * gui_task.h — FreeRTOS GUI 任务接口
 */
#ifndef __GUI_TASK_H
#define __GUI_TASK_H

#define GUI_REFRESH_MS  50
#define GUI_TASK_STACK  2048
#define GUI_TASK_PRIO   (tskIDLE_PRIORITY + 3)

void GUI_Task(void *pvParameters);

#endif
