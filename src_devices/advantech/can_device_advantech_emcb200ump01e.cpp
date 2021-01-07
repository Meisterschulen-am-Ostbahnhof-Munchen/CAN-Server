#include "can_server_common.h"
#include "EMCB.h"

#define HARDWARE "Advantech EMCB200U-MP01E"
#define HARDWARE_PATCH 0

#define ADV_EXTENDED_FRAME 1
#define ADV_STANDARD_FRAME 0

using namespace __HAL;

static struct canDevice_s {
  struct canBus_s {
    bool          mb_canBusIsOpen;
    canBus_s();
  };
  canBus_s &canBus(size_t n_index);
  size_t nCanBusses();

private:
  std::vector< canBus_s > mvec_canBus;
} ss_canDevice;

inline canDevice_s::canBus_s &canDevice_s::canBus(size_t n_index)
{
  if (mvec_canBus.size() <= n_index)
    mvec_canBus.resize(n_index + 1);
  return mvec_canBus[n_index];
}

inline size_t canDevice_s::nCanBusses()
{
  return mvec_canBus.size();
}

canDevice_s::canBus_s::canBus_s() :
  mb_canBusIsOpen(false)
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

uint32_t initCardApi ()
{
  switch(EMCBLibInitialize())
  {
    case EMCB_STATUS_SUCCESS:
    {
      return 1;
    }
    break;
    case EMCB_STATUS_INITIALIZED:
    {
      printf("Warning: Tried to initialize the EMCB library in line %d although the library has already been initialized!\n", __LINE__);
      return 1;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Error: The EMCB library could not be initialized!\n", __LINE__);
      return 0;
    }
    break;
    default:
    {
      printf("Error: The EMCB library behaves different than specified via the EMCB-API (line %d)!\n", __LINE__);
	  return 0;
    }
  }
}

bool resetCard(void)
{
  int busCntr;
  for(busCntr=0; busCntr < ss_canDevice.nCanBusses(); busCntr++)
  {
    if(isBusOpen(busCntr))
    {
      switch(EMCBReset(busCntr))
      {
        case EMCB_STATUS_SUCCESS:
        {
          //Success -> No need to do anything
        }
        break;
        case EMCB_STATUS_NOT_INITIALIZED:
        {
          printf("Could not reset card as the library hasn't been initialized yet! (line %d)\n", __LINE__);
          return false;
        }
        break;
        case EMCB_STATUS_UNSUPPORTED:
        {
          printf("Could not reset the card for bus %d because the bus is unknown (most likely caused by an intern error of the CAN-Server)! (line %d)\n", busCntr, __LINE__);
          return false;
        }
        break;
        case EMCB_STATUS_WRITE_ERROR:
        {
          printf("Could not reset the card for bus %d! (line %d)\n", busCntr, __LINE__);
          return false;
        }
        break;
        case EMCB_STATUS_TIMEOUT:
        {
          printf("Could not reset the card for bus %d due to a timeout! (line %d)\n", busCntr, __LINE__);
          return false;
        }
        break;
        case EMCB_STATUS_ERROR:
        {
          printf("Could not reset the card for the bus %d: Unknown error! (line %d)\n", busCntr, __LINE__);
          return false;
        }
        break;
        default:
        {
          printf("Could not reset the card for the bus %d: Unknown (and unspecified) error! (line %d)\n", busCntr, __LINE__);
          return false;
        }
      }
    }
  }
  return true;
}

bool openBusOnCard(uint8_t ui8_bus, uint32_t  wBitrate , server_c* pc_serverData)
{
  (void)pc_serverData;
  switch(wBitrate)
  {
    case 1000:
    case 800:
    case 500:
    case 250:
    case 125:
    case 50:
    case 20:
    case 10:
    {
      //Bitrate is allright -> continue
    }
    break;
    default:
    {
      printf("Invalid bitrate (%d bps)! Valid values are 1000, 800, 500, 250, 125, 50, 20 and 10.\n", wBitrate);
      return false;
    }
  }
  switch(EMCBSetChannelValue(ui8_bus, EMCB_IID_BUS_SPEED, wBitrate))
  {
      case EMCB_STATUS_SUCCESS:
      {
          //Success -> Continue
      }
      break;
      case EMCB_STATUS_NOT_INITIALIZED:
      {
          printf("The EMCB library has not been initialized => Either the initialization failed or there hasn't been any call to the initialization method for this CAN bus (line %d)\n", __LINE__);
          return false;
      }
      break;
      case EMCB_STATUS_INVALID_PARAMETER:
      {
          printf("A parameter passed to a EMCB-API function has been invalid => Different library version? (line %d)\n", __LINE__);
          return false;
      }
      break;
      case EMCB_STATUS_UNSUPPORTED:
      {
          printf("Could not set baudrate for CAN channel %d => Invalid channel or changed library version? (line %d)\n", ui8_bus, __LINE__);
          return false;
      }
      break;
      case EMCB_STATUS_WRITE_ERROR:
      {
          printf("Could not set baudrate for channel %d due to a write error! => Disconnected? (line %d)\n", ui8_bus, __LINE__);
          return false;
      }
      break;
      case EMCB_STATUS_TIMEOUT:
      {
          printf("The baudrate could not be set for channel %d due to a communication timeout! (line %d)\n", ui8_bus, __LINE__);
          return false;
      }
      break;
      case EMCB_STATUS_ERROR:
      {
          printf("Unknown error after call to EMCB-API function in line %d!\n", __LINE__);
          return false;
      }
      break;
      default:
      {
          printf("Unknown (and unspecified) error after call to EMCB-API function in line %d! Changed library version?\n", __LINE__);
          return false;
      }
  }
  switch(EMCBSetChannelValue(ui8_bus, EMCB_IID_TERMINAL_RES, 0))
  {
    case EMCB_STATUS_SUCCESS:
    {
      //Success -> Do nothing
    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("The EMCB library has not been initialized => Either the initialization failed or there hasn't been any call to the initialization method for this CAN bus (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      printf("A parameter passed to a EMCB-API function has been invalid => Different library version? (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Could not set operation mode for CAN channel %d => Invalid channel or changed library version? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_WRITE_ERROR:
    {
      printf("Could not set operation mode for channel %d due to a write error! => Disconnected? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("The operation mode could not be set for channel %d due to a communication timeout! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error after call to EMCB-API function in line %d!\n", __LINE__);
      return false;
    }
    break;
    default:
    {
      printf("Unknown (and unspecified) error after call to EMCB-API function in line %d! Changed library version?\n", __LINE__);
      return false;
    }
  }
  /*switch(EMCBSetChannelValue(ui8_bus, EMCB_IID_BUS_STATUS, EMCB_BUS_STATUS_NORMAL))
  {
    case EMCB_STATUS_SUCCESS:
    {
      //Success -> Do nothing and continue

    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("The EMCB library has not been initialized => Either the initialization failed or there hasn't been any call to the initialization method for this CAN bus (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      printf("A parameter passed to a EMCB-API function has been invalid => Different library version? (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Could not set status for CAN channel %d => Invalid channel or changed library version? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_WRITE_ERROR:
    {
      printf("Could not set status for channel %d due to a write error! => Disconnected? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("The status could not be set for channel %d due to a communication timeout! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error after call to EMCB-API function in line %d!\n", __LINE__);
      return false;
    }
    break;
    default:
    {
      printf("Unknown (and unspecified) error after call to EMCB-API function in line %d! Changed library version?\n", __LINE__);
      return false;
    }
  }*/
  switch(EMCBSetChannelValue(ui8_bus, EMCB_IID_BUS_MODE, EMCB_MODE_NORMAL))
  {
    case EMCB_STATUS_SUCCESS:
    {
      //Success -> Do nothing and continue
    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("The EMCB library has not been initialized => Either the initialization failed or there hasn't been any call to the initialization method for this CAN bus (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      printf("A parameter passed to a EMCB-API function has been invalid => Different library version? (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Could not set operation mode for CAN channel %d => Invalid channel or changed library version? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_WRITE_ERROR:
    {
      printf("Could not set operation mode for channel %d due to a write error! => Disconnected? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("The operation mode could not be set for channel %d due to a communication timeout! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error after call to EMCB-API function in line %d!\n", __LINE__);
      return false;
    }
    break;
    default:
    {
      printf("Unknown (and unspecified) error after call to EMCB-API function in line %d! Changed library version?\n", __LINE__);
      return false;
    }
  }
  switch(EMCBSetChannelValue(ui8_bus, EMCB_IID_RXB0_MODE, EMCB_BUF_MODE_BOTH))
  {
    case EMCB_STATUS_SUCCESS:
    {
      //Success -> Do nothing and continue
    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("The EMCB library has not been initialized => Either the initialization failed or there hasn't been any call to the initialization method for this CAN bus (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      printf("A parameter passed to a EMCB-API function has been invalid => Different library version? (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Could not set receive buffer 0 mode for CAN channel %d => Invalid channel or changed library version? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_WRITE_ERROR:
    {
      printf("Could not set receive buffer 0 mode for channel %d due to a write error! => Disconnected? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("The receive buffer 0 mode could not be set for channel %d due to a communication timeout! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error after call to EMCB-API function in line %d!\n", __LINE__);
      return false;
    }
    break;
    default:
    {
      printf("Unknown (and unspecified) error after call to EMCB-API function in line %d! Changed library version?\n", __LINE__);
      return false;
    }
  }
  switch(EMCBSetChannelValue(ui8_bus, EMCB_IID_RXB1_MODE, EMCB_BUF_MODE_BOTH))
  {
    case EMCB_STATUS_SUCCESS:
    {
      //Success -> Do nothing and continue
    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("The EMCB library has not been initialized => Either the initialization failed or there hasn't been any call to the initialization method for this CAN bus (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      printf("A parameter passed to a EMCB-API function has been invalid => Different library version? (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Could not set receive buffer 1 mode for CAN channel %d => Invalid channel or changed library version? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_WRITE_ERROR:
    {
      printf("Could not set receive buffer 1 mode for channel %d due to a write error! => Disconnected? (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("The receive buffer 1 mode could not be set for channel %d due to a communication timeout! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error after call to EMCB-API function in line %d!\n", __LINE__);
      return false;
    }
    break;
    default:
    {
      printf("Unknown (and unspecified) error after call to EMCB-API function in line %d! Changed library version?\n", __LINE__);
      return false;
    }
  }

  ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen = true;
  return true;
}

void closeBusOnCard(uint8_t ui8_bus, server_c* pc_serverData)
{
  (void)pc_serverData;
  printf("Warning: Bus closed in program although it is not possible to close it on the hardware.\n");
  ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen = false;
}


// PURPOSE: To send a msg on the specified CAN BUS
int16_t sendToBus(uint8_t ui8_bus, canMsg_s* ps_canMsg, server_c* pc_serverData)
{
  (void)pc_serverData;
  UINT32 ret;
  while ((ret = EMCBMsgTx(ui8_bus, ps_canMsg->ui32_id, ps_canMsg->i32_msgType ? ADV_EXTENDED_FRAME : ADV_STANDARD_FRAME, 0, ps_canMsg->ui8_data, ps_canMsg->i32_len)) == EMCB_STATUS_WRITE_BUSY) {}
  switch(ret)
  {
    case EMCB_STATUS_SUCCESS:
    {
      return 1;
    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("Tried to send a message via CAN although the EMCB-Library has not been initialized! (line %d)\n", __LINE__);
      exit(EMCB_STATUS_NOT_INITIALIZED);    /*Exit because return value is currently ignored*/
      return 0;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      if(ps_canMsg->ui8_data == NULL)
      {
        printf("Tried to send CAN-Frame with NULL as data buffer! (line %d)\n", __LINE__);
      }
      else
      {
        printf("EMCB-API function returns error (invalid parameter) although the given parameter is allowed by the specification! (line %d)\n", __LINE__);
      }
      exit(EMCB_STATUS_INVALID_PARAMETER); /*Exit because return value is currently ignored*/
      return 0;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Made an attempt to write on unknown CAN-channel! (line %d)\n", __LINE__);
      exit(EMCB_STATUS_UNSUPPORTED); /*Exit because return value is currently ignored*/
      return 0;
    }
    break;
    case EMCB_STATUS_WRITE_ERROR:
    {
      printf("Could not write on CAN-bus! (TX-Error; line %d)\n", __LINE__);
      exit(EMCB_STATUS_WRITE_ERROR); /*Exit because return value is currently ignored*/
      return 0;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("Could not transfer CAN-Frame on bus %d due to a timeout! (line %d)\n", ui8_bus, __LINE__);
      exit(EMCB_STATUS_TIMEOUT); /*Exit because return value is currently ignored*/
      return 0;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error when trying to write CAN-Frame to bus %d! (line %d)\n", ui8_bus, __LINE__);
      exit(EMCB_STATUS_ERROR); /*Exit because return value is currently ignored*/
      return 0;
    }
    break;
    default:
    {
      printf("The behavior of an EMCB-API function does not match the specifications => Different library version? (line %d)\n", __LINE__);
      exit(0xDEFA); /*Exit because return value is currently ignored (dummy value)*/
      return 0;
    }
  }
}

bool readFromBus(uint8_t ui8_bus, canMsg_s* ps_canMsg, server_c* pc_serverData)
{
  (void)pc_serverData;
  if (!ss_canDevice.canBus(ui8_bus).mb_canBusIsOpen)
  {
      return false;
  }
  BYTE eid, rtr, dlc;
  UINT32 retVal;
  UINT32 id;

  retVal = EMCBMsgRx((BYTE)ui8_bus, &id, &eid, &rtr, (BYTE*)(ps_canMsg->ui8_data), &dlc);

  switch(retVal)
  {
    case EMCB_STATUS_SUCCESS:
    {
      if(rtr==0)
      {
        ps_canMsg->ui32_id = id;
        ps_canMsg->i32_len = dlc;
        ps_canMsg->i32_msgType = (eid == ADV_EXTENDED_FRAME) ? 1 : 0;
        return true;
      }
      else
      {
        printf("Received package with incorrect format (Length: %d, EID: %s, RTR-Bit: %d)!\n", ps_canMsg->i32_len, eid==0 ? "TRUE" : "FALSE", rtr);
        return false;
      }
    }
    break;
    case EMCB_STATUS_NOT_INITIALIZED:
    {
      printf("Tried to read from bus %d although the EMCB-Library has not been initialized yet! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_INVALID_PARAMETER:
    {
      printf("Unexpected error (due to invalid parameter)! (line %d)\n", __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_UNSUPPORTED:
    {
      printf("Tried to read CAN frame from invalid channel %d! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_READ_ERROR:
    {
      printf("Could not read from CAN channel %d! (RX-Error; line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_TIMEOUT:
    {
      printf("Could not read from CAN channel %d due to timeout! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_ERROR:
    {
      printf("Unknown error when trying to read from CAN channel %d! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
    break;
    case EMCB_STATUS_NO_DATA:
    {
        return false;
    }
    break;
    default:
    {
      printf("Unknown (and unspecified) error when trying to read from CAN channel %d! (line %d)\n", ui8_bus, __LINE__);
      return false;
    }
  }
}

