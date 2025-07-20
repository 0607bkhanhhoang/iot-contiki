#include "contiki.h"
#include "dev/leds.h"
#include <stdio.h>

/* Log level Configurations 1 - INFO, 2 - WARNING, 3 - DEBUG*/
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
  #define LOG_ERROR(fmt, ...) \
    printf("[ERROR] (%s:%d) " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
  #define LOG_ERROR(fmt, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARN
  #define LOG_WARN(fmt, ...) \
    printf("[WARN]  (%s:%d) " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
  #define LOG_WARN(fmt, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
  #define LOG_INFO(fmt, ...) \
    printf("[INFO]  (%s:%d) " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
  #define LOG_INFO(fmt, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
  #define LOG_DEBUG(fmt, ...) \
    printf("[DEBUG] (%s:%d) " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
  #define LOG_DEBUG(fmt, ...)
#endif
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS(led_reds, "LED RED Blynking with timer");
PROCESS(led_greens, "LED GREEN Blynking with timer");
AUTOSTART_PROCESSES(&led_greens,&led_reds);



/*---------------------------------------------------------------------------*/

PROCESS_THREAD(led_reds, ev, data)
{
  static struct etimer timer_red;
  static int i = 0;

  PROCESS_BEGIN();
  // Modify *5 for green
  etimer_set(&timer_red, CLOCK_CONF_SECOND*5);


  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      leds_toggle(LEDS_RED);  // Toggle RED
      LOG_INFO("Led red blynked");
      LOG_DEBUG("TIMER value after LED Blynked %d",timer_red);
      etimer_reset(&timer_red);     // Restart the timer
      LOG_WARN("TIMER EXPIRE CHECK %d",timer_red);
      i=i+2;
      LOG_INFO("THE VALUE OF I %d",i);
    }
    LOG_INFO("led red off");
  }

  PROCESS_END();
}




PROCESS_THREAD(led_greens, ev, data)
{
  static struct etimer timer_green;
  static int j = 0;

  PROCESS_BEGIN();
  etimer_set(&timer_green, CLOCK_CONF_SECOND*5);


  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      leds_toggle(LEDS_GREEN);  // Toggle RED
      LOG_INFO("Led green blynked");
      LOG_DEBUG("TIMER value after LED Blynked %d",timer_green);
      etimer_reset(&timer_green);     // Restart the timer
      LOG_WARN("TIMER EXPIRE CHECK %d",timer_green);
      j=j+5;
      LOG_INFO("THE VALUE OF J %d",j);
    }
    LOG_INFO("led green off");
  }

  PROCESS_END();
}
