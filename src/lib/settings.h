#ifndef SETTINGS_H
#define SETTINGS_H

//max mailbox id
const unsigned int MAX_ID = 4294967295;
//maximum packet size
const unsigned int MAX_PACKET_SIZE = 4096;
//number of file chunks that can be in flight
const unsigned int FILE_CHUNK_WINDOW_SIZE = 32;
//ip link server port
const unsigned int IP_LINK_PORT = 3333;
//timeouts and rates in ms
const unsigned int MAX_DIRECTORY_AGE = 10000;
const unsigned int MAX_ROUTE_AGE = 10000;
const unsigned int MAX_MESSAGE_AGE = 5000;
const unsigned int MAX_PARCEL_AGE = 10000;
const unsigned int FILE_TRANSFER_TIMEOUT = 10000;
const unsigned int USB_BULK_TRANSFER_TIMEOUT = 100;
const unsigned int GUI_FRAME_RATE = 1000/60;
const unsigned int SELECT_POLLING_RATE = 10;
const unsigned int LINK_PING_RATE = 1000;
const unsigned int DIRECTORY_PING_RATE = 5000;
const unsigned int IP_LINK_MANAGER_POLLING_RATE = 1000;
const unsigned int USB_LINK_MANAGER_POLLING_RATE = 1000;

#endif
