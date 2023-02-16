#include "led.h"

#include "driver/ledc.h"

#define RED_LED_GPIO (13)
#define GREEN_LED_GPIO (12)
#define BLUE_LED_GPIO (14)

#define GREEN_LEDC_CHANNEL LEDC_CHANNEL_0
#define RED_LEDC_CHANNEL LEDC_CHANNEL_1
#define BLUE_LEDC_CHANNEL LEDC_CHANNEL_2

#define LEDC_NUM_CH 3

#define pulseFade_time 25
#define pulse_lowDuty 0
#define pulse_highDuty 4000
#define pulse_deltaDuty_slow 100
#define pulse_deltaDuty_fast 500

void init_led(void)
{
  int ch;

  ledc_timer_config_t ledc_timer = {
      .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .freq_hz = 5000,                      // frequency of PWM signal
      .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
      .timer_num = LEDC_TIMER_0,            // timer index
      .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
  };
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel[LEDC_NUM_CH] = {
      {
          .speed_mode = LEDC_LOW_SPEED_MODE,
          .channel = RED_LEDC_CHANNEL,
          .timer_sel = LEDC_TIMER_0,
          .gpio_num = RED_LED_GPIO,
          .duty = 0,
          .hpoint = 0,
          .flags.output_invert = 1,
      },
      {
          .speed_mode = LEDC_LOW_SPEED_MODE,
          .channel = GREEN_LEDC_CHANNEL,
          .timer_sel = LEDC_TIMER_0,
          .gpio_num = GREEN_LED_GPIO,
          .duty = 0,
          .hpoint = 0,
          .flags.output_invert = 1,
      },
      {
          .speed_mode = LEDC_LOW_SPEED_MODE,
          .channel = BLUE_LEDC_CHANNEL,
          .timer_sel = LEDC_TIMER_0,
          .gpio_num = BLUE_LED_GPIO,
          .duty = 0,
          .hpoint = 0,
          .flags.output_invert = 1,
      },
  };

  for (ch = 0; ch < LEDC_NUM_CH; ch++)
  {
    ledc_channel_config(&ledc_channel[ch]);
  }
}

void set_led_color(int red, int green, int blue)
{
  red = 0.12 * (red * red);
  green = 0.12 * (green * green);
  blue = 0.12 * (blue * blue);

  ledc_set_duty(LEDC_LOW_SPEED_MODE, RED_LEDC_CHANNEL, red);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, RED_LEDC_CHANNEL);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, GREEN_LEDC_CHANNEL, green);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, GREEN_LEDC_CHANNEL);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, BLUE_LEDC_CHANNEL, blue);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, BLUE_LEDC_CHANNEL);
}

void set_led_off()
{
  ledc_set_duty(LEDC_LOW_SPEED_MODE, RED_LEDC_CHANNEL, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, RED_LEDC_CHANNEL);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, GREEN_LEDC_CHANNEL, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, GREEN_LEDC_CHANNEL);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, BLUE_LEDC_CHANNEL, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, BLUE_LEDC_CHANNEL);
}
