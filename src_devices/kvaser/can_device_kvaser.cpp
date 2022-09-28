/*
  can_device_kvaser.cpp: Interface for Kvaser CAN devices

  (C) Copyright 2009 - 2022 by OSB connagtive GmbH

  Use, modification and distribution are subject to the GNU General
  Public License, see accompanying file LICENSE.txt
*/
#include "can_server_common.h"
#include <iostream>
#include <assert.h>
#include <canlib.h>

#ifdef WIN32
#include <windows.h>
#endif

#define HARDWARE "KVASER"
#define HARDWARE_PATCH 0

using namespace __HAL;

static struct canDevice_s {
    struct canBus_s {
        bool      mb_canBusIsOpen;
        bool      mb_channelVirtual;
        CanHandle mi_channelHandle;
        canBus_s();
    };
    canBus_s& canBus(size_t n_index);
    size_t nCanBusses();

private:
    std::vector<canBus_s> mvec_canBus;
} ss_canDevice;

inline canDevice_s::canBus_s& canDevice_s::canBus(size_t n_index)
{
    if (mvec_canBus.size() <= n_index)
        mvec_canBus.resize(n_index + 1);
    return mvec_canBus[n_index];
}

static void printErrorStatus(char* id, canStatus stat)
{
    if (stat != canOK)
    {
        char buf[100];
        buf[0] = '\0';
        // Retreive informational text about the status code
        canGetErrorText(stat, buf, sizeof(buf));
        std::cerr << id << " failed, status = " << (int)stat << " (" << buf << ")" << std::endl;
    }
}


inline size_t canDevice_s::nCanBusses()
{
    return mvec_canBus.size();
}

canDevice_s::canBus_s::canBus_s() :
    mb_canBusIsOpen(false),
    mb_channelVirtual(false),
    mi_channelHandle(-1)
{
}

bool isBusOpen(uint8_t ui8_bus)
{
    return ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen;
}

const char* getHardware()
{
    return HARDWARE;
}

unsigned getHardwarePatch()
{
    return HARDWARE_PATCH;
}

uint32_t initCardApi()
{
    return 1;
}

bool resetCard(void)
{
    return true;
}

static canStatus getAvailableChannels(int& numberOfChannels)
{
    canStatus status;

    // Channel info (enum canCHANNELDATA_*)
    char deviceProductName[255];
    int  channelNumberOnCard;
    int  cardType;

    numberOfChannels = 0;

    // Get number of channels
    status = canGetNumberOfChannels(&numberOfChannels);
    printErrorStatus((char*)"canGetNumberOfChannels", status);

    DEBUG_PRINT1("Found %d channels\n", numberOfChannels);

    // Loop and print all channels
    for (int i = 0; i < numberOfChannels; i++)
    {
        status = canGetChannelData(i, canCHANNELDATA_DEVDESCR_ASCII, deviceProductName, sizeof(deviceProductName));
        printErrorStatus((char*)"canGetChannelData", status);
        DEBUG_PRINT1("Device product name: %s\n", deviceProductName);

        status = canGetChannelData(i, canCHANNELDATA_CHAN_NO_ON_CARD, &channelNumberOnCard, sizeof(channelNumberOnCard));
        printErrorStatus((char*)"canGetChannelData", status);
        DEBUG_PRINT1("Channel number on card: %s\n", channelNumberOnCard);

        status = canGetChannelData(i, canCHANNELDATA_CARD_TYPE, &cardType, sizeof(cardType));
        printErrorStatus((char*)"canGetChannelData", status);

        ss_canDevice.canBus(i).mb_channelVirtual = cardType == canHWTYPE_VIRTUAL;

        switch (cardType)
        {
        case canHWTYPE_NONE:
            DEBUG_PRINT("Card type: undefined\n");
            break;
        case canHWTYPE_VIRTUAL:
            DEBUG_PRINT("Card type: virtual\n");
            break;
        case canHWTYPE_PCCAN:
            DEBUG_PRINT("Card type: PCCAN\n");
            break;
        case canHWTYPE_PCICAN:     // fall through
        case canHWTYPE_PCICAN_II:  // fall through
        case canHWTYPE_PCICANX_II:
            DEBUG_PRINT("Card type: PCICAN*\n");
            break;
        case canHWTYPE_USBCAN:       // fall through
        case canHWTYPE_USBCAN_II:    // fall through
        case canHWTYPE_USBCAN_KLINE: // fall through
        case canHWTYPE_USBCAN_LIGHT: // fall through
        case canHWTYPE_USBCAN_PRO:   // fall through
        case canHWTYPE_USBCAN_PRO2:
            DEBUG_PRINT("Card type: USBCAN*\n");
            break;
        default:
            DEBUG_PRINT("Card type: other\n");
        }
    }

    return status;
}

static int extractKvaserBaudrate(uint32_t wBitrate)
{
    switch (wBitrate)
    {
    case 10:
        return canBITRATE_10K;
    case 50:
        return canBITRATE_50K;
    case 62:
        return canBITRATE_62K;
    case 83:
        return canBITRATE_83K;
    case 100:
        return canBITRATE_100K;
    case 125:
        return canBITRATE_125K;
    case 250:
        return canBITRATE_250K;
    case 500:
        return canBITRATE_500K;
    case 1000:
        return canBITRATE_1M;
    default:
        //default we set the baudrate to 250
        return canBITRATE_250K;
    }
}

// PURPOSE: To initialize the specified CAN BUS to begin sending/receiving msgs
bool openBusOnCard(uint8_t ui8_bus, uint32_t wBitrate, server_c* pc_serverData)
{
    DEBUG_PRINT1("init CAN bus %d\n", ui8_bus);

    if (!ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen)
    {
        DEBUG_PRINT1("Opening CAN bus channel=%d\n", ui8_bus);

        canStatus status;
        int       numberOfChannels;
        CanHandle channelHandle;

        canInitializeLibrary();

        status = getAvailableChannels(numberOfChannels);

        if (status != canOK)
        {
            printErrorStatus((char*)"getAvailableChannels", (canStatus)status);
            return false;
        }

        if (!numberOfChannels)
        {
            std::cerr << "Could not find any CAN interface" << std::endl;
            return false;
        }

        if (ui8_bus >= numberOfChannels)
        {
            std::cerr << "Could not find CAN bus channel " << ui8_bus << std::endl;
            return false;
        }

        channelHandle = canOpenChannel(ui8_bus, canOPEN_EXCLUSIVE);

        if (channelHandle < 0)
        {
            if (pc_serverData->mb_virtualSubstitute)
            {
                channelHandle = canOpenChannel(ui8_bus, canOPEN_EXCLUSIVE | canOPEN_ACCEPT_VIRTUAL);

                if (channelHandle >= 0)
                {
                    ss_canDevice.canBus(ui8_bus).mb_channelVirtual = true;
                }
            }
        }

        if (channelHandle < 0)
        {
            printErrorStatus((char*)"canOpenChannel", (canStatus)channelHandle);
            return false;
        }

        // set baudrate
        status = canSetBusParams(
            channelHandle,
            extractKvaserBaudrate(wBitrate),
            0, 0, 0, 0, 0);

        if (status != canOK)
        {
            printErrorStatus((char*)"canSetBusParams", (canStatus)status);
            return false;
        }

        status = canBusOn(channelHandle);

        if (status != canOK)
        {
            printErrorStatus((char*)"canBusOn", (canStatus)status);
            return false;
        }


        ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen = true;
        ss_canDevice.canBus(ui8_bus).mi_channelHandle = channelHandle;
        DEBUG_PRINT1("Opened CAN bus channel %d\n", ui8_bus);

        return true;
    }
    else
    {
        return true; // already initialized and files are already open
    }
}

void closeBusOnCard(uint8_t ui8_bus, server_c* pc_serverData)
{
    (void)ui8_bus;
    (void)pc_serverData;
    canStatus status;

    if (ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen)
    {
        status = canBusOff(ss_canDevice.canBus(ui8_bus).mi_channelHandle);

        if (status != canOK)
        {
            printErrorStatus((char*)"canBusOff", (canStatus)status);
            return;
        }

        status = canClose(ss_canDevice.canBus(ui8_bus).mi_channelHandle);

        if (status != canOK)
        {
            printErrorStatus((char*)"canClose", (canStatus)status);
            return;
        }

        ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen   = false;
        ss_canDevice.canBus(ui8_bus).mb_channelVirtual = false;
        ss_canDevice.canBus(ui8_bus).mi_channelHandle  = -1;
        DEBUG_PRINT1("Closed CAN bus %d\n", ui8_bus);
    }
}

// PURPOSE: To send a msg on the specified CAN BUS
// RETURNS: non-zero if msg was sent ok
//          0 on error
int16_t sendToBus(uint8_t ui8_bus, canMsg_s* ps_canMsg, server_c* pc_serverData)
{
    canStatus           status;
    const unsigned char payloadSize = sizeof(ps_canMsg->ui8_data) / sizeof(ps_canMsg->ui8_data[0]);
          unsigned char payload[payloadSize];

    for (unsigned int i = 0; i < payloadSize; i++)
    {
        payload[i] = ps_canMsg->ui8_data[i];
    }

    assert(ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen);
    if (!ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen)
    {
        return 0;
    }

    // TODO: uncomment this?
    /*if (ss_canDevice.canBus(ui8_bus).mb_channelVirtual)
    {
        return 1;
    }*/
    status = ps_canMsg->i32_msgType ? canWrite(ss_canDevice.canBus(ui8_bus).mi_channelHandle, ps_canMsg->ui32_id, payload, payloadSize, canMSG_EXT)
                                    : canWrite(ss_canDevice.canBus(ui8_bus).mi_channelHandle, ps_canMsg->ui32_id, payload, payloadSize, 0);

    if (status != canOK)
    {
        printErrorStatus((char*)"canWrite", (canStatus)status);
        return 0;
    }
    else
    {
        return 1;
    }
}

bool readFromBus(uint8_t ui8_bus, canMsg_s* ps_canMsg, server_c* pc_serverData)
{
    canStatus     status;
    long          id;
    unsigned long timestamp;
    unsigned int  payloadSize, flags;
    unsigned char payload[sizeof(ps_canMsg->ui8_data) / sizeof(ps_canMsg->ui8_data[0])];

    if (!ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen)
    {
        return false;
    }

    // TODO: uncomment this?
    /*if (ss_canDevice.canBus(ui8_bus).mb_channelVirtual)
    {
        return 1;
    }*/
    status = canRead(ss_canDevice.canBus(ui8_bus).mi_channelHandle, &id, payload, &payloadSize, &flags, &timestamp);
    if (status != canERR_NOMSG)
    {
        printErrorStatus((char*)"canRead", (canStatus)status);
        return false;
    }

    if (status == canOK)
    {
        // a message was read
        ps_canMsg->ui32_id     = id;
        ps_canMsg->i32_msgType = (flags & canMSG_EXT ? 1 : 0);
        ps_canMsg->i32_len     = payloadSize;

        (void)memcpy(ps_canMsg->ui8_data, payload, payloadSize);

        return true;
    }

    return false;
}

int32_t getServerTimeFromClientTime(client_c& r_receiveClient, int32_t ai32_clientTime)
{
    return ai32_clientTime + r_receiveClient.i32_msecStartDeltaClientMinusServer;
}
