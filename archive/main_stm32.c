/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_I2C_ADDR    (0x27 << 1)
#define LCD_COLS        16
#define LCD_ROWS        2

#define NOTE_DO     262
#define NOTE_RE     294
#define NOTE_MI     330
#define NOTE_FA     349
#define NOTE_SOL    392
#define NOTE_LA     440
#define NOTE_SI     494
#define NOTE_DO2    523
#define NOTE_REST   0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/* Software PWM */
volatile uint8_t  pwm_counter = 0;
volatile uint8_t  pwm_duty    = 0;
volatile uint8_t  pwm_enable  = 0;

/* LCD */
static I2C_HandleTypeDef *_hi2c_lcd;
static uint8_t _backlight = 0x08;

/* Mode & navigasi */
volatile uint8_t  mode     = 0;
volatile uint8_t  sub_mode = 0;

/* ADC & brightness */
volatile uint16_t adc_value          = 0;
volatile uint16_t brightness         = 100;
volatile uint8_t  display_brightness = 0;

/* Status LED */
volatile uint8_t led_state[8] = {0};
volatile uint8_t LED1_status  = 0;
volatile uint8_t LED2_status  = 0;
volatile uint8_t LED3_status  = 0;
volatile uint8_t LED4_status  = 0;
volatile uint8_t LED5_status  = 0;
volatile uint8_t LED6_status  = 0;
volatile uint8_t LED7_status  = 0;
volatile uint8_t LED8_status  = 0;

/* Tombol */
uint8_t btn1_prev = 1;
uint8_t btn2_prev = 1;
uint8_t btn3_prev = 1;

/* Interrupt BTN2 */
volatile uint8_t  interrupt_active      = 0;
volatile uint32_t interrupt_timer       = 0;
volatile uint8_t  mode_before_interrupt = 0;

/* Mode 1: LED Running */
volatile uint32_t shift_timer     = 0;
volatile uint8_t  shift_pos       = 0;
volatile uint32_t shift_speed     = 300;
volatile uint32_t blink_count     = 0;
volatile uint8_t  fill_count      = 0;
volatile uint8_t  bounce_dir      = 1;
volatile uint8_t  all_blink_state = 0;
volatile uint32_t all_blink_timer = 0;
volatile uint8_t  filling_phase   = 0;

/* Mode 3: Potensio LED */
volatile uint8_t  pot_led_count = 0;
volatile uint16_t pot_gauge     = 0;

/* Mode 2: Sawtooth counter */
volatile uint32_t sawtooth = 0;

/* Mode 4: Musik */
volatile uint8_t  song_index       = 0;
volatile uint32_t note_timer       = 0;
volatile uint16_t current_note_idx = 0;
volatile uint8_t  note_started     = 0;

typedef struct {
    uint16_t freq;
    uint16_t duration;
    char     lyric[17];
    char     note_name[4];
} Note;

const Note song1[] = {
    {NOTE_DO,   300, "Hap-         ", "DO "},
    {NOTE_DO,   300, "py           ", "DO "},
    {NOTE_RE,   600, "birth-       ", "RE "},
    {NOTE_DO,   600, "day          ", "DO "},
    {NOTE_FA,   600, "to           ", "FA "},
    {NOTE_MI,   900, "you          ", "MI "},

    {NOTE_DO,   300, "Hap-         ", "DO "},
    {NOTE_DO,   300, "py           ", "DO "},
    {NOTE_RE,   600, "birth-       ", "RE "},
    {NOTE_DO,   600, "day          ", "DO "},
    {NOTE_SOL,  600, "to           ", "SOL"},
    {NOTE_FA,   900, "you          ", "FA "},

    {NOTE_DO,   300, "Hap-         ", "DO "},
    {NOTE_DO,   300, "py           ", "DO "},
    {NOTE_DO2,  600, "birth-       ", "DO2"},
    {NOTE_LA,   600, "day          ", "LA "},
    {NOTE_FA,   600, "dear         ", "FA "},
    {NOTE_MI,   600, "~            ", "MI "},
    {NOTE_RE,   900, "~            ", "RE "},

    {NOTE_SI,   300, "Hap-         ", "SI "},
    {NOTE_SI,   300, "py           ", "SI "},
    {NOTE_LA,   600, "birth-       ", "LA "},
    {NOTE_FA,   600, "day          ", "FA "},
    {NOTE_SOL,  600, "to           ", "SOL"},
    {NOTE_FA,   900, "you          ", "FA "},

    {NOTE_REST, 500, "             ", "   "},
};
const uint16_t song1_len = sizeof(song1) / sizeof(Note);

const Note song2[] = {
    {NOTE_DO,   300, "Twin-        ", "DO "},
    {NOTE_DO,   300, "kle          ", "DO "},
    {NOTE_SOL,  300, "twin-        ", "SOL"},
    {NOTE_SOL,  300, "kle          ", "SOL"},
    {NOTE_LA,   300, "lit-         ", "LA "},
    {NOTE_LA,   300, "tle          ", "LA "},
    {NOTE_SOL,  600, "star         ", "SOL"},

    {NOTE_FA,   300, "How          ", "FA "},
    {NOTE_FA,   300, "I            ", "FA "},
    {NOTE_MI,   300, "won-         ", "MI "},
    {NOTE_MI,   300, "der          ", "MI "},
    {NOTE_RE,   300, "what         ", "RE "},
    {NOTE_RE,   300, "you          ", "RE "},
    {NOTE_DO,   600, "are          ", "DO "},

    {NOTE_SOL,  300, "Up           ", "SOL"},
    {NOTE_SOL,  300, "a-           ", "SOL"},
    {NOTE_FA,   300, "bove         ", "FA "},
    {NOTE_FA,   300, "the          ", "FA "},
    {NOTE_MI,   300, "world        ", "MI "},
    {NOTE_MI,   300, "so           ", "MI "},
    {NOTE_RE,   600, "high         ", "RE "},

    {NOTE_SOL,  300, "Like         ", "SOL"},
    {NOTE_SOL,  300, "a            ", "SOL"},
    {NOTE_FA,   300, "dia-         ", "FA "},
    {NOTE_FA,   300, "mond         ", "FA "},
    {NOTE_MI,   300, "in           ", "MI "},
    {NOTE_MI,   300, "the          ", "MI "},
    {NOTE_RE,   600, "sky          ", "RE "},

    {NOTE_DO,   300, "Twin-        ", "DO "},
    {NOTE_DO,   300, "kle          ", "DO "},
    {NOTE_SOL,  300, "twin-        ", "SOL"},
    {NOTE_SOL,  300, "kle          ", "SOL"},
    {NOTE_LA,   300, "lit-         ", "LA "},
    {NOTE_LA,   300, "tle          ", "LA "},
    {NOTE_SOL,  600, "star         ", "SOL"},

    {NOTE_REST, 300, "             ", "   "},
};
const uint16_t song2_len = sizeof(song2) / sizeof(Note);

const Note song3[] = {
    {NOTE_MI,   300, "Ma-          ", "MI "},
    {NOTE_RE,   300, "ry           ", "RE "},
    {NOTE_DO,   300, "had          ", "DO "},
    {NOTE_RE,   300, "a            ", "RE "},
    {NOTE_MI,   300, "lit-         ", "MI "},
    {NOTE_MI,   300, "tle          ", "MI "},
    {NOTE_MI,   600, "lamb         ", "MI "},

    {NOTE_RE,   300, "lit-         ", "RE "},
    {NOTE_RE,   300, "tle          ", "RE "},
    {NOTE_RE,   600, "lamb         ", "RE "},
    {NOTE_MI,   300, "lit-         ", "MI "},
    {NOTE_SOL,  300, "tle          ", "SOL"},
    {NOTE_SOL,  600, "lamb         ", "SOL"},

    {NOTE_MI,   300, "Ma-          ", "MI "},
    {NOTE_RE,   300, "ry           ", "RE "},
    {NOTE_DO,   300, "had          ", "DO "},
    {NOTE_RE,   300, "a            ", "RE "},
    {NOTE_MI,   300, "lit-         ", "MI "},
    {NOTE_MI,   300, "tle          ", "MI "},
    {NOTE_MI,   300, "lamb         ", "MI "},
    {NOTE_MI,   300, "its          ", "MI "},

    {NOTE_RE,   300, "fleece       ", "RE "},
    {NOTE_RE,   300, "was          ", "RE "},
    {NOTE_MI,   300, "white        ", "MI "},
    {NOTE_RE,   300, "as           ", "RE "},
    {NOTE_DO,   900, "snow         ", "DO "},

    {NOTE_REST, 500, "             ", "   "},
};
const uint16_t song3_len = sizeof(song3) / sizeof(Note);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
void Buzzer_PlayNote(uint16_t freq);
void Buzzer_Stop(void);
void Buzzer_Beep(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ---- LCD Driver ---- */

static void LCD_SendNibble(uint8_t nibble)
{
    uint8_t data = (nibble & 0xF0) | _backlight;
    uint8_t buf[4];
    buf[0] = data | 0x04;
    buf[1] = data | 0x04;
    buf[2] = data & ~0x04;
    buf[3] = data & ~0x04;
    HAL_I2C_Master_Transmit(_hi2c_lcd, LCD_I2C_ADDR, buf, 4, 10);
    HAL_Delay(1);
}

static void LCD_SendByte(uint8_t data, uint8_t mode)
{
    uint8_t high = (data & 0xF0) | mode | _backlight;
    uint8_t low  = ((data << 4) & 0xF0) | mode | _backlight;
    uint8_t buf[4];
    buf[0] = high | 0x04;
    buf[1] = high & ~0x04;
    buf[2] = low  | 0x04;
    buf[3] = low  & ~0x04;
    HAL_I2C_Master_Transmit(_hi2c_lcd, LCD_I2C_ADDR, buf, 4, 10);
    HAL_Delay(1);
}

void LCD_Init(I2C_HandleTypeDef *hi2c)
{
    _hi2c_lcd = hi2c;
    HAL_Delay(100);
    LCD_SendNibble(0x30); HAL_Delay(5);
    LCD_SendNibble(0x30); HAL_Delay(1);
    LCD_SendNibble(0x30); HAL_Delay(1);
    LCD_SendNibble(0x20); HAL_Delay(1);
    LCD_SendByte(0x28, 0); HAL_Delay(1);
    LCD_SendByte(0x08, 0); HAL_Delay(1);
    LCD_SendByte(0x01, 0); HAL_Delay(2);
    LCD_SendByte(0x06, 0); HAL_Delay(1);
    LCD_SendByte(0x0C, 0); HAL_Delay(1);
}

void LCD_Clear(void)
{
    LCD_SendByte(0x01, 0);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t col, uint8_t row)
{
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_SendByte(addr, 0);
}

void LCD_PrintChar(char c)
{
    LCD_SendByte((uint8_t)c, 0x01);
}

void LCD_Print(char *str)
{
    while (*str) LCD_PrintChar(*str++);
}

void LCD_CreateChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7;
    LCD_SendByte(0x40 | (location << 3), 0);
    for (uint8_t i = 0; i < 8; i++)
    {
        LCD_SendByte(charmap[i], 0x01);
    }
}

void LCD_Backlight(uint8_t state)
{
    _backlight = state ? 0x08 : 0x00;
    uint8_t data = _backlight;
    HAL_I2C_Master_Transmit(_hi2c_lcd, LCD_I2C_ADDR, &data, 1, 10);
}

void LCD_InitCustomChars(void)
{
    uint8_t led_off[8]    = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00};
    uint8_t led_on[8]     = {0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x00};
    uint8_t bar_full[8]   = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
    uint8_t bar_empty[8]  = {0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
    uint8_t batt_cap[8]   = {0x0E, 0x1F, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00};
    uint8_t batt_fill[8]  = {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};
    uint8_t batt_empty[8] = {0x00, 0x1F, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00};
    uint8_t batt_half[8]  = {0x00, 0x1F, 0x11, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};

    LCD_CreateChar(0, led_off);
    LCD_CreateChar(1, led_on);
    LCD_CreateChar(2, bar_full);
    LCD_CreateChar(3, bar_empty);
    LCD_CreateChar(4, batt_cap);
    LCD_CreateChar(5, batt_fill);
    LCD_CreateChar(6, batt_empty);
    LCD_CreateChar(7, batt_half);
    LCD_SendByte(0x80, 0);
}

/* ---- Tampilan LCD per Mode ---- */

void LCD_ShowBrightness(uint8_t bright)
{
    char buf[17];
    LCD_SetCursor(0, 0);
    snprintf(buf, 17, "BRIGHTNESS: %3d%%", bright);
    LCD_Print(buf);

    LCD_SetCursor(0, 1);
    LCD_Print("[");
    uint8_t filled = bright * 14 / 100;
    for (uint8_t i = 0; i < 14; i++)
    {
        if (i < filled) LCD_SendByte(2, 0x01);
        else            LCD_SendByte(3, 0x01);
    }
    LCD_Print("]");
}

void LCD_ShowLEDRunning(uint8_t pos, uint8_t smode)
{
    LCD_SetCursor(0, 0);
    if      (smode == 0) LCD_Print(">> SHIFT LEFT   ");
    else if (smode == 1) LCD_Print("<< SHIFT RIGHT  ");
    else if (smode == 2) LCD_Print("** BLINK ALL    ");
    else if (smode == 3) LCD_Print("<> BOUNCE       ");
    else                 LCD_Print("|> FILLING      ");

    LCD_SetCursor(0, 1);
    for (uint8_t i = 0; i < 8; i++)
    {
        if (i == pos) LCD_SendByte(1, 0x01);
        else          LCD_SendByte(0, 0x01);
    }
    LCD_Print("        ");
}

void LCD_ShowCounter(uint32_t count, uint8_t maxval)
{
    char buf[17];
    LCD_SetCursor(0, 0);
    snprintf(buf, 17, "COUNTER:  %5lu  ", count % maxval);
    LCD_Print(buf);

    LCD_SetCursor(0, 1);
    LCD_Print("[");
    uint8_t filled = (count % maxval) * 14 / maxval;
    for (uint8_t i = 0; i < 14; i++)
    {
        if (i < filled) LCD_SendByte(2, 0x01);
        else            LCD_SendByte(3, 0x01);
    }
    LCD_Print("]");
}

void LCD_ShowBattery(uint8_t led_count)
{
    char buf[17];
    LCD_SetCursor(0, 0);
    LCD_Print("BATT");
    LCD_SendByte(4, 0x01);
    for (uint8_t i = 0; i < 8; i++)
    {
        if (i < led_count) LCD_SendByte(5, 0x01);
        else               LCD_SendByte(6, 0x01);
    }
    LCD_Print("   ");

    LCD_SetCursor(0, 1);
    snprintf(buf, 17, "   %d/8 LED ON   ", led_count);
    LCD_Print(buf);
}

void LCD_ShowSong(uint8_t song_num, const Note *notes, uint16_t idx)
{
    LCD_SetCursor(0, 0);
    if      (song_num == 0) LCD_Print("Happy Birthday  ");
    else if (song_num == 1) LCD_Print("Twinkle Twinkle ");
    else                    LCD_Print("Mary Had a Lamb ");

    char buf[17];
    char tmp[21];
    snprintf(tmp, 21, "%-3s %-12s", notes[idx].note_name, notes[idx].lyric);
    tmp[16] = '\0';
    memcpy(buf, tmp, 17);
    LCD_SetCursor(0, 1);
    LCD_Print(buf);
}

/* ---- Kontrol LED ---- */

void set_all_leds(uint8_t state)
{
    GPIO_PinState s = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    LED1_status = state; LED2_status = state;
    LED3_status = state; LED4_status = state;
    LED5_status = state; LED6_status = state;
    LED7_status = state; LED8_status = state;
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, s);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, s);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, s);
    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, s);
    HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, s);
    HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, s);
    HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, s);
    HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, s);
}

void set_all_leds_no_status(uint8_t state)
{
    GPIO_PinState s = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, s);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, s);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, s);
    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, s);
    HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, s);
    HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, s);
    HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, s);
    HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, s);
}

void set_led_by_index(uint8_t index, uint8_t state)
{
    GPIO_PinState s = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    switch (index)
    {
        case 0: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, s); LED1_status = state; break;
        case 1: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, s); LED2_status = state; break;
        case 2: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, s); LED3_status = state; break;
        case 3: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, s); LED4_status = state; break;
        case 4: HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, s); LED5_status = state; break;
        case 5: HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, s); LED6_status = state; break;
        case 6: HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, s); LED7_status = state; break;
        case 7: HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, s); LED8_status = state; break;
    }
}

/* ---- Kontrol Buzzer ---- */

void Buzzer_PlayNote(uint16_t freq)
{
    if (freq == 0)
    {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
        return;
    }
    uint32_t arr = (1000000 / freq) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, arr / 2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Buzzer_Stop(void)
{
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

void Buzzer_Beep(void)
{
    Buzzer_PlayNote(NOTE_DO2);
    HAL_Delay(80);
    Buzzer_Stop();
}

void Buzzer_Siren(uint32_t now)
{
    uint32_t phase = (now / 700) % 2;
    if (phase == 0) Buzzer_PlayNote(NOTE_LA);
    else            Buzzer_PlayNote(NOTE_DO2);
}

/* ---- LED indikator nada ---- */

void LED_ShowNote(uint16_t freq)
{
    set_all_leds(0);
    if      (freq == NOTE_DO)  set_led_by_index(0, 1);
    else if (freq == NOTE_RE)  set_led_by_index(1, 1);
    else if (freq == NOTE_MI)  set_led_by_index(2, 1);
    else if (freq == NOTE_FA)  set_led_by_index(3, 1);
    else if (freq == NOTE_SOL) set_led_by_index(4, 1);
    else if (freq == NOTE_LA)  set_led_by_index(5, 1);
    else if (freq == NOTE_SI)  set_led_by_index(6, 1);
    else if (freq == NOTE_DO2) set_led_by_index(7, 1);
}

/* ---- Deklarasi fungsi mode ---- */
void Handle_Buttons(uint32_t now);
void Mode_Brightness(void);
void Mode_LEDRunning(uint32_t now);
void Mode_SawtoothCounter(void);
void Mode_PotensioLED(void);
void Mode_Musik(uint32_t now);
void Handle_Interrupt(uint32_t now);

/* ---- Handler Tombol ---- */

void Handle_Buttons(uint32_t now)
{
    uint8_t btn1 = HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin);
    if (btn1 == 0 && btn1_prev == 1 && !interrupt_active)
    {
        HAL_Delay(50);
        btn1 = HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin);
        if (btn1 == 0)
        {
            if (mode != 4) Buzzer_Beep();
            mode++;
            if (mode > 4) mode = 0;

            pwm_enable  = 0;
            shift_pos   = 0;
            shift_timer = now;
            sub_mode    = 0;
            fill_count  = 0;
            pot_gauge   = 0;
            sawtooth    = 0;

            set_all_leds(0);
            if (mode == 0) set_all_leds(1);

            if (mode == 4)
            {
                current_note_idx = 0;
                song_index       = 0;
                note_timer       = now;
                note_started     = 0;
                Buzzer_Stop();
                LCD_Clear();
                LCD_SetCursor(0, 0); LCD_Print("  Mode Musik!   ");
                LCD_SetCursor(0, 1); LCD_Print(" BTN3=Ganti Lagu");
            }
            else
            {
                Buzzer_Stop();
            }
        }
    }
    btn1_prev = btn1;

    uint8_t btn3 = HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin);
    if (btn3 == 0 && btn3_prev == 1 && (mode == 1 || mode == 4) && !interrupt_active)
    {
        HAL_Delay(50);
        btn3 = HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin);
        if (btn3 == 0)
        {
            if (mode != 4) Buzzer_Beep();

            if (mode == 1)
            {
                sub_mode++;
                if (sub_mode > 4) sub_mode = 0;
                shift_pos     = 0;
                fill_count    = 0;
                bounce_dir    = 1;
                filling_phase = 0;
                set_all_leds(0);
            }
            else if (mode == 4)
            {
                song_index++;
                if (song_index > 2) song_index = 0;
                current_note_idx = 0;
                note_timer       = now;
                note_started     = 0;
                Buzzer_Stop();
                set_all_leds(0);
                LCD_Clear();
            }
        }
    }
    btn3_prev = btn3;
}

/* ---- Mode 0: Brightness ---- */

void Mode_Brightness(void)
{
    if (interrupt_active)
    {
        pwm_enable = 0;
        set_all_leds_no_status(1);
        return;
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_value  = HAL_ADC_GetValue(&hadc1);
    brightness = adc_value * 100 / 4095;

    pwm_duty   = brightness;
    pwm_enable = 1;

    display_brightness = brightness;
    LED1_status = 1; LED2_status = 1;
    LED3_status = 1; LED4_status = 1;
    LED5_status = 1; LED6_status = 1;
    LED7_status = 1; LED8_status = 1;

    static uint16_t last_brightness = 255;
    static uint32_t last_lcd_update = 0;
    uint32_t now_b = HAL_GetTick();

    if (brightness != last_brightness && (now_b - last_lcd_update >= 300))
    {
        last_brightness = brightness;
        last_lcd_update = now_b;
        LCD_ShowBrightness(brightness);
    }
}

/* ---- Mode 1: LED Running ---- */

void Mode_LEDRunning(uint32_t now)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_value   = HAL_ADC_GetValue(&hadc1);
    shift_speed = 50 + (adc_value * 950 / 4095);

    if (sub_mode == 0)
    {
        if (now - shift_timer >= shift_speed)
        {
            shift_timer = now;
            set_all_leds(0);
            set_led_by_index(shift_pos, 1);
            LCD_ShowLEDRunning(shift_pos, sub_mode);
            if (shift_pos == 0) shift_pos = 7;
            else                shift_pos--;
            blink_count++;
        }
    }
    else if (sub_mode == 1)
    {
        if (now - shift_timer >= shift_speed)
        {
            shift_timer = now;
            set_all_leds(0);
            set_led_by_index(shift_pos, 1);
            LCD_ShowLEDRunning(shift_pos, sub_mode);
            shift_pos++;
            if (shift_pos > 7) shift_pos = 0;
            blink_count++;
        }
    }
    else if (sub_mode == 2)
    {
        if (now - all_blink_timer >= shift_speed)
        {
            all_blink_timer = now;
            all_blink_state = !all_blink_state;
            set_all_leds_no_status(all_blink_state);
            LED1_status = all_blink_state; LED2_status = all_blink_state;
            LED3_status = all_blink_state; LED4_status = all_blink_state;
            LED5_status = all_blink_state; LED6_status = all_blink_state;
            LED7_status = all_blink_state; LED8_status = all_blink_state;

            LCD_SetCursor(0, 0);
            LCD_Print("** BLINK ALL    ");
            LCD_SetCursor(0, 1);
            for (uint8_t i = 0; i < 8; i++)
            {
                if (all_blink_state) LCD_SendByte(1, 0x01);
                else                 LCD_SendByte(0, 0x01);
            }
            LCD_Print("        ");
        }
    }
    else if (sub_mode == 3)
    {
        if (now - shift_timer >= shift_speed)
        {
            shift_timer = now;
            set_all_leds(0);
            set_led_by_index(shift_pos, 1);
            LCD_ShowLEDRunning(shift_pos, sub_mode);
            if (bounce_dir == 1)
            {
                shift_pos++;
                if (shift_pos >= 7) bounce_dir = 0;
            }
            else
            {
                if (shift_pos > 0) shift_pos--;
                if (shift_pos == 0) bounce_dir = 1;
            }
            blink_count++;
        }
    }
    else if (sub_mode == 4)
    {
        if (now - shift_timer >= shift_speed)
        {
            shift_timer = now;
            if (filling_phase == 0)
            {
                set_led_by_index(fill_count, 1);
                LCD_ShowLEDRunning(fill_count, sub_mode);
                fill_count++;
                if (fill_count >= 8)
                {
                    filling_phase = 1;
                    fill_count    = 7;
                }
            }
            else
            {
                set_led_by_index(fill_count, 0);
                LCD_ShowLEDRunning(fill_count, sub_mode);
                if (fill_count == 0)
                {
                    filling_phase = 0;
                    fill_count    = 0;
                }
                else
                {
                    fill_count--;
                }
            }
            blink_count++;
        }
    }
}

/* ---- Mode 2: Sawtooth Counter ---- */

void Mode_SawtoothCounter(void)
{
    static uint8_t sawtooth_max = 95;
    set_all_leds(0);
    sawtooth++;
    if (sawtooth >= sawtooth_max)
    {
        sawtooth     = 0;
        sawtooth_max = (sawtooth_max == 95) ? 47 : 95;
    }
    LCD_ShowCounter(sawtooth, sawtooth_max);
    HAL_Delay(50);
}

/* ---- Mode 3: Potensio LED ---- */

void Mode_PotensioLED(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_value = HAL_ADC_GetValue(&hadc1);
    pot_gauge = (adc_value >= 3800) ? 8 : (adc_value * 8 / 3800);

    uint8_t target = pot_gauge;
    set_all_leds(0);
    for (uint8_t i = 0; i < target; i++) set_led_by_index(i, 1);

    LCD_ShowBattery(target);
    HAL_Delay(20);
}

/* ---- Mode 4: Musik ---- */

void Mode_Musik(uint32_t now)
{
    const Note *cur_song;
    uint16_t    cur_len;

    if      (song_index == 0) { cur_song = song1; cur_len = song1_len; }
    else if (song_index == 1) { cur_song = song2; cur_len = song2_len; }
    else                      { cur_song = song3; cur_len = song3_len; }

    uint32_t note_dur = (uint32_t)cur_song[current_note_idx].duration;
    uint32_t elapsed  = now - note_timer;

    if (!note_started)
    {
        Buzzer_PlayNote(cur_song[current_note_idx].freq);
        LED_ShowNote(cur_song[current_note_idx].freq);
        LCD_ShowSong(song_index, cur_song, current_note_idx);
        note_timer   = now;
        note_started = 1;
        return;
    }

    if (elapsed < (note_dur * 80 / 100))
    {
        /* fase bunyi aktif */
    }
    else if (elapsed < note_dur)
    {
        Buzzer_Stop();
        set_all_leds(0);
    }
    else
    {
        current_note_idx++;
        if (current_note_idx >= cur_len) current_note_idx = 0;
        note_started = 0;
    }
}

/* ---- Handler Interrupt ---- */

void Handle_Interrupt(uint32_t now)
{
    static uint8_t  interrupt_lcd_shown = 0;
    static uint32_t last_countdown      = 0;

    if (!interrupt_lcd_shown)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("!! INTERRUPT !! ");
        interrupt_lcd_shown = 1;
        last_countdown      = now;
    }

    uint32_t elapsed_int = now - interrupt_timer;
    uint8_t  sisa        = 5 - (elapsed_int / 1000);
    if (sisa > 5) sisa = 0;

    if (now - last_countdown >= 500)
    {
        last_countdown = now;
        char buf[17];
        LCD_SetCursor(0, 1);
        snprintf(buf, 17, "SIREN ON  [%ds]  ", sisa);
        LCD_Print(buf);
    }

    if (elapsed_int >= 5000)
    {
        interrupt_active    = 0;
        interrupt_lcd_shown = 0;
        Buzzer_Stop();
        set_all_leds(0);
        mode            = mode_before_interrupt;
        shift_pos       = 0;
        shift_timer     = now;
        all_blink_timer = now;
        note_started    = 0;
        sawtooth        = 0;
        fill_count      = 0;
        filling_phase   = 0;
        bounce_dir      = 1;
        LCD_Clear();
        if (mode == 0) set_all_leds(1);
    }
    else
    {
        Buzzer_Siren(now);
    }
    HAL_Delay(10);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  HAL_Delay(200);

  uint8_t found_addr = 0;
  for (uint8_t addr = 1; addr < 128; addr++)
  {
      if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 3, 50) == HAL_OK)
      {
          found_addr = addr;
          break;
      }
  }
  (void)found_addr;

  LCD_Init(&hi2c1);
  HAL_Delay(50);
  LCD_Backlight(1);
  LCD_InitCustomChars();
  HAL_Delay(10);
  LCD_Clear();
  HAL_Delay(5);
  LCD_SetCursor(0, 0);
  LCD_Print("Hello World!    ");
  LCD_SetCursor(0, 1);
  LCD_Print("Test LCD OK     ");

  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
      uint32_t now = HAL_GetTick();

      Handle_Buttons(now);

      if (interrupt_active) Handle_Interrupt(now);

      if (!interrupt_active)
      {
          if      (mode == 0) Mode_Brightness();
          else if (mode == 1) Mode_LEDRunning(now);
          else if (mode == 2) Mode_SawtoothCounter();
          else if (mode == 3) Mode_PotensioLED();
          else if (mode == 4) Mode_Musik(now);
      }
  /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 8;
  RCC_OscInitStruct.PLL.PLLN            = 168;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ            = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel      = ADC_CHANNEL_4;
  sConfig.Rank         = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  hi2c1.Instance             = I2C1;
  hi2c1.Init.ClockSpeed      = 100000;
  hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1     = 0;
  hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2     = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{
  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig      = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */

  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = 83;
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim1.Init.Period            = 999;
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig      = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */

  htim2.Instance               = TIM2;
  htim2.Init.Prescaler         = 8;
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = 999;
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{
  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig      = {0};
  TIM_OC_InitTypeDef      sConfigOC          = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */

  htim3.Instance               = TIM3;
  htim3.Init.Prescaler         = 83;
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim3.Init.Period            = 999;
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

  HAL_TIM_MspPostInit(&htim3);
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{
  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig      = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */

  htim4.Instance               = TIM4;
  htim4.Init.Prescaler         = 83;
  htim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim4.Init.Period            = 999;
  htim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, LED5_Pin | LED6_Pin | LED3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, LED7_Pin | LED8_Pin | LED4_Pin | LED2_Pin | LED1_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin  = BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin   = LED5_Pin | LED6_Pin | LED3_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin   = LED7_Pin | LED8_Pin | LED4_Pin | LED2_Pin | LED1_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN2_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = BTN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN3_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (interrupt_active)
        {
            set_all_leds_no_status(1);
            return;
        }

        if (!pwm_enable) return;

        pwm_counter++;
        if (pwm_counter >= 100) pwm_counter = 0;

        if (pwm_counter < pwm_duty) set_all_leds_no_status(1);
        else                        set_all_leds_no_status(0);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BTN2_Pin && !interrupt_active)
    {
        interrupt_active      = 1;
        interrupt_timer       = HAL_GetTick();
        mode_before_interrupt = mode;
        pwm_enable            = 0;
        pwm_duty              = 100;
        pwm_counter           = 0;
        set_all_leds(1);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1) {}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */