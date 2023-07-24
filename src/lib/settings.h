#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

//max mailbox id
const uint32_t MAX_ID = 4294967295;
//maximum packet size, should match ChrysaLisp lk_data_size
//if your going to integrate things !
const uint32_t MAX_PACKET_SIZE = 4056;
//number of file chunks that can be in flight
const uint32_t FILE_CHUNK_WINDOW_SIZE = 32;
//ip link server port
const uint32_t IP_LINK_PORT = 3333;
#define IP_LINK_PORT_STRING "3333"
//timeouts and rates in ms
const uint32_t MAX_DIRECTORY_AGE = 10000;
const uint32_t MAX_ROUTE_AGE = 10000;
const uint32_t MAX_MESSAGE_AGE = 5000;
const uint32_t MAX_PARCEL_AGE = 10000;
const uint32_t FILE_TRANSFER_TIMEOUT = 10000;
const uint32_t USB_BULK_TRANSFER_TIMEOUT = 100;
const uint32_t GUI_FRAME_RATE = 1000/60;
const uint32_t SELECT_POLLING_RATE = 10;
const uint32_t LINK_PING_RATE = 1000;
const uint32_t DIRECTORY_PING_RATE = 5000;
const uint32_t IP_LINK_MANAGER_POLLING_RATE = 1000;
const uint32_t USB_LINK_MANAGER_POLLING_RATE = 1000;

#endif
