/**
 * Firefighter Project - Bare Metal STM32F4xx
 *
 * Peripherals used:
 *   ADC1  CH0 (PA0) -> MQ7 gas sensor
 *   ADC1  CH1 (PA1) -> LM35 temperature sensor
 *   USART1 (PA9/PA10) -> GPS module  (115200 8N1)
 *   USART2 (PA2/PA3)  -> Debug printf (115200 8N1)
 *   PA5               -> Buzzer (active HIGH)
 *   PA6               -> Emergency Button (EXTI9_5, falling edge)
 *   PA7               -> Alarm LED (active HIGH, indicates alarm sent)
 *
 * Assumptions:
 *   - STM32F4xx family (e.g. F401/F411/F446), 84 MHz core clock
 *   - HSI 16 MHz -> PLL -> 84 MHz SYSCLK  (adjust PLL constants if needed)
 *   - 3.3 V VDDA/Vref
 *   - GPS sends standard NMEA $GPGGA sentences
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/* ============================================================
 *  Register-base addresses
 * ============================================================ */
#define PERIPH_BASE     0x40000000UL
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)
#define APB1PERIPH_BASE (PERIPH_BASE + 0x00000000UL)
#define APB2PERIPH_BASE (PERIPH_BASE + 0x00010000UL)

/* RCC */
#define RCC_BASE        (AHB1PERIPH_BASE + 0x3800UL)

/* GPIO */
#define GPIOA_BASE      (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOC_BASE      (AHB1PERIPH_BASE + 0x0800UL)

/* EXTI */
#define EXTI_BASE       0x40013C00UL

/* SYSCFG */
#define SYSCFG_BASE     0x40013800UL

/* ADC */
#define ADC1_BASE       (APB2PERIPH_BASE + 0x2000UL)
#define ADC_COMMON_BASE (APB2PERIPH_BASE + 0x2300UL)

/* USART */
#define USART1_BASE     (APB2PERIPH_BASE + 0x1000UL)
#define USART2_BASE     (APB1PERIPH_BASE + 0x4400UL)

/* Flash */
#define FLASH_BASE      0x40023C00UL

/* SysTick */
#define SYSTICK_BASE    0xE000E010UL

/* NVIC */
#define NVIC_BASE       0xE000E100UL

/* ============================================================
 *  Register structs
 * ============================================================ */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    uint32_t RESERVED0[2];
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    uint32_t RESERVED2[2];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t MEMRMP;
    volatile uint32_t PMC;
    volatile uint32_t EXTICR[4];
    uint32_t RESERVED0[2];
    volatile uint32_t CMPCR;
} SYSCFG_TypeDef;

typedef struct {
    volatile uint32_t IMR;
    volatile uint32_t EMR;
    volatile uint32_t RTSR;
    volatile uint32_t FTSR;
    volatile uint32_t SWIER;
    volatile uint32_t PR;
} EXTI_TypeDef;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR[4];
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR[4];
    volatile uint32_t DR;
} ADC_TypeDef;

typedef struct {
    volatile uint32_t CSR;
    volatile uint32_t CCR;
    volatile uint32_t CDR;
} ADC_Common_TypeDef;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t OPTCR;
} FLASH_TypeDef;

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

typedef struct {
    volatile uint32_t ISER[8];
    uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];
    uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];
    uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];
    uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];
    uint32_t RESERVED4[56];
    volatile uint32_t IPR[43];
} NVIC_TypeDef;

/* ============================================================
 *  Peripheral pointers
 * ============================================================ */
#define RCC        ((RCC_TypeDef *)   RCC_BASE)
#define GPIOA      ((GPIO_TypeDef *)  GPIOA_BASE)
#define GPIOC      ((GPIO_TypeDef *)  GPIOC_BASE)
#define SYSCFG     ((SYSCFG_TypeDef *)SYSCFG_BASE)
#define EXTI       ((EXTI_TypeDef *)  EXTI_BASE)
#define ADC1       ((ADC_TypeDef *)   ADC1_BASE)
#define ADC_COMMON ((ADC_Common_TypeDef *) ADC_COMMON_BASE)
#define USART1     ((USART_TypeDef *) USART1_BASE)
#define USART2     ((USART_TypeDef *) USART2_BASE)
#define FLASH      ((FLASH_TypeDef *) FLASH_BASE)
#define SYSTICK    ((SysTick_TypeDef *)SYSTICK_BASE)
#define NVIC       ((NVIC_TypeDef *)  NVIC_BASE)

/* ============================================================
 *  Bit-field constants
 * ============================================================ */
/* RCC AHB1ENR */
#define RCC_AHB1ENR_GPIOAEN     (1U << 0)
#define RCC_AHB1ENR_GPIOCEN     (1U << 2)

/* RCC APB2ENR */
#define RCC_APB2ENR_ADC1EN      (1U << 8)
#define RCC_APB2ENR_USART1EN    (1U << 4)
#define RCC_APB2ENR_SYSCFGEN    (1U << 14)

/* RCC APB1ENR */
#define RCC_APB1ENR_USART2EN    (1U << 17)

/* RCC CR */
#define RCC_CR_HSION            (1U << 0)
#define RCC_CR_HSIRDY           (1U << 1)
#define RCC_CR_PLLON            (1U << 24)
#define RCC_CR_PLLRDY           (1U << 25)

/* RCC CFGR */
#define RCC_CFGR_SW_PLL         (0x2U)
#define RCC_CFGR_SWS_PLL        (0x2U << 2)

/* FLASH ACR */
#define FLASH_ACR_LATENCY_2WS   (0x2U)
#define FLASH_ACR_PRFTEN        (1U << 8)
#define FLASH_ACR_ICEN          (1U << 9)
#define FLASH_ACR_DCEN          (1U << 10)

/* ADC SR */
#define ADC_SR_EOC              (1U << 1)

/* ADC CR2 */
#define ADC_CR2_ADON            (1U << 0)
#define ADC_CR2_SWSTART         (1U << 30)

/* USART SR */
#define USART_SR_RXNE           (1U << 5)
#define USART_SR_TXE            (1U << 7)
#define USART_SR_TC             (1U << 6)

/* USART CR1 */
#define USART_CR1_UE            (1U << 13)
#define USART_CR1_TE            (1U << 3)
#define USART_CR1_RE            (1U << 2)

/* EXTI */
#define EXTI_IMR_MR6            (1U << 6)
#define EXTI_FTSR_TR6           (1U << 6)
#define EXTI_PR_PR6             (1U << 6)

/* SysTick CTRL */
#define SYSTICK_CTRL_ENABLE     (1U << 0)
#define SYSTICK_CTRL_TICKINT    (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1U << 2)   /* 1 = processor clock */
#define SYSTICK_CTRL_COUNTFLAG  (1U << 16)

/* ============================================================
 *  Configuration constants
 * ============================================================ */
#define SYSCLK_HZ       84000000UL  /* 84 MHz via PLL */
#define APB1_HZ         42000000UL  /* APB1 = SYSCLK/2 */
#define APB2_HZ         84000000UL  /* APB2 = SYSCLK/1 */
#define BAUD_RATE       115200UL

#define GPS_BUFFER_SIZE 128U

/* Alarm states */
#define ALARM_STATE_NONE        0
#define ALARM_STATE_LOCAL       1   /* Buzzer only */
#define ALARM_STATE_EMERGENCY   2   /* Buzzer + station notification */

/* Emergency contact (simulated) */
#define STATION_ADDRESS         0xAA

/* ============================================================
 *  Global state
 * ============================================================ */
static volatile uint32_t g_tick_ms = 0;   /* incremented by SysTick IRQ */
static volatile uint32_t g_alarm_state = ALARM_STATE_NONE;
static volatile uint32_t g_alarm_sent = 0;  /* Flag when station notified */
static volatile uint32_t g_emergency_button_pressed = 0;

static uint16_t mq7_raw   = 0;
static uint16_t lm35_raw  = 0;
static float    mq7_voltage = 0.0f;
static float    lm35_temp   = 0.0f;
static float    latitude    = 0.0f;
static float    longitude   = 0.0f;

static char    gps_buffer[GPS_BUFFER_SIZE];
static uint8_t gps_index = 0;

/* ============================================================
 *  Forward declarations
 * ============================================================ */
static void     SystemClock_Config(void);
static void     SysTick_Init(void);
static void     GPIO_Init(void);
static void     ADC1_Init(void);
static void     USART1_Init(void);
static void     USART2_Init(void);
static void     EXTI_Init(void);
static void     NVIC_Config(void);

static void     Read_Sensors(void);
static void     Update_Alarm(void);
static void     Send_Emergency_Alarm(void);
static void     Read_GPS(void);
static float    parse_latitude(const char *lat_str, char ns);
static float    parse_longitude(const char *lon_str, char ew);

static void     delay_ms(uint32_t ms);
static void     usart2_putchar(char c);
static void     usart2_print(const char *str);
static int      usart1_getchar_timeout(uint8_t *ch, uint32_t timeout_ms);

/* Redirect printf → USART2 */
int _write(int fd, char *buf, int len)
{
    (void)fd;
    for (int i = 0; i < len; i++)
        usart2_putchar(buf[i]);
    return len;
}

/* ============================================================
 *  EXTI IRQ Handler (Emergency Button)
 * ============================================================ */
void EXTI9_5_IRQHandler(void)
{
    /* Check if interrupt is from line 6 */
    if (EXTI->PR & EXTI_PR_PR6)
    {
        EXTI->PR = EXTI_PR_PR6;  /* Clear pending bit */
        
        g_emergency_button_pressed = 1;
        g_alarm_state = ALARM_STATE_EMERGENCY;
        g_alarm_sent = 0;  /* Force re-send */
        
        printf("!!! EMERGENCY BUTTON PRESSED !!!\r\n");
    }
}

/* ============================================================
 *  SysTick IRQ handler
 * ============================================================ */
void SysTick_Handler(void)
{
    g_tick_ms++;
}

/* ============================================================
 *  main
 * ============================================================ */
int main(void)
{
    SystemClock_Config();
    SysTick_Init();
    GPIO_Init();
    ADC1_Init();
    USART1_Init();
    USART2_Init();
    EXTI_Init();
    NVIC_Config();

    printf("\r\n========================================\r\n");
    printf("Firefighter Project Started\r\n");
    printf("System ready. Press emergency button (PA6) if needed.\r\n");
    printf("========================================\r\n\n");

    while (1)
    {
        Read_Sensors();
        Update_Alarm();
        Read_GPS();

        printf("State: ");
        if (g_alarm_state == ALARM_STATE_NONE)
            printf("NORMAL");
        else if (g_alarm_state == ALARM_STATE_LOCAL)
            printf("LOCAL ALARM");
        else if (g_alarm_state == ALARM_STATE_EMERGENCY)
            printf("*** EMERGENCY ***");
        printf(" | Gas: %.2fV | Temp: %.2fC | Lat: %.6f | Lon: %.6f\r\n",
               mq7_voltage, lm35_temp, latitude, longitude);

        delay_ms(500);
    }
}

/* ============================================================
 *  Clock configuration  (HSI -> PLL -> 84 MHz)
 * ============================================================ */
static void SystemClock_Config(void)
{
    /* 1. Enable HSI and wait */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    /* 2. Configure Flash latency for 84 MHz (2 wait states) */
    FLASH->ACR = FLASH_ACR_LATENCY_2WS |
                 FLASH_ACR_PRFTEN      |
                 FLASH_ACR_ICEN        |
                 FLASH_ACR_DCEN;

    /*
     * 3. PLLCFGR:
     *   PLLSRC = HSI (bit 22 = 0)
     *   PLLM   = 16  (bits 5:0)
     *   PLLN   = 336 (bits 14:6)
     *   PLLP   = 4   (bits 17:16 = 0b01)
     *   PLLQ   = 7   (bits 27:24)
     */
    RCC->PLLCFGR = (16U  <<  0)  |  /* PLLM */
                   (336U <<  6)  |  /* PLLN */
                   (1U   << 16)  |  /* PLLP = 4 */
                   (7U   << 24);    /* PLLQ */

    /* 4. Enable PLL and wait */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /*
     * 5. CFGR: AHB prescaler = 1, APB1 = /2, APB2 = /1,
     *          then switch SYSCLK to PLL
     */
    RCC->CFGR = (0U << 4)  |   /* HPRE  : AHB  /1  */
                (4U << 10) |   /* PPRE1 : APB1 /2  */
                (0U << 13) |   /* PPRE2 : APB2 /1  */
                RCC_CFGR_SW_PLL;

    /* Wait until PLL is used as system clock */
    while ((RCC->CFGR & (3U << 2)) != (RCC_CFGR_SWS_PLL));
}

/* ============================================================
 *  SysTick: 1 ms tick using processor clock (84 MHz)
 * ============================================================ */
static void SysTick_Init(void)
{
    SYSTICK->LOAD = (SYSCLK_HZ / 1000U) - 1U;  /* reload for 1 ms */
    SYSTICK->VAL  = 0U;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE |    /* processor clock */
                    SYSTICK_CTRL_TICKINT    |    /* enable IRQ      */
                    SYSTICK_CTRL_ENABLE;         /* start counter   */
}

/* ============================================================
 *  GPIO initialisation
 *   PA0  -> Analog  (ADC1 CH0, MQ7)
 *   PA1  -> Analog  (ADC1 CH1, LM35)
 *   PA2  -> AF7     (USART2 TX)
 *   PA3  -> AF7     (USART2 RX)
 *   PA5  -> Output  (Buzzer)
 *   PA6  -> Input   (Emergency Button, EXTI9_5)
 *   PA7  -> Output  (Alarm LED)
 *   PA9  -> AF7     (USART1 TX)
 *   PA10 -> AF7     (USART1 RX)
 * ============================================================ */
static void GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    __asm volatile ("nop");
    __asm volatile ("nop");

    uint32_t moder = GPIOA->MODER;

    /* PA0, PA1 -> analog (11) */
    moder |= (3U << (0*2)) | (3U << (1*2));

    /* PA2 -> AF (10), PA3 -> AF (10) */
    moder &= ~((3U << (2*2)) | (3U << (3*2)));
    moder |=  (2U << (2*2)) | (2U << (3*2));

    /* PA5 -> output (01) - Buzzer */
    moder &= ~(3U << (5*2));
    moder |=  (1U << (5*2));

    /* PA6 -> input (00) - Emergency Button */
    moder &= ~(3U << (6*2));

    /* PA7 -> output (01) - Alarm LED */
    moder &= ~(3U << (7*2));
    moder |=  (1U << (7*2));

    /* PA9 -> AF (10), PA10 -> AF (10) */
    moder &= ~((3U << (9*2)) | (3U << (10*2)));
    moder |=  (2U << (9*2)) | (2U << (10*2));

    GPIOA->MODER = moder;

    /* PA6 input with pull-up (to avoid floating) */
    GPIOA->PUPDR &= ~(3U << (6*2));
    GPIOA->PUPDR |=  (1U << (6*2));  /* Pull-up */

    /* Alternate function: AF7 = USART1/2 */
    GPIOA->AFR[0] &= ~(0xFU << (2*4));
    GPIOA->AFR[0] |=  (7U   << (2*4));
    GPIOA->AFR[0] &= ~(0xFU << (3*4));
    GPIOA->AFR[0] |=  (7U   << (3*4));
    GPIOA->AFR[1] &= ~(0xFU << ((9-8)*4));
    GPIOA->AFR[1] |=  (7U   << ((9-8)*4));
    GPIOA->AFR[1] &= ~(0xFU << ((10-8)*4));
    GPIOA->AFR[1] |=  (7U   << ((10-8)*4));

    /* PA5, PA7 output speed: high speed (11) */
    GPIOA->OSPEEDR |= (3U << (5*2));
    GPIOA->OSPEEDR |= (3U << (7*2));

    /* Initial states: Buzzer OFF, LED OFF */
    GPIOA->BSRR = (1U << (5 + 16));  /* PA5 low */
    GPIOA->BSRR = (1U << (7 + 16));  /* PA7 low */
}

/* ============================================================
 *  EXTI Initialisation (Emergency Button on PA6)
 *  PA6 -> EXTI line 6, falling edge trigger
 * ============================================================ */
static void EXTI_Init(void)
{
    /* Enable SYSCFG clock */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    __asm volatile ("nop");
    __asm volatile ("nop");

    /* Connect PA6 to EXTI line 6 */
    SYSCFG->EXTICR[1] &= ~(0xFU << 8);
    SYSCFG->EXTICR[1] |=  (0x0U << 8);  /* PA[0..3]? Wait, need to check */
    /* EXTI line 6: bits 11:8 in EXTICR1 */
    /* PA6 = 0000, PB6 = 0001, PC6 = 0010, etc. */
    /* But we're using PA6, so leave bits 11:8 = 0 */
    SYSCFG->EXTICR[1] &= ~(0xFU << 8);  /* Clear bits 11:8 */
    /* PA is 0, so no need to set anything */

    /* Configure EXTI line 6 for falling edge trigger */
    EXTI->FTSR |= EXTI_FTSR_TR6;
    EXTI->RTSR &= ~(1U << 6);  /* Disable rising edge */

    /* Unmask EXTI line 6 */
    EXTI->IMR |= EXTI_IMR_MR6;

    /* Clear any pending interrupt */
    EXTI->PR = EXTI_PR_PR6;
}

/* ============================================================
 *  NVIC Configuration
 * ============================================================ */
static void NVIC_Config(void)
{
    /* Enable EXTI9_5 interrupt in NVIC */
    NVIC->ISER[1] = (1U << 23);  /* EXTI9_5 is IRQ 23 */
    /* Priority: set to 0 (highest) */
    NVIC->IPR[5] = (0 << 4);  /* IRQ 23: IPR index = 23/4 = 5 */
}

/* ============================================================
 *  ADC1 initialisation
 * ============================================================ */
static void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    __asm volatile ("nop");
    __asm volatile ("nop");

    ADC_COMMON->CCR = (1U << 16);  /* PCLK2/4 */
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;
    ADC1->SMPR2 = (5U << (0*3)) | (5U << (1*3));  /* 84 cycles */
    ADC1->CR2 |= ADC_CR2_ADON;
    delay_ms(1);
}

/* ============================================================
 *  Helper: read one ADC channel
 * ============================================================ */
static uint16_t ADC1_Read(uint8_t channel)
{
    ADC1->SQR3 = (channel & 0x1FU);
    ADC1->SQR1 = 0;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

/* ============================================================
 *  USART1  (GPS, 115200 8N1)
 * ============================================================ */
static void USART1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    __asm volatile ("nop");
    USART1->BRR = (uint32_t)(APB2_HZ / BAUD_RATE);
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/* ============================================================
 *  USART2  (debug printf, 115200 8N1)
 * ============================================================ */
static void USART2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    __asm volatile ("nop");
    USART2->BRR = (uint32_t)(APB1_HZ / BAUD_RATE);
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/* ============================================================
 *  Delay using SysTick millisecond counter
 * ============================================================ */
static void delay_ms(uint32_t ms)
{
    uint32_t start = g_tick_ms;
    while ((g_tick_ms - start) < ms);
}

/* ============================================================
 *  USART2 TX helpers
 * ============================================================ */
static void usart2_putchar(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint32_t)(uint8_t)c;
}

static void usart2_print(const char *str)
{
    while (*str)
        usart2_putchar(*str++);
}

/* ============================================================
 *  USART1 RX with timeout
 * ============================================================ */
static int usart1_getchar_timeout(uint8_t *ch, uint32_t timeout_ms)
{
    uint32_t start = g_tick_ms;
    while (!(USART1->SR & USART_SR_RXNE))
    {
        if ((g_tick_ms - start) >= timeout_ms)
            return 0;
    }
    *ch = (uint8_t)(USART1->DR & 0xFFU);
    return 1;
}

/* ============================================================
 *  Read_Sensors
 * ============================================================ */
static void Read_Sensors(void)
{
    const float VREF = 3.3f;

    mq7_raw  = ADC1_Read(0);
    lm35_raw = ADC1_Read(1);

    mq7_voltage = (mq7_raw  / 4095.0f) * VREF;
    lm35_temp   = (lm35_raw / 4095.0f) * VREF / 0.01f;

    printf("Sensors: MQ7=%.2fV | LM35=%.2fC\r\n", mq7_voltage, lm35_temp);
}

/* ============================================================
 *  Send_Emergency_Alarm  (simulated station notification)
 *  In a real system, this would transmit via radio/LoRa/GSM
 * ============================================================ */
static void Send_Emergency_Alarm(void)
{
    if (g_alarm_sent)
        return;

    printf("\r\n");
    printf("****************************************\r\n");
    printf("*  EMERGENCY ALARM TO STATION          *\r\n");
    printf("*  ----------------------------------- *\r\n");
    printf("*  Time:   %lu ms                     *\r\n", g_tick_ms);
    printf("*  GPS:    %.6f, %.6f                *\r\n", latitude, longitude);
    printf("*  Gas:    %.2f V                     *\r\n", mq7_voltage);
    printf("*  Temp:   %.2f C                     *\r\n", lm35_temp);
    printf("*  Device: 0x%02X                      *\r\n", STATION_ADDRESS);
    printf("*  Status: ALERT - Immediate response  *\r\n");
    printf("****************************************\r\n");

    /* Blink alarm LED to indicate transmission */
    for (int i = 0; i < 5; i++)
    {
        GPIOA->BSRR = (1U << 7);         /* LED ON */
        delay_ms(100);
        GPIOA->BSRR = (1U << (7 + 16));  /* LED OFF */
        delay_ms(100);
    }

    g_alarm_sent = 1;
}

/* ============================================================
 *  Update_Alarm  (main alarm logic)
 * ============================================================ */
static void Update_Alarm(void)
{
    /* Check if emergency button was pressed (handled in interrupt) */
    if (g_emergency_button_pressed)
    {
        g_alarm_state = ALARM_STATE_EMERGENCY;
        g_alarm_sent = 0;
        g_emergency_button_pressed = 0;
        Send_Emergency_Alarm();
    }

    /* Check sensor thresholds */
    if (mq7_voltage > 1.2f || lm35_temp > 60.0f)  /* Adjusted threshold! */
    {
        /* Escalate from local to emergency if critical */
        if (mq7_voltage > 2.5f || lm35_temp > 80.0f)
        {
            if (g_alarm_state != ALARM_STATE_EMERGENCY)
            {
                g_alarm_state = ALARM_STATE_EMERGENCY;
                g_alarm_sent = 0;
                Send_Emergency_Alarm();
            }
        }
        else
        {
            g_alarm_state = ALARM_STATE_LOCAL;
        }

        /* Buzzer ON */
        GPIOA->BSRR = (1U << 5);
    }
    else
    {
        /* No alarm condition */
        if (g_alarm_state == ALARM_STATE_EMERGENCY)
        {
            /* Emergency alarm persists - re-send every 10 seconds */
            static uint32_t last_resend = 0;
            if ((g_tick_ms - last_resend) > 10000)
            {
                last_resend = g_tick_ms;
                g_alarm_sent = 0;
                Send_Emergency_Alarm();
            }
            /* Keep buzzer on */
            GPIOA->BSRR = (1U << 5);
        }
        else
        {
            g_alarm_state = ALARM_STATE_NONE;
            /* Buzzer OFF */
            GPIOA->BSRR = (1U << (5 + 16));
            /* LED OFF */
            GPIOA->BSRR = (1U << (7 + 16));
        }
    }

    /* LED indicates alarm state */
    if (g_alarm_state == ALARM_STATE_EMERGENCY)
    {
        GPIOA->BSRR = (1U << 7);  /* LED ON (steady for emergency) */
    }
    else if (g_alarm_state == ALARM_STATE_LOCAL)
    {
        /* Blink LED for local alarm */
        static uint32_t last_blink = 0;
        if ((g_tick_ms - last_blink) > 500)
        {
            last_blink = g_tick_ms;
            GPIOA->ODR ^= (1U << 7);  /* Toggle LED */
        }
    }
}

/* ============================================================
 *  parse_latitude
 * ============================================================ */
static float parse_latitude(const char *lat_str, char ns)
{
    if (!lat_str || lat_str[0] == '\0') return 0.0f;
    float raw = (float)atof(lat_str);
    int   deg = (int)(raw / 100);
    float min = raw - (float)(deg * 100);
    float result = (float)deg + min / 60.0f;
    if (ns == 'S') result = -result;
    return result;
}

/* ============================================================
 *  parse_longitude
 * ============================================================ */
static float parse_longitude(const char *lon_str, char ew)
{
    if (!lon_str || lon_str[0] == '\0') return 0.0f;
    float raw = (float)atof(lon_str);
    int   deg = (int)(raw / 100);
    float min = raw - (float)(deg * 100);
    float result = (float)deg + min / 60.0f;
    if (ew == 'W') result = -result;
    return result;
}

/* ============================================================
 *  Read_GPS
 * ============================================================ */
static void Read_GPS(void)
{
    uint8_t c;

    while (usart1_getchar_timeout(&c, 10) == 1)
    {
        if (c == '\n')
        {
            gps_buffer[gps_index] = '\0';
            gps_index = 0;

            if (strncmp(gps_buffer, "$GPGGA", 6) == 0)
            {
                char lat_str[15] = {0};
                char lon_str[15] = {0};
                char ns = 'N';
                char ew = 'E';
                int  field = 0;

                char *token = strtok(gps_buffer, ",");
                while (token != NULL)
                {
                    field++;
                    switch (field)
                    {
                        case 3: strncpy(lat_str, token, sizeof(lat_str) - 1); break;
                        case 4: ns = token[0]; break;
                        case 5: strncpy(lon_str, token, sizeof(lon_str) - 1); break;
                        case 6: ew = token[0]; break;
                        default: break;
                    }
                    token = strtok(NULL, ",");
                }

                latitude  = parse_latitude(lat_str,  ns);
                longitude = parse_longitude(lon_str, ew);

                printf("GPS update: %.6f, %.6f\r\n", latitude, longitude);
            }
        }
        else if (c != '\r')
        {
            if (gps_index < GPS_BUFFER_SIZE - 1U)
                gps_buffer[gps_index++] = (char)c;
        }
    }
}
