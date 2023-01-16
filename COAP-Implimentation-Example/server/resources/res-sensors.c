/* Standard library */
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/leds.h"
#include "lib/sensors.h"
#if CONTIKI_TARGET_ZOUL
/* Sensors */
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#else
/* Assumes Z1 mote */
#include "dev/z1-phidgets.h"
#endif

/* CoAP engine */
#include "coap-engine.h"
/* A counter to keep track of the number of sent messages */
static int counter = 0;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* Define the resource */
RESOURCE(res_all_sensors, "title=\"Temperature, Motion, LDR, LED Status \";rt=\"JSON\"", res_get_handler, NULL, NULL, NULL);

static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

#if CONTIKI_TARGET_ZOUL
    adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);
#else /* Assumes Z1 mote */
    SENSORS_ACTIVATE(phidgets);
#endif

#if CONTIKI_TARGET_ZOUL

    int motion_sensor_reading = adc_zoul.value(ZOUL_SENSORS_ADC3);
    int ldr_sensor_reading = adc_zoul.value(ZOUL_SENSORS_ADC1);
    printf("motion_sensor_reading ADC1 = %u mV\n", motion_sensor_reading);
    printf("ldr_sensor_reading ADC3 = %u mV\n", ldr_sensor_reading);
    int motion_sensor_threshold = 0;
    int ldr_sensor_threshold = 5000;
    bool condition = (motion_sensor_reading >= motion_sensor_threshold && ldr_sensor_reading <= ldr_sensor_threshold);
    if (condition)
    {
        leds_toggle(LEDS_RED);
    }
    else
    {
        leds_toggle(LEDS_GREEN);
    }
#else
    printf("Phidget 5V 1:%d\n", phidgets.value(PHIDGET5V_1));
    printf("Phidget 5V 2:%d\n", phidgets.value(PHIDGET5V_2));
    printf("Phidget 3V 1:%d\n", phidgets.value(PHIDGET3V_1));
    printf("Phidget 3V 2:%d\n\n", phidgets.value(PHIDGET3V_2));
#endif

    /* Receive sensor values and encode them */
    char message[COAP_MAX_CHUNK_SIZE] = "";

    /* int result = snprintf(message, COAP_MAX_CHUNK_SIZE - 1, "{\"message_id\": %d, \"temperature\": %d , \"motion_reading\": %d , \"ldr_reading\": %d , \"lde_status\": %d}", counter, cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED), adc_zoul.value(ZOUL_SENSORS_ADC1), adc_zoul.value(ZOUL_SENSORS_ADC3), leds_get());

    make clean
    make TARGET=zoul savetarget
    make server.upload MOTES=/dev/ttyUSB0
    make login MOTES=/dev/ttyUSB0
    make clean && make server.upload MOTES=/dev/ttyUSB0 && make login
    */
    int result = snprintf(message, COAP_MAX_CHUNK_SIZE - 1, "{\"message_id\": %d, \"motion_reading\": %d , \"ldr_reading\": %d , \"lde_status\": %d}", counter, adc_zoul.value(ZOUL_SENSORS_ADC3), adc_zoul.value(ZOUL_SENSORS_ADC1), leds_get());

    counter++;

    /* Send messages if encoding succeeded */
    if (result < 0)
    {
        puts("Error while encoding message");
    }
    else
    {
        puts("Sending temperature, motion, ldr, led values");

        int length = strlen(message);
        memcpy(buffer, message, length);

        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_header_etag(response, (uint8_t *)&length, 1);
        coap_set_payload(response, buffer, length);
    }
}
