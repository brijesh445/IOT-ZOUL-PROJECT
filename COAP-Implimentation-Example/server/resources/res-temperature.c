/*
 * Copyright (c) 2022, Schmalkalden University of Applied Sciences
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *      Example CoAP resource
 * \author
 *      Tobias Tefke <t.tefke@stud.fh-sm.de>
 */


/* Standard library */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Sensors */
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h" 

/* CoAP engine */
#include "coap-engine.h"

/* A counter to keep track of the number of sent messages */
static int counter = 0;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* Define the resource */
RESOURCE(res_temperature,
         "title=\"Temperature\";rt=\"JSON\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    /* Receive sensor values and encode them */
    char message[COAP_MAX_CHUNK_SIZE] = "";
    int result = snprintf(message, COAP_MAX_CHUNK_SIZE - 1,
                    "{\"message_id\": %d, \"temperature\": %d}",
                    counter, cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED));
    counter++;
    
    /* Send messages if encoding succeeded */
    if (result < 0) {
        puts("Error while encoding message");
    } else {
        puts("Sending temperature value");

        int length = strlen(message);
        memcpy(buffer, message, length);
        
        coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_header_etag(response, (uint8_t*)&length, 1);
        coap_set_payload(response, buffer, length);
    }
}
