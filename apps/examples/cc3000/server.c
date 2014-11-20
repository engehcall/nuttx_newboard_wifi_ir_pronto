/*****************************************************************************
*
*  server.c - CC3000 Sensor Application Server functionality implementation
*  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <nuttx/wireless/cc3000/wlan.h>
#include <nuttx/wireless/cc3000/evnt_handler.h>    // callback function declaration
#include <nuttx/wireless/cc3000/nvmem.h>
#include <nuttx/wireless/cc3000/socket.h>
#include <nuttx/wireless/cc3000/netapp.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "board.h"

#include <arch/board/IRremote.h>
#include <arch/board/IRremoteInt.h>
#include <arch/board/IRrecord.h>

#define SERVER_PORT 3333

long serverSocket;
sockaddr serverSocketAddr;

/** \brief Definition of data packet to be sent by server */
unsigned char dataPacket[] = { '\r', 'H', 'E', 'L', 'L', 'O', '\r', '\n' };
//char requestBuffer[SERVER_RECV_BUF_SIZE];
char requestBuffer[16];
char serverErrorCode = 0;
unsigned int closeconnection = 0;

//*****************************************************************************
//
//! Initialize Connection Server
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Waits for a connection where we will 
//
//*****************************************************************************
void initServer(void)
{       
    short  nonBlocking = 0;
    short Status;
    char portStr[12];
    memset(portStr,0,sizeof(portStr));
    
    // Open Server Socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == -1)
    {
        serverError(SERV_ERR_SOCKET);
        wlan_stop();
        while(1)
        {
		usleep(5000);	
        }
    }
    
    // Open port
    int port = SERVER_PORT;
    serverSocketAddr.sa_family = AF_INET;
    
    // Set the Port Number
    serverSocketAddr.sa_data[0] = (port & 0xFF00)>> 8;
    serverSocketAddr.sa_data[1] = (port & 0x00FF);
    
    memset (&serverSocketAddr.sa_data[2], 0, 4);
    
    if (bind(serverSocket, &serverSocketAddr, sizeof(sockaddr)) != 0) 
    {
        serverError(SERV_ERR_BIND);        
        return;
    }

        
    // Start Listening
    if (listen (serverSocket, 1) != 0)
    {         
        serverError(SERV_ERR_LISTEN);
        return;
    }
    
    Status = setsockopt(serverSocket, SOL_SOCKET, SOCKOPT_ACCEPT_NONBLOCK, &nonBlocking, sizeof(nonBlocking));
    if( Status < 0 )
    {
       while(1); //error  
    } 
    setCC3000MachineState(CC3000_SERVER_INIT);
    itoa(port, portStr, 10);
    printf("Server Initialized on port %s\n", portStr);

}

//*****************************************************************************
//
//! \brief  Waits and handle a client connection
//!
//! \param  none
//!
//! \return none
//!
//
//*****************************************************************************

void waitForConnection(void)
{
    sockaddr clientaddr;  
    socklen_t addrlen;
    volatile int bytesRecvd = 0;
    //volatile long bytesSent = 0;
    int clientDescriptor = -1;
    char clientIP[4];
    
    TICC3000fd_set readsds;
    long maxFD;
    int ret;
    struct timeval timeout;
    memset(&timeout, 0, sizeof(struct timeval));
    timeout.tv_sec = 100;
    timeout.tv_usec = 0;//50*100;
    signed char curSocket;
    int optval, optlen;
  
    
    // Check whether the server functionality is successfully initialized
    if(currentCC3000State() & CC3000_SERVER_INIT)
    {
        // Begin waiting for client and handle the connection
        while(1)
        {
            addrlen = sizeof(clientaddr);
            
            printf("Waiting for Clients\n");
            // accept blocks until we receive a connection
            while ( (clientDescriptor == -1) || (clientDescriptor == -2) )
            {
              clientDescriptor = accept(serverSocket, (sockaddr *) &clientaddr, &addrlen);
	      usleep(5000);
            }
            
            // Call user specified Clietn Accepted Event Handler            
            
            if(clientDescriptor >= 0)
            {
                setCC3000MachineState(CC3000_CLIENT_CONNECTED);
                printf("Client ");
                // Read IP and print
                clientIP[0] = clientaddr.sa_data[2];
                clientIP[1] = clientaddr.sa_data[3];
                clientIP[2] = clientaddr.sa_data[4];
                clientIP[3] = clientaddr.sa_data[5];

                printf("%d.%d.%d.%d", clientIP[3], clientIP[2], clientIP[1], clientIP[0]);
                printf(" Connected\n");
                
                //Add client socket ID to to the read set
                FD_SET(clientDescriptor, &readsds);
                maxFD = clientDescriptor + 1;
                
                while(currentCC3000State() & CC3000_CLIENT_CONNECTED)
                {

                        ret = select(maxFD, &readsds, NULL, NULL, &timeout);
                        if(ret > 0)
                        {  
                          memset(requestBuffer, 0, 128);
                          bytesRecvd = recv(clientDescriptor, requestBuffer, 128, 0);
                          if(bytesRecvd > 0)
                          {
				//printf("Dado = 0x%02X\n", bytesRecvd);
				if (bytesRecvd == 1){
					printf("Power On!\n");
					sony_poweron();
				}
				if (bytesRecvd == 2) {
					printf("Volume Down!\n");
					sony_voldown();
				}
				if (bytesRecvd == 3) {
					printf("Volume Up!\n");
					sony_volup();
				}
				if (bytesRecvd == 4) {
					printf("Channel Up!\n");
					sony_chup();
				}
				if (bytesRecvd == 5) {
					printf("Channel Down!\n");
					sony_chdown();
				}

				closeconnection = 1;
				
				//for(i = 0; i < bytesRecvd; i++)
				//	printf("%d = %s\n", requestBuffer[i], requestBuffer[i]);
                          }
                        }

                        //usleep(1000);

                        /*ret = select(maxFD, NULL, &readsds, NULL, &timeout);
			printf("SEND select returned %d\n", ret);
                        if(ret > 0)
                        {                  
                          bytesSent = send(clientDescriptor, (unsigned char *)dataPacket, sizeof(dataPacket), 0);					
                          if (bytesSent != sizeof(dataPacket))
                          {
                            bytesSent = send(clientDescriptor, (unsigned char *)dataPacket, sizeof(dataPacket), 0);
                            if (bytesSent != sizeof(dataPacket))
                            {
                              // Check if connection is closed
                              if(closeconnection == 1)
                              {
                                closeconnection = 0;
                                closesocket(clientDescriptor);
                                printf("Client Disconnected\n");
                                clientDescriptor = -1;
                                unsetCC3000MachineState(CC3000_CLIENT_CONNECTED);
                                break;
                              }
                            }
                          }
                        }*/

                        //else
                        //{
                          // Check if connection is closed
                          if(closeconnection == 1)
                          {
                            closeconnection = 0;
                            closesocket(clientDescriptor);
                            printf("Client Disconnected\n");
                            clientDescriptor = -1;
                            unsetCC3000MachineState(CC3000_CLIENT_CONNECTED);
                            break;
                          }
                          // Check if connection is timed out
                          curSocket =  getsockopt(clientDescriptor, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK  , &optval, (socklen_t*)&optlen);
                          if (curSocket == -57)
                          {
                            closeconnection = 0;
                            closesocket(clientDescriptor);
                            printf("Client Disconnected\n");
                            clientDescriptor = -1;
                            unsetCC3000MachineState(CC3000_CLIENT_CONNECTED);
                            break;
                          }
                        //}
                          
                    }
                usleep(1000);
            }
            else if(clientDescriptor == SOCKET_INACTIVE_ERR)
            {
                printf("Socket Server Timeout. Restarting Server\n");
		clientDescriptor = -1;
                // Reinitialize the server
                shutdownServer();
                initServer();
            }
        }
    }
}

//*****************************************************************************
//
//! Shut down server sockets
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Waits for a connection where we will 
//
//*****************************************************************************
void shutdownServer(void)
{
     // Close the Server's Socket
    closesocket(serverSocket);
    serverSocket = 0xFFFFFFFF;    
    unsetCC3000MachineState(CC3000_SERVER_INIT);
}

//*****************************************************************************
//
//! \brief  Waits for a connection where we will 
//!
//! \param  none
//!
//! \return none
//!
//
//*****************************************************************************
void serverError(char err)
{     
     switch(err)
     {
        case SERV_ERR_SOCKET:
            serverErrorCode = SERV_ERR_SOCKET;
         break;
        case SERV_ERR_BIND:
            serverErrorCode = SERV_ERR_BIND;
         break;
        case SERV_ERR_LISTEN:
            serverErrorCode = SERV_ERR_LISTEN;
            break;
     }
     printf("Server Error\n");
	 
     while(1)
     {
	usleep(5000);
     }
}
