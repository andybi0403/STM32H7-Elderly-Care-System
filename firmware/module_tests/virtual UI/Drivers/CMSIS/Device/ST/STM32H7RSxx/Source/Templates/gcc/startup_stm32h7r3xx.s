.syntax unified
.cpu cortex-m7
.fpu fpv5-sp-d16
.thumb

.global g_pfnVectors
.global Default_Handler
.global Reset_Handler

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
.word _estack
.word Reset_Handler + 1
.word NMI_Handler + 1
.word HardFault_Handler + 1
.word MemManage_Handler + 1
.word BusFault_Handler + 1
.word UsageFault_Handler + 1
.word 0
.word 0
.word 0
.word 0
.word SVC_Handler + 1
.word DebugMon_Handler + 1
.word 0
.word PendSV_Handler + 1
.word SysTick_Handler + 1
.word WWDG_IRQHandler
.word PVD_AVD_IRQHandler
.word TAMP_STAMP_IRQHandler
.word RTC_WKUP_IRQHandler
.word FLASH_IRQHandler
.word RCC_IRQHandler
.word EXTI0_IRQHandler
.word EXTI1_IRQHandler
.word EXTI2_IRQHandler
.word EXTI3_IRQHandler
.word EXTI4_IRQHandler
.word DMA1_Stream0_IRQHandler
.word DMA1_Stream1_IRQHandler
.word DMA1_Stream2_IRQHandler
.word DMA1_Stream3_IRQHandler
.word DMA1_Stream4_IRQHandler
.word DMA1_Stream5_IRQHandler
.word DMA1_Stream6_IRQHandler
.word ADC1_2_IRQHandler
.word FDCAN1_IT0_IRQHandler
.word FDCAN2_IT0_IRQHandler
.word FDCAN1_IT1_IRQHandler
.word FDCAN2_IT1_IRQHandler
.word 0
.word 0
.word SPI1_IRQHandler
.word SPI2_IRQHandler
.word USART1_IRQHandler
.word USART2_IRQHandler
.word USART3_IRQHandler
.word EXTI5_9_IRQHandler
.word TIM8_BRK_TIM12_IRQHandler
.word TIM8_UP_TIM12_IRQHandler
.word TIM8_TRG_COM_TIM12_IRQHandler
.word TIM8_CC_IRQHandler
.word DMA1_Stream7_IRQHandler
.word FMC_IRQHandler
.word SDMMC1_IRQHandler
.word TIM5_IRQHandler
.word SPI3_IRQHandler
.word UART4_IRQHandler
.word UART5_IRQHandler
.word TIM6_DAC_IRQHandler
.word TIM7_IRQHandler
.word DMA2_Stream0_IRQHandler
.word DMA2_Stream1_IRQHandler
.word DMA2_Stream2_IRQHandler
.word DMA2_Stream3_IRQHandler
.word DMA2_Stream4_IRQHandler
.word ETH_IRQHandler
.word ETH_WKUP_IRQHandler
.word FDCAN_CAL_IRQHandler
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word DMA2_Stream5_IRQHandler
.word DMA2_Stream6_IRQHandler
.word DMA2_Stream7_IRQHandler
.word USART6_IRQHandler
.word I2C3_EV_IRQHandler
.word I2C3_ER_IRQHandler
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word UART7_IRQHandler
.word UART8_IRQHandler
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0

.thumb_func
Reset_Handler:
  ldr sp, =_estack
  ldr r0, =SystemInit
  blx r0
  ldr r0, =main
  bx r0

.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler
.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler
.weak MemManage_Handler
.thumb_set MemManage_Handler, Default_Handler
.weak BusFault_Handler
.thumb_set BusFault_Handler, Default_Handler
.weak UsageFault_Handler
.thumb_set UsageFault_Handler, Default_Handler
.weak SVC_Handler
.thumb_set SVC_Handler, Default_Handler
.weak DebugMon_Handler
.thumb_set DebugMon_Handler, Default_Handler
.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler
.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler
.weak Default_Handler

.weak WWDG_IRQHandler
.thumb_set WWDG_IRQHandler, Default_Handler
.weak PVD_AVD_IRQHandler
.thumb_set PVD_AVD_IRQHandler, Default_Handler
.weak TAMP_STAMP_IRQHandler
.thumb_set TAMP_STAMP_IRQHandler, Default_Handler
.weak RTC_WKUP_IRQHandler
.thumb_set RTC_WKUP_IRQHandler, Default_Handler
.weak FLASH_IRQHandler
.thumb_set FLASH_IRQHandler, Default_Handler
.weak RCC_IRQHandler
.thumb_set RCC_IRQHandler, Default_Handler
.weak EXTI0_IRQHandler
.thumb_set EXTI0_IRQHandler, Default_Handler
.weak EXTI1_IRQHandler
.thumb_set EXTI1_IRQHandler, Default_Handler
.weak EXTI2_IRQHandler
.thumb_set EXTI2_IRQHandler, Default_Handler
.weak EXTI3_IRQHandler
.thumb_set EXTI3_IRQHandler, Default_Handler
.weak EXTI4_IRQHandler
.thumb_set EXTI4_IRQHandler, Default_Handler
.weak DMA1_Stream0_IRQHandler
.thumb_set DMA1_Stream0_IRQHandler, Default_Handler
.weak DMA1_Stream1_IRQHandler
.thumb_set DMA1_Stream1_IRQHandler, Default_Handler
.weak DMA1_Stream2_IRQHandler
.thumb_set DMA1_Stream2_IRQHandler, Default_Handler
.weak DMA1_Stream3_IRQHandler
.thumb_set DMA1_Stream3_IRQHandler, Default_Handler
.weak DMA1_Stream4_IRQHandler
.thumb_set DMA1_Stream4_IRQHandler, Default_Handler
.weak DMA1_Stream5_IRQHandler
.thumb_set DMA1_Stream5_IRQHandler, Default_Handler
.weak DMA1_Stream6_IRQHandler
.thumb_set DMA1_Stream6_IRQHandler, Default_Handler
.weak ADC1_2_IRQHandler
.thumb_set ADC1_2_IRQHandler, Default_Handler
.weak FDCAN1_IT0_IRQHandler
.thumb_set FDCAN1_IT0_IRQHandler, Default_Handler
.weak FDCAN2_IT0_IRQHandler
.thumb_set FDCAN2_IT0_IRQHandler, Default_Handler
.weak FDCAN1_IT1_IRQHandler
.thumb_set FDCAN1_IT1_IRQHandler, Default_Handler
.weak FDCAN2_IT1_IRQHandler
.thumb_set FDCAN2_IT1_IRQHandler, Default_Handler
.weak SPI1_IRQHandler
.thumb_set SPI1_IRQHandler, Default_Handler
.weak SPI2_IRQHandler
.thumb_set SPI2_IRQHandler, Default_Handler
.weak USART1_IRQHandler
.thumb_set USART1_IRQHandler, Default_Handler
.weak USART2_IRQHandler
.thumb_set USART2_IRQHandler, Default_Handler
.weak USART3_IRQHandler
.thumb_set USART3_IRQHandler, Default_Handler
.weak EXTI5_9_IRQHandler
.thumb_set EXTI5_9_IRQHandler, Default_Handler
.weak TIM8_BRK_TIM12_IRQHandler
.thumb_set TIM8_BRK_TIM12_IRQHandler, Default_Handler
.weak TIM8_UP_TIM12_IRQHandler
.thumb_set TIM8_UP_TIM12_IRQHandler, Default_Handler
.weak TIM8_TRG_COM_TIM12_IRQHandler
.thumb_set TIM8_TRG_COM_TIM12_IRQHandler, Default_Handler
.weak TIM8_CC_IRQHandler
.thumb_set TIM8_CC_IRQHandler, Default_Handler
.weak DMA1_Stream7_IRQHandler
.thumb_set DMA1_Stream7_IRQHandler, Default_Handler
.weak FMC_IRQHandler
.thumb_set FMC_IRQHandler, Default_Handler
.weak SDMMC1_IRQHandler
.thumb_set SDMMC1_IRQHandler, Default_Handler
.weak TIM5_IRQHandler
.thumb_set TIM5_IRQHandler, Default_Handler
.weak SPI3_IRQHandler
.thumb_set SPI3_IRQHandler, Default_Handler
.weak UART4_IRQHandler
.thumb_set UART4_IRQHandler, Default_Handler
.weak UART5_IRQHandler
.thumb_set UART5_IRQHandler, Default_Handler
.weak TIM6_DAC_IRQHandler
.thumb_set TIM6_DAC_IRQHandler, Default_Handler
.weak TIM7_IRQHandler
.thumb_set TIM7_IRQHandler, Default_Handler
.weak DMA2_Stream0_IRQHandler
.thumb_set DMA2_Stream0_IRQHandler, Default_Handler
.weak DMA2_Stream1_IRQHandler
.thumb_set DMA2_Stream1_IRQHandler, Default_Handler
.weak DMA2_Stream2_IRQHandler
.thumb_set DMA2_Stream2_IRQHandler, Default_Handler
.weak DMA2_Stream3_IRQHandler
.thumb_set DMA2_Stream3_IRQHandler, Default_Handler
.weak DMA2_Stream4_IRQHandler
.thumb_set DMA2_Stream4_IRQHandler, Default_Handler
.weak ETH_IRQHandler
.thumb_set ETH_IRQHandler, Default_Handler
.weak ETH_WKUP_IRQHandler
.thumb_set ETH_WKUP_IRQHandler, Default_Handler
.weak FDCAN_CAL_IRQHandler
.thumb_set FDCAN_CAL_IRQHandler, Default_Handler
.weak DMA2_Stream5_IRQHandler
.thumb_set DMA2_Stream5_IRQHandler, Default_Handler
.weak DMA2_Stream6_IRQHandler
.thumb_set DMA2_Stream6_IRQHandler, Default_Handler
.weak DMA2_Stream7_IRQHandler
.thumb_set DMA2_Stream7_IRQHandler, Default_Handler
.weak USART6_IRQHandler
.thumb_set USART6_IRQHandler, Default_Handler
.weak I2C3_EV_IRQHandler
.thumb_set I2C3_EV_IRQHandler, Default_Handler
.weak I2C3_ER_IRQHandler
.thumb_set I2C3_ER_IRQHandler, Default_Handler
.weak UART7_IRQHandler
.thumb_set UART7_IRQHandler, Default_Handler
.weak UART8_IRQHandler
.thumb_set UART8_IRQHandler, Default_Handler

Default_Handler:
  b .
