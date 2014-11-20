/**************************************************************************
*
*  This file is part of the ArduinoCC3000 library.

*  Version 1.0.1b
* 
*  Copyright (C) 2013 Chris Magagna - cmagagna@yahoo.com
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  Don't sue me if my code blows up your board and burns down your house
*
*
*  This file is the main module for the Arduino CC3000 library.
*  Your program must call CC3000_Init() before any other API calls.
* 
****************************************************************************/

#include <stdint.h>

#define MAC_ADDR_LEN	6
#define DISABLE	(0)
#define ENABLE	(1)

extern uint8_t asyncNotificationWaiting;
extern long lastAsyncEvent;
extern uint8_t dhcpIPAddress[];

extern void CC3000_Init(void);

extern volatile unsigned long ulSmartConfigFinished,
	ulCC3000Connected,
	ulCC3000DHCP,
	OkToDoShutDown,
	ulCC3000DHCP_configured;

extern volatile uint8_t ucStopSmartConfig;
