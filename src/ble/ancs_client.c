/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#include "btstack_config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack_run_loop.h"
#include "classic/sdp_util.h"

#include "ancs_client.h"

#include "ble/att.h"
#include "btstack_debug.h"
#include "gap.h"
#include "ble/gatt_client.h"
#include "ble/sm.h"

// ancs_client.h Start
typedef enum ancs_chunk_parser_state {
    W4_ATTRIBUTE_ID,
    W4_ATTRIBUTE_LEN,
    W4_ATTRIBUTE_COMPLETE,
} ancs_chunk_parser_state_t;

typedef enum {
    TC_IDLE,
    TC_W4_ENCRYPTED_CONNECTION,
    TC_W4_SERVICE_RESULT,
    TC_W4_CHARACTERISTIC_RESULT,
    TC_W4_DATA_SOURCE_SUBSCRIBED,
    TC_W4_NOTIFICATION_SOURCE_SUBSCRIBED,
    TC_SUBSCRIBED,
    TC_W4_DISCONNECT
} tc_state_t;

static const char * ancs_attribute_names[] = { 
    "AppIdentifier",
    "IDTitle",
    "IDSubtitle",
    "IDMessage",
    "IDMessageSize",
    "IDDate"
};
static const int ANCS_ATTRBUTE_NAMES_COUNT = sizeof(ancs_attribute_names) / sizeof(char *);

static const uint8_t ancs_service_uuid[] =             {0x79,0x05,0xF4,0x31,0xB5,0xCE,0x4E,0x99,0xA4,0x0F,0x4B,0x1E,0x12,0x2D,0x00,0xD0};
static const uint8_t ancs_notification_source_uuid[] = {0x9F,0xBF,0x12,0x0D,0x63,0x01,0x42,0xD9,0x8C,0x58,0x25,0xE6,0x99,0xA2,0x1D,0xBD};
static const uint8_t ancs_control_point_uuid[] =       {0x69,0xD1,0xD8,0xF3,0x45,0xE1,0x49,0xA8,0x98,0x21,0x9B,0xBD,0xFD,0xAA,0xD9,0xD9};
static const uint8_t ancs_data_source_uuid[] =         {0x22,0xEA,0xC6,0xE9,0x24,0xD6,0x4B,0xB5,0xBE,0x44,0xB3,0x6A,0xCE,0x7C,0x7B,0xFB};

static uint32_t ancs_notification_uid;
static uint16_t gc_handle, gc_id;
static int ancs_service_found;
static le_service_t  ancs_service;
static le_characteristic_t ancs_notification_source_characteristic;
static le_characteristic_t ancs_control_point_characteristic;
static le_characteristic_t ancs_data_source_characteristic;
static int ancs_characteristcs;
static tc_state_t tc_state = TC_IDLE;

static ancs_chunk_parser_state_t chunk_parser_state;
static uint8_t  ancs_notification_buffer[50];
static uint16_t ancs_bytes_received;
static uint16_t ancs_bytes_needed;
static uint8_t  ancs_attribute_id;
static uint16_t ancs_attribute_len;
static void (*client_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size);

static btstack_packet_callback_registration_t hci_event_callback_registration;

void ancs_client_register_callback(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size)){
    client_handler = handler; 
}

static void notify_client_text(int event_type){
    if (!client_handler) return;
    uint8_t event[6 + sizeof(ancs_notification_buffer) + 1];
    event[0] = event_type;
    event[1] = 6 + ancs_attribute_len;
    little_endian_store_16(event, 2, gc_handle);
    little_endian_store_16(event, 4, ancs_attribute_id);
    memcpy(&event[6], ancs_notification_buffer, ancs_attribute_len);
    // we're nice
    event[6+ancs_attribute_len] = 0;
    (*client_handler)(HCI_EVENT_PACKET, event, event[1] + 2);
}

static void notify_client_simple(int event_type){
    if (!client_handler) return;
    uint8_t event[4];
    event[0] = event_type;
    event[1] = 2;
    little_endian_store_16(event, 2, gc_handle);
    (*client_handler)(HCI_EVENT_PACKET, event, sizeof(event));
}

static void ancs_chunk_parser_init(void){
    chunk_parser_state = W4_ATTRIBUTE_ID;
    ancs_bytes_received = 0;
    ancs_bytes_needed = 6;
}

const char * ancs_client_attribute_name_for_id(int id){
    if (id >= ANCS_ATTRBUTE_NAMES_COUNT) return 0;
    return ancs_attribute_names[id];
}

static void ancs_chunk_parser_handle_byte(uint8_t data){
    ancs_notification_buffer[ancs_bytes_received++] = data;
    if (ancs_bytes_received < ancs_bytes_needed) return;
    switch (chunk_parser_state){
        case W4_ATTRIBUTE_ID:
            ancs_attribute_id   = ancs_notification_buffer[ancs_bytes_received-1];
            ancs_bytes_received = 0;
            ancs_bytes_needed   = 2;
            chunk_parser_state  = W4_ATTRIBUTE_LEN;
            break;
        case W4_ATTRIBUTE_LEN:
            ancs_attribute_len  = little_endian_read_16(ancs_notification_buffer, ancs_bytes_received-2);
            ancs_bytes_received = 0;
            ancs_bytes_needed   = ancs_attribute_len;
            if (ancs_attribute_len == 0) {
                ancs_bytes_needed   = 1;
                chunk_parser_state  = W4_ATTRIBUTE_ID;
                break;
            }
            chunk_parser_state  = W4_ATTRIBUTE_COMPLETE;
            break;
        case W4_ATTRIBUTE_COMPLETE:
            ancs_notification_buffer[ancs_bytes_received] = 0;
            notify_client_text(ANCS_EVENT_CLIENT_NOTIFICATION);
            ancs_bytes_received = 0;
            ancs_bytes_needed   = 1;
            chunk_parser_state  = W4_ATTRIBUTE_ID;
            break;
    }
}

static void extract_service(le_service_t * service, uint8_t * packet){
    service->start_group_handle = little_endian_read_16(packet, 4);
    service->end_group_handle   = little_endian_read_16(packet, 6);
    service->uuid16 = 0;
    swap128(&packet[8], service->uuid128);
    if (uuid_has_bluetooth_prefix(service->uuid128)){
        service->uuid16 = big_endian_read_32(service->uuid128, 0);
    }
}

static void extract_characteristic(le_characteristic_t * characteristic, uint8_t * packet){
    characteristic->start_handle = little_endian_read_16(packet, 4);
    characteristic->value_handle = little_endian_read_16(packet, 6);
    characteristic->end_handle =   little_endian_read_16(packet, 8);
    characteristic->properties =   little_endian_read_16(packet, 10);
    characteristic->uuid16 = 0;
    swap128(&packet[12], characteristic->uuid128);
    if (uuid_has_bluetooth_prefix(characteristic->uuid128)){
        characteristic->uuid16 = big_endian_read_32(characteristic->uuid128, 0);
    }
}

static void handle_hci_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){

    int connection_encrypted;

    // handle connect / disconncet events first
    switch (packet[0]) {
        case HCI_EVENT_LE_META:
            switch (packet[2]) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    gc_handle = little_endian_read_16(packet, 4);
                    printf("Connection handle 0x%04x, request encryption\n", gc_handle);

                    // we need to be paired to enable notifications
                    tc_state = TC_W4_ENCRYPTED_CONNECTION;
                    sm_send_security_request(gc_handle);
                    break;
                default:
                    break;
            }
            return;

        case HCI_EVENT_ENCRYPTION_CHANGE: 
            if (gc_handle != little_endian_read_16(packet, 3)) return;
            connection_encrypted = packet[5];
            log_info("Encryption state change: %u", connection_encrypted);
            if (!connection_encrypted) return;
            if (tc_state != TC_W4_ENCRYPTED_CONNECTION) return;

            // let's start
            printf("\nANCS Client - CONNECTED, discover ANCS service\n");
            tc_state = TC_W4_SERVICE_RESULT;
            gatt_client_discover_primary_services_by_uuid128(gc_id, gc_handle, ancs_service_uuid);
            return;
            
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            notify_client_simple(ANCS_EVENT_CLIENT_DISCONNECTED);
            return;

        default:
            break;
    }

    le_characteristic_t characteristic;
    uint8_t *           value;
    uint16_t            value_handle;
    uint16_t            value_length;

    switch(tc_state){
        case TC_W4_SERVICE_RESULT:
            switch(packet[0]){
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    extract_service(&ancs_service, packet);
                    ancs_service_found = 1;
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (!ancs_service_found){
                        printf("ANCS Service not found");
                        tc_state = TC_IDLE;
                        break;
                    }
                    tc_state = TC_W4_CHARACTERISTIC_RESULT;
                    printf("ANCS Client - Discover characteristics for ANCS SERVICE \n");
                    gatt_client_discover_characteristics_for_service(gc_id, gc_handle, &ancs_service);
                    break;
                default:
                    break;
            }
            break;
            
        case TC_W4_CHARACTERISTIC_RESULT:
            switch(packet[0]){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    extract_characteristic(&characteristic, packet);
                    if (memcmp(characteristic.uuid128, ancs_notification_source_uuid, 16) == 0){
                        printf("ANCS Notification Source Characterisic found\n");
                        ancs_notification_source_characteristic = characteristic;
                        ancs_characteristcs++;
                        break;                        
                    }
                    if (memcmp(characteristic.uuid128, ancs_control_point_uuid, 16) == 0){
                        printf("ANCS Control Point found\n");
                        ancs_control_point_characteristic = characteristic;
                        ancs_characteristcs++;
                        break;                        
                    }
                    if (memcmp(characteristic.uuid128, ancs_data_source_uuid, 16) == 0){
                        printf("ANCS Data Source Characterisic found\n");
                        ancs_data_source_characteristic = characteristic;
                        ancs_characteristcs++;
                        break;                        
                    }
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    printf("ANCS Characteristcs count %u\n", ancs_characteristcs);
                    tc_state = TC_W4_NOTIFICATION_SOURCE_SUBSCRIBED;
                    gatt_client_write_client_characteristic_configuration(gc_id, gc_handle, &ancs_notification_source_characteristic,
                        GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_NOTIFICATION_SOURCE_SUBSCRIBED:
            switch(packet[0]){
                case GATT_EVENT_QUERY_COMPLETE:
                    printf("ANCS Notification Source subscribed\n");
                    tc_state = TC_W4_DATA_SOURCE_SUBSCRIBED;
                    gatt_client_write_client_characteristic_configuration(gc_id, gc_handle, &ancs_data_source_characteristic,
                        GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_DATA_SOURCE_SUBSCRIBED:
            switch(packet[0]){
                case GATT_EVENT_QUERY_COMPLETE:
                    printf("ANCS Data Source subscribed\n");
                    tc_state = TC_SUBSCRIBED;
                    notify_client_simple(ANCS_EVENT_CLIENT_CONNECTED);
                    break;
                default:
                    break;
            }
            break;
        case TC_SUBSCRIBED:
            if (packet[0] != GATT_EVENT_NOTIFICATION && packet[0] != GATT_EVENT_INDICATION ) break;

            value_handle = little_endian_read_16(packet, 4);
            value_length = little_endian_read_16(packet, 6);
            value = &packet[8];

            if (value_handle == ancs_data_source_characteristic.value_handle){
                int i;
                for (i=0;i<value_length;i++) {
                    ancs_chunk_parser_handle_byte(value[i]);
                }
            } else if (value_handle == ancs_notification_source_characteristic.value_handle){
                ancs_notification_uid = little_endian_read_32(value, 4);
                printf("Notification received: EventID %02x, EventFlags %02x, CategoryID %02x, CategoryCount %u, UID %04x\n",
                    value[0], value[1], value[2], value[3], (int) ancs_notification_uid);
                static uint8_t get_notification_attributes[] = {0, 0,0,0,0,  0,  1,32,0,  2,32,0, 3,32,0, 4, 5};
                little_endian_store_32(get_notification_attributes, 1, ancs_notification_uid);
                ancs_notification_uid = 0;
                ancs_chunk_parser_init();
                gatt_client_write_value_of_characteristic(gc_id, gc_handle, ancs_control_point_characteristic.value_handle, 
                    sizeof(get_notification_attributes), get_notification_attributes);
            } else {
                printf("Unknown Source: ");
                printf_hexdump(value , value_length);
            }
            break;
        default:
            break;
    }    
    // app_run();
}

void ancs_client_init(void){
    hci_event_callback_registration.callback = &handle_hci_event;
    hci_add_event_handler(&hci_event_callback_registration);

    gc_id = gatt_client_register_packet_handler(&handle_hci_event);
}