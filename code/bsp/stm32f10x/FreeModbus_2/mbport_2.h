/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2006 Christian Walter <wolti@sil.at>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbport.h,v 1.17 2006/12/07 22:10:34 wolti Exp $
 *            mbport.h,v 1.60 2013/08/17 11:42:56 Armink Add Master Functions  $
 */

#ifndef _MB_PORT_H
#define _MB_PORT_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

typedef enum
{
    EV_READY            = 1<<0,         /*!< Startup finished. */
    EV_FRAME_RECEIVED   = 1<<1,         /*!< Frame received. */
    EV_EXECUTE          = 1<<2,         /*!< Execute function. */
    EV_FRAME_SENT       = 1<<3          /*!< Frame sent. */
} eMBEventType;

typedef enum
{
    EV_MASTER_READY                    = 1<<0,  /*!< Startup finished. */
    EV_MASTER_FRAME_RECEIVED           = 1<<1,  /*!< Frame received. */
    EV_MASTER_EXECUTE                  = 1<<2,  /*!< Execute function. */
    EV_MASTER_FRAME_SENT               = 1<<3,  /*!< Frame sent. */
    EV_MASTER_ERROR_PROCESS            = 1<<4,  /*!< Frame error process. */
    EV_MASTER_PROCESS_SUCESS           = 1<<5,  /*!< Request process success. */
    EV_MASTER_ERROR_RESPOND_TIMEOUT    = 1<<6,  /*!< Request respond timeout. */
    EV_MASTER_ERROR_RECEIVE_DATA       = 1<<7,  /*!< Request receive data error. */
    EV_MASTER_ERROR_EXECUTE_FUNCTION   = 1<<8,  /*!< Request execute function error. */
} eMBMasterEventType;

typedef enum
{
    EV_ERROR_RESPOND_TIMEOUT,         /*!< Slave respond timeout. */
    EV_ERROR_RECEIVE_DATA,            /*!< Receive frame data erroe. */
    EV_ERROR_EXECUTE_FUNCTION,        /*!< Execute function error. */
} eMBMasterErrorEventType;

/*! \ingroup modbus
 * \brief Parity used for characters in serial mode.
 *
 * The parity which should be applied to the characters sent over the serial
 * link. Please note that this values are actually passed to the porting
 * layer and therefore not all parity modes might be available.
 */
typedef enum
{
    MB_PAR_NONE,                /*!< No parity. */
    MB_PAR_ODD,                 /*!< Odd parity. */
    MB_PAR_EVEN                 /*!< Even parity. */
} eMBParity;

/* ----------------------- Supporting functions -----------------------------*/
BOOL            xMBPortEventInit_2( void );

BOOL            xMBPortEventPost_2( eMBEventType eEvent );

BOOL            xMBPortEventGet_2(  /*@out@ */ eMBEventType * eEvent );

BOOL            xMBMasterPortEventInit_2( void );

BOOL            xMBMasterPortEventPost_2( eMBMasterEventType eEvent );

BOOL            xMBMasterPortEventGet_2(  /*@out@ */ eMBMasterEventType * eEvent );

void            vMBMasterOsResInit_2( void );

BOOL            xMBMasterRunResTake_2( int32_t time );

void            vMBMasterRunResRelease_2( void );

/* ----------------------- Serial port functions ----------------------------*/

BOOL            xMBPortSerialInit_2( UCHAR ucPort, ULONG ulBaudRate,
                                   UCHAR ucDataBits, eMBParity eParity );

void            vMBPortClose_2( void );

void            xMBPortSerialClose_2( void );

void            vMBPortSerialEnable_2( BOOL xRxEnable, BOOL xTxEnable );

INLINE BOOL     xMBPortSerialGetByte_2( CHAR * pucByte );

INLINE BOOL     xMBPortSerialPutByte_2( CHAR ucByte );

BOOL            xMBMasterPortSerialInit_2( UCHAR ucPort, ULONG ulBaudRate,
                                   UCHAR ucDataBits, eMBParity eParity );

void            vMBMasterPortClose_2( void );

void            xMBMasterPortSerialClose_2( void );

void            vMBMasterPortSerialEnable_2( BOOL xRxEnable, BOOL xTxEnable );

INLINE BOOL     xMBMasterPortSerialGetByte_2( CHAR * pucByte );

INLINE BOOL     xMBMasterPortSerialPutByte_2( CHAR ucByte );

/* ----------------------- Timers functions ---------------------------------*/
BOOL            xMBPortTimersInit_2( USHORT usTimeOut50us );

void            xMBPortTimersClose_2( void );

INLINE void     vMBPortTimersEnable_2( void );

INLINE void     vMBPortTimersDisable_2( void );

BOOL            xMBMasterPortTimersInit_2( USHORT usTimeOut50us );

void            xMBMasterPortTimersClose_2( void );

INLINE void     vMBMasterPortTimersT35Enable_2( void );

INLINE void     vMBMasterPortTimersConvertDelayEnable_2( void );

INLINE void     vMBMasterPortTimersRespondTimeoutEnable_2( void );

INLINE void     vMBMasterPortTimersDisable_2( void );

/* ----------------- Callback for the master error process ------------------*/
void            vMBMasterErrorCBRespondTimeout_2( UCHAR ucDestAddress, const UCHAR* pucPDUData,
                                                USHORT ucPDULength );

void            vMBMasterErrorCBReceiveData_2( UCHAR ucDestAddress, const UCHAR* pucPDUData,
                                             USHORT ucPDULength );

void            vMBMasterErrorCBExecuteFunction_2( UCHAR ucDestAddress, const UCHAR* pucPDUData,
                                                 USHORT ucPDULength );

void            vMBMasterCBRequestScuuess_2( void );

/* ----------------------- Callback for the protocol stack ------------------*/

/*!
 * \brief Callback function for the porting layer when a new byte is
 *   available.
 *
 * Depending upon the mode this callback function is used by the RTU or
 * ASCII transmission layers. In any case a call to xMBPortSerialGetByte()
 * must immediately return a new character.
 *
 * \return <code>TRUE</code> if a event was posted to the queue because
 *   a new byte was received. The port implementation should wake up the
 *   tasks which are currently blocked on the eventqueue.
 */
extern          BOOL( *pxMBFrameCBByteReceived_2 ) ( void );

extern          BOOL( *pxMBFrameCBTransmitterEmpty_2 ) ( void );

extern          BOOL( *pxMBPortCBTimerExpired_2 ) ( void );

extern          BOOL( *pxMBMasterFrameCBByteReceived_2 ) ( void );

extern          BOOL( *pxMBMasterFrameCBTransmitterEmpty_2 ) ( void );

extern          BOOL( *pxMBMasterPortCBTimerExpired_2 ) ( void );


#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
