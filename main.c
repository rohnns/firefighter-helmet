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

/* ============================================================
 *  Peripheral pointers
 * ============================================================ */
#define RCC        ((RCC_TypeDef *)   RCC_BASE)
#define GPIOA      ((GPIO_TypeDef *)  GPIOA_BASE)
#define ADC1       ((ADC_TypeDef *)   ADC1_BASE)
#define ADC_COMMON ((ADC_Common_TypeDef *) ADC_COMMON_BASE)
#define USART1     ((USART_TypeDef *) USART1_BASE)
#define USART2     ((USART_TypeDef *) USART2_BASE)
#define FLASH      ((FLASH_TypeDef *) FLASH_BASE)
#define SYSTICK    ((SysTick_TypeDef *)SYSTICK_BASE)

/* ============================================================
 *  Bit-field constants
 * ============================================================ */
/* RCC AHB1ENR */
#define RCC_AHB1ENR_GPIOAEN     (1U << 0)

/* RCC APB2ENR */
#define RCC_APB2ENR_ADC1EN      (1U << 8)
#define RCC_APB2ENR_USART1EN    (1U << 4)

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

/* ============================================================
 *  Global state
 * ============================================================ */
static volatile uint32_t g_tick_ms = 0;   /* incremented by SysTick IRQ */

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

static void     Read_Sensors(void);
static void     Update_Buzzer(void);
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

    printf("Firefighter Project Started\r\n");

    while (1)
    {
        Read_Sensors();
        Update_Buzzer();
        Read_GPS();

        printf("Gas: %.2fV | Temp: %.2fC | Lat: %.6f | Lon: %.6f\r\n",
               mq7_voltage, lm35_temp, latitude, longitude);

        delay_ms(500);
    }
}

/* ============================================================
 *  Clock configuration  (HSI -> PLL -> 84 MHz)
 *
 *  PLL input  = HSI/M = 16/16 = 1 MHz
 *  VCO output = 1 * N  = 1 * 336 = 336 MHz
 *  SYSCLK     = VCO/P  = 336/4  = 84 MHz
 *  USB/SDIO   = VCO/Q  = 336/7  = 48 MHz
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
 *   PA9  -> AF7     (USART1 TX)
 *   PA10 -> AF7     (USART1 RX)
 * ============================================================ */
static void GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    __asm volatile ("nop"); /* short delay after clock enable */
    __asm volatile ("nop");

    /*
     * MODER: 00=input, 01=output, 10=AF, 11=analog
     * Clear then set the relevant 2-bit fields.
     */
    uint32_t moder = GPIOA->MODER;

    /* PA0, PA1 -> analog (11) */
    moder |= (3U << (0*2)) | (3U << (1*2));

    /* PA2 -> AF (10), PA3 -> AF (10) */
    moder &= ~((3U << (2*2)) | (3U << (3*2)));
    moder |=  (2U << (2*2)) | (2U << (3*2));

    /* PA5 -> output (01) */
    moder &= ~(3U << (5*2));
    moder |=  (1U << (5*2));

    /* PA9 -> AF (10), PA10 -> AF (10) */
    moder &= ~((3U << (9*2)) | (3U << (10*2)));
    moder |=  (2U << (9*2)) | (2U << (10*2));

    GPIOA->MODER = moder;

    /* Alternate function: AF7 = USART1/2
     * AFR[0] covers pins 0-7, AFR[1] covers pins 8-15 */
    /* PA2 (AF7) */
    GPIOA->AFR[0] &= ~(0xFU << (2*4));
    GPIOA->AFR[0] |=  (7U   << (2*4));
    /* PA3 (AF7) */
    GPIOA->AFR[0] &= ~(0xFU << (3*4));
    GPIOA->AFR[0] |=  (7U   << (3*4));
    /* PA9 (AF7) */
    GPIOA->AFR[1] &= ~(0xFU << ((9-8)*4));
    GPIOA->AFR[1] |=  (7U   << ((9-8)*4));
    /* PA10 (AF7) */
    GPIOA->AFR[1] &= ~(0xFU << ((10-8)*4));
    GPIOA->AFR[1] |=  (7U   << ((10-8)*4));

    /* PA5 output speed: high speed (11) */
    GPIOA->OSPEEDR |= (3U << (5*2));

    /* PA5 initially LOW (buzzer off) */
    GPIOA->BSRR = (1U << (5 + 16));  /* reset bit 5 */
}

/* ============================================================
 *  ADC1 initialisation  (single-channel, software-triggered)
 *  Prescaler: PCLK2/4 = 84/4 = 21 MHz  (must be ≤ 36 MHz)
 * ============================================================ */
static void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    __asm volatile ("nop");
    __asm volatile ("nop");

    /* ADC common: prescaler = PCLK2/4 (ADCPRE bits 17:16 = 01) */
    ADC_COMMON->CCR = (1U << 16);

    /* CR1: 12-bit resolution (bits 25:24 = 00), no scan mode */
    ADC1->CR1 = 0;

    /* CR2: right-align data, software trigger, no DMA */
    ADC1->CR2 = 0;

    /* Sample time for CH0 and CH1: 84 cycles (SMPR2 bits 5:0) */
    ADC1->SMPR2 = (5U << (0*3)) |  /* CH0: 101 = 84 cycles */
                  (5U << (1*3));   /* CH1: 101 = 84 cycles */

    /* Turn ADC on */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Small stabilisation delay (~10 µs) */
    delay_ms(1);
}

/* ============================================================
 *  Helper: read one ADC channel (1..18)
 * ============================================================ */
static uint16_t ADC1_Read(uint8_t channel)
{
    /* SQR3: single conversion, select channel */
    ADC1->SQR3 = (channel & 0x1FU);
    /* SQR1: sequence length = 1 (bits 23:20 = 0000) */
    ADC1->SQR1 = 0;

    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Wait for EOC */
    while (!(ADC1->SR & ADC_SR_EOC));

    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

/* ============================================================
 *  USART1  (GPS, 115200 8N1, RX only needed)
 * ============================================================ */
static void USART1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    __asm volatile ("nop");

    /* BRR = APB2_CLK / baud  (oversampling by 16) */
    USART1->BRR = (uint32_t)(APB2_HZ / BAUD_RATE);

    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/* ============================================================
 *  USART2  (debug printf, 115200 8N1, TX only needed)
 * ============================================================ */
static void USART2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    __asm volatile ("nop");

    /* BRR = APB1_CLK / baud */
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
(void)usart2_print; /* suppress unused-function warning if printf is used */

/* ============================================================
 *  USART1 RX with timeout
 *  Returns 1 if a byte was received, 0 on timeout.
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

    mq7_raw  = ADC1_Read(0);   /* PA0 = ADC1 channel 0 */
    lm35_raw = ADC1_Read(1);   /* PA1 = ADC1 channel 1 */

    mq7_voltage = (mq7_raw  / 4095.0f) * VREF;
    lm35_temp   = (lm35_raw / 4095.0f) * VREF / 0.01f;

    printf("MQ7 Voltage: %.2f V | LM35 Temp: %.2f C\r\n",
           mq7_voltage, lm35_temp);
}

/* ============================================================
 *  Update_Buzzer
 * ============================================================ */
static void Update_Buzzer(void)
{
    if (mq7_voltage > 1.2f || lm35_temp > 10.0f)
    {
        GPIOA->BSRR = (1U << 5);           /* set PA5 HIGH */
        printf("Buzzer: ON\r\n");
    }
    else
    {
        GPIOA->BSRR = (1U << (5 + 16));    /* set PA5 LOW  */
        printf("Buzzer: OFF\r\n");
    }
}

/* ============================================================
 *  parse_latitude
 *  NMEA format: DDMM.MMMM  → decimal degrees
 * ============================================================ */
static float parse_latitude(const char *lat_str, char ns)
{
    if (!lat_str || lat_str[0] == '\0') return 0.0f;

    float raw = (float)atof(lat_str);           /* e.g. 1234.5678 */
    int   deg = (int)(raw / 100);               /* 12              */
    float min = raw - (float)(deg * 100);       /* 34.5678         */
    float result = (float)deg + min / 60.0f;

    if (ns == 'S') result = -result;
    return result;
}

/* ============================================================
 *  parse_longitude
 *  NMEA format: DDDMM.MMMM → decimal degrees
 * ============================================================ */
static float parse_longitude(const char *lon_str, char ew)
{
    if (!lon_str || lon_str[0] == '\0') return 0.0f;

    float raw = (float)atof(lon_str);           /* e.g. 07734.5678 */
    int   deg = (int)(raw / 100);               /* 77              */
    float min = raw - (float)(deg * 100);       /* 34.5678         */
    float result = (float)deg + min / 60.0f;

    if (ew == 'W') result = -result;
    return result;
}

/* ============================================================
 *  Read_GPS  (non-blocking character accumulator)
 *  Reads chars from USART1 with a 10 ms per-byte timeout until
 *  no more arrive, parses any complete $GPGGA sentence.
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

                printf("GPS: Latitude %.6f | Longitude %.6f\r\n",
                       latitude, longitude);
            }
        }
        else if (c != '\r')
        {
            if (gps_index < GPS_BUFFER_SIZE - 1U)
                gps_buffer[gps_index++] = (char)c;
        }
    }
}
