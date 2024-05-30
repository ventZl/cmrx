#define BREAK_ISR(NAME) __attribute__((naked,weak)) void NAME() { asm volatile("BKPT #0\n\t"); }

#define EMPTY_ISR(NAME) __attribute__((weak, alias("__unhandled_user_irq"))) void NAME();

// These interrupt handlers will cause breakpoint to be hit
BREAK_ISR(isr_invalid)
BREAK_ISR(NMI_Handler)
BREAK_ISR(HardFault_Handler)
BREAK_ISR(SysTick_Handler)

// These handlers are entirely empty, hitting them will cause undefined behavior
EMPTY_ISR(TIMER_IRQ_0_Handler)
EMPTY_ISR(TIMER_IRQ_1_Handler)
EMPTY_ISR(TIMER_IRQ_2_Handler)
EMPTY_ISR(TIMER_IRQ_3_Handler)
EMPTY_ISR(PWM_IRQ_WRAP_Handler)
EMPTY_ISR(USBCTRL_IRQ_Handler)
EMPTY_ISR(XIP_IRQ_Handler)
EMPTY_ISR(PIO0_IRQ_0_Handler)
EMPTY_ISR(PIO0_IRQ_1_Handler)
EMPTY_ISR(PIO1_IRQ_0_Handler)
EMPTY_ISR(PIO1_IRQ_1_Handler)
EMPTY_ISR(DMA_IRQ_0_Handler)
EMPTY_ISR(DMA_IRQ_1_Handler)
EMPTY_ISR(IO_IRQ_BANK0_Handler)
EMPTY_ISR(IO_IRQ_QSPI_Handler)
EMPTY_ISR(SIO_IRQ_PROC0_Handler)
EMPTY_ISR(SIO_IRQ_PROC1_Handler)
EMPTY_ISR(CLOCKS_IRQ_Handler)
EMPTY_ISR(SPI0_IRQ_Handler)
EMPTY_ISR(SPI1_IRQ_Handler)
EMPTY_ISR(UART0_IRQ_Handler)
EMPTY_ISR(UART1_IRQ_Handler)
EMPTY_ISR(ADC_IRQ_FIFO_Handler)
EMPTY_ISR(I2C0_IRQ_Handler)
EMPTY_ISR(I2C1_IRQ_Handler)
EMPTY_ISR(RTC_IRQ_Handler)
EMPTY_ISR(isr_irq26)
EMPTY_ISR(isr_irq27)
EMPTY_ISR(isr_irq28)
EMPTY_ISR(isr_irq29)
EMPTY_ISR(isr_irq30)
EMPTY_ISR(isr_irq31)

void __attribute__((naked)) __unhandled_user_irq()
{
    asm volatile(
			".syntax unified\n\t"
            "MRS R0, IPSR\n\t"
            "SUBS R0, #16\n\t"
            "BKPT #0"
    );
}
