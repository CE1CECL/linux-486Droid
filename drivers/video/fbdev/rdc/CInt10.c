/* 
 * Copyright (C) 2009 RDC Semiconductor Co.,Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For technical support : 
 *     <rdc_xorg@rdc.com.tw>
 */


#include <linux/string.h>
#include <asm/io.h>

#include "typedef.h"
#include "CInt10.h"
#include "CInt10FunProto.h"
#include "CInt10Tbl.h"

#define I2C_ERROR    0
#define I2C_OK       1
#define I2CWriteCMD  0
#define I2CReadCMD   1


USHORT Relocate_IOAddress;


__inline void OutPort(UCHAR Index,UCHAR Value)     
{              

    outb(Value, Relocate_IOAddress+Index);

    return;                          
}                                                                       

__inline UCHAR InPort(UCHAR Index)                        
{                                                                      
    UCHAR bInVal = 0x0;    

    bInVal = inb(Relocate_IOAddress+Index);
    
    return bInVal;                                                 
}

void I2CWriteClock(UCHAR I2CPort, UCHAR data)
{
    UCHAR i;
    UCHAR ujCRB7, jtemp;
    
    for (i=0;i<0x1000; i++)
    {
        ujCRB7 = ((data & 0x01) ? 0:1);            
        SetCRReg(I2CPort, ujCRB7, 0xFE);
        
        jtemp = GetCRReg(I2CPort) & 0x01;
        
        if (ujCRB7 == jtemp) break;
    }
}

void I2CDelay(UCHAR I2CPort)
{
    ULONG     i;
    UCHAR     jtemp;
         
    
             
    
    for (i=0;i<100;i++)
    {
        
        jtemp = inb(0x80);
    }    
}

void I2CWriteData(UCHAR I2CPort, UCHAR data)
{
    UCHAR       ujCRB7, jtemp;
    ULONG    i;
    
    for (i=0;i<0x1000; i++)
    {        
        ujCRB7 = ((data & 0x01) ? 0:1) << 2;        
        SetCRReg(I2CPort, ujCRB7, 0xFB);
        
        jtemp = GetCRReg(I2CPort) & 0x04;
        
        if (ujCRB7 == jtemp) break;        
    }
}

void I2CStart(UCHAR I2CPort)
{
    I2CWriteClock(I2CPort, 0x00);               
    I2CDelay(I2CPort);
    I2CWriteData(I2CPort, 0x01);                
    I2CDelay(I2CPort);    
    I2CWriteClock(I2CPort, 0x01);               
    I2CDelay(I2CPort);    
    I2CWriteData(I2CPort, 0x00);                
    I2CDelay(I2CPort);    
    I2CWriteClock(I2CPort, 0x01);                  
    I2CDelay(I2CPort);                    
}

void SendI2CDataByte(UCHAR I2CPort, UCHAR Data)
{
    UCHAR jData;
    UCHAR i;

    for (i=7;i>=0;i--)
    {
        I2CWriteClock(I2CPort, 0x00);           
        I2CDelay(I2CPort);         
        
        jData = ((Data >> i) & 0x01) ? 1:0;
        I2CWriteData(I2CPort, jData);           
        I2CDelay(I2CPort);         
        
        I2CWriteClock(I2CPort, 0x01);           
        I2CDelay(I2CPort);                           
    }                
}

rdcbool CheckACK(UCHAR I2CPort)
{
    UCHAR Data;
    
    I2CWriteClock(I2CPort, 0x00);               
    I2CDelay(I2CPort);    
    I2CWriteData(I2CPort, 0x01);                
    I2CDelay(I2CPort);    
    I2CWriteClock(I2CPort, 0x01);               
    I2CDelay(I2CPort);    
    Data = (GetCRReg(I2CPort) & 0x20) >> 5;     

                  
    return ((Data & 0x01) ? 0:1);                
}

UCHAR ReceiveI2CDataByte(UCHAR I2CPort, UCHAR I2CSlave)
{
    
    UCHAR jData=0, jTempData;   
    UCHAR i, j;

    for (i=7;i>=0;i--)
    {
        I2CWriteClock(I2CPort, 0x00);                
        I2CDelay(I2CPort);     
            
        I2CWriteData(I2CPort, 0x01);                 
        I2CDelay(I2CPort);         
        
        I2CWriteClock(I2CPort, 0x01);                
        I2CDelay(I2CPort);           
        
        for (j=0; j<0x1000; j++)
        {   
            if (((GetCRReg(I2CPort) & 0x10) >> 4)) break;
        }    
                    
        jTempData =  (GetCRReg(I2CPort) & 0x20) >> 5;
        jData |= ((jTempData & 0x01) << i); 
        
        I2CWriteClock(I2CPort, 0x01);                
        I2CDelay(I2CPort);                           
    }    
    
    return (jData);                              
}

void SendNACK(UCHAR I2CPort)
{
    I2CWriteClock(I2CPort, 0x00);               
    I2CDelay(I2CPort);    
    I2CWriteData(I2CPort, 0x01);                
    I2CDelay(I2CPort);    
    I2CWriteClock(I2CPort, 0x01);               
    I2CDelay(I2CPort);    
}

void I2CStop(UCHAR I2CPort)
{
    I2CWriteClock(I2CPort, 0x00);               
    I2CDelay(I2CPort);    
    I2CWriteData(I2CPort, 0x00);                
    I2CDelay(I2CPort);    
    I2CWriteClock(I2CPort, 0x01);               
    I2CDelay(I2CPort);    
    I2CWriteData(I2CPort, 0x01);                
    I2CDelay(I2CPort);    
    I2CWriteClock(I2CPort, 0x01);                
    I2CDelay(I2CPort);                      
}


UCHAR ReadI2C(UCHAR I2CPort, UCHAR I2CSlave, UCHAR RegIdx, UCHAR* RegData)
{
    I2CStart(I2CPort);

    
    SendI2CDataByte(I2CPort, I2CSlave|I2CWriteCMD);
    if (!CheckACK(I2CPort))
    {
        return I2C_ERROR;
    }    

    
    SendI2CDataByte(I2CPort, RegIdx);
    if (!CheckACK(I2CPort))
    {
        return I2C_ERROR;
    }    
        
    I2CStart(I2CPort);
   
    
    SendI2CDataByte(I2CPort, I2CSlave|I2CReadCMD);
    if (!CheckACK(I2CPort))
    {
        return I2C_ERROR;
    }    
   
    
    *RegData = ReceiveI2CDataByte(I2CPort, I2CSlave);
    SendNACK(I2CPort);
         
    I2CStop(I2CPort);

    return I2C_OK;       
}

UCHAR WriteI2C(UCHAR I2CPort, UCHAR I2CSlave, UCHAR RegIdx, UCHAR RegData)
{
    I2CStart(I2CPort);

    
    SendI2CDataByte(I2CPort, I2CSlave|I2CWriteCMD);
    if (!CheckACK(I2CPort))
    {
        return I2C_ERROR;
    }    

    
    SendI2CDataByte(I2CPort, RegIdx);
    if (!CheckACK(I2CPort))
    {
        return I2C_ERROR;
    }    

    
    SendI2CDataByte(I2CPort, RegData);
    if (!CheckACK(I2CPort))
    {
        return I2C_ERROR;
    }
    
    SendNACK(I2CPort);
         
    I2CStop(I2CPort);
    return I2C_OK;       
}


void SetVBERerurnStatus(USHORT VBEReturnStatus, CBIOS_ARGUMENTS *pCBiosArguments)
{
    pCBiosArguments->reg.x.AX = VBEReturnStatus;
}


void SetTimingRegs(UCHAR ucDisplayPath, MODE_INFO *pModeInfo, RRATE_TABLE *pRRateTable)
{
    USHORT usHBorder = 0, usVBorder = 0;
    USHORT usHtotal, usHDispEnd, usHBlankStart, usHBlankEnd, usHSyncStart, usHSyncEnd;
    USHORT usVtotal, usVDispEnd, usVBlankStart, usVBlankEnd, usVSyncStart, usVSyncEnd;
    ULONG  ulPixelClock;
    
    PRINTK(KERN_INFO "==Enter SetTimingRegs()==\n");

    if (pRRateTable->Attribute & HB)
    {
        usHBorder = 8;
    }

    if (pRRateTable->Attribute & VB)
    {
        usVBorder = 8;
    }

    usHtotal =      pModeInfo->H_Size + usHBorder*2 + pRRateTable->H_Blank_Time;
    usHDispEnd =    pModeInfo->H_Size;
    usHBlankStart = pModeInfo->H_Size + usHBorder;
    usHBlankEnd =   pModeInfo->H_Size + usHBorder + pRRateTable->H_Blank_Time;
    usHSyncStart =  pRRateTable->H_Sync_Start;
    usHSyncEnd =    pRRateTable->H_Sync_Start + pRRateTable->H_Sync_Time;

    usVtotal =      pModeInfo->V_Size + usVBorder*2 + pRRateTable->V_Blank_Time;
    usVDispEnd =    pModeInfo->V_Size;
    usVBlankStart = pModeInfo->V_Size + usVBorder;
    usVBlankEnd =   pModeInfo->V_Size + usVBorder + pRRateTable->V_Blank_Time;
    usVSyncStart =  pRRateTable->V_Sync_Start;
    usVSyncEnd =    pRRateTable->V_Sync_Start + pRRateTable->V_Sync_Time;

    ulPixelClock =  pRRateTable->Clock;
    
    PRINTK(KERN_INFO "H total = %d\n", usHtotal);
    SetHTotal(ucDisplayPath, usHtotal);
    
    PRINTK(KERN_INFO "H disp end = %d\n", usHDispEnd);
    SetHDisplayEnd(ucDisplayPath, usHDispEnd);

    PRINTK(KERN_INFO "H blank start = %d\n", usHBlankStart);
    SetHBlankingStart(ucDisplayPath, usHBlankStart);

    PRINTK(KERN_INFO "H blank end = %d\n", usHBlankEnd);
    SetHBlankingEnd(ucDisplayPath, usHBlankEnd);

    PRINTK(KERN_INFO "H sync start = %d\n", usHSyncStart);
    SetHSyncStart(ucDisplayPath, usHSyncStart);

    PRINTK(KERN_INFO "H sync end = %d\n", usHSyncEnd);
    SetHSyncEnd(ucDisplayPath, usHSyncEnd);

    PRINTK(KERN_INFO "V total = %d\n", usVtotal);
    SetVTotal(ucDisplayPath, usVtotal);
    
    PRINTK(KERN_INFO "V disp end = %d\n", usVDispEnd);
    SetVDisplayEnd(ucDisplayPath, usVDispEnd);

    PRINTK(KERN_INFO "V blank start = %d\n", usVBlankStart);
    SetVBlankingStart(ucDisplayPath, usVBlankStart);

    PRINTK(KERN_INFO "V blank end = %d\n", usVBlankEnd);
    SetVBlankingEnd(ucDisplayPath, usVBlankEnd);

    PRINTK(KERN_INFO "V sync start = %d\n", usVSyncStart);
    SetVSyncStart(ucDisplayPath, usVSyncStart);

    PRINTK(KERN_INFO "V sync end = %d\n", usVSyncEnd);
    SetVSyncEnd(ucDisplayPath, usVSyncEnd);
    
    PRINTK(KERN_INFO "Pixel clock = %ld\n", ulPixelClock);
    SetPixelClock(ucDisplayPath, ulPixelClock);

    PRINTK(KERN_INFO "==Exit SetTimingRegs()==\n");
}


void SetHTotal(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 40;
    
    
    Value += 7;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(HTotal1, Value);
    }
    else
    {
        WriteRegToHW(HTotal2, Value);
    }

    return;
}


void SetHDisplayEnd(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 8;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(HDispEnd1, Value);
    }
    else
    {
        WriteRegToHW(HDispEnd2, Value);
    }

    return;
    
}


void SetHBlankingStart(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 8;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(HBnkSt1, Value);
    }
    else
    {
        WriteRegToHW(HBnkSt2, Value);
    }

    return;
    
}


void SetHBlankingEnd(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 8;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(HBnkEnd1, Value);
    }
    else
    {
        WriteRegToHW(HBnkEnd2, Value);
    }

    return;
    
}


void SetHSyncStart(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 0;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(HSyncSt1, Value);
    }
    else
    {
        WriteRegToHW(HSyncSt2, Value);
    }

    return;
    
}


void SetHSyncEnd(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 0;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(HSyncEnd1, Value);
    }
    else
    {
        WriteRegToHW(HSyncEnd2, Value);
    }

    return;
    
}


void SetVTotal(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 2;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(VTotal1, Value);
    }
    else
    {
        WriteRegToHW(VTotal2, Value);
    }

    return;
    
}


void SetVDisplayEnd(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 1;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(VDispEnd1, Value);
    }
    else
    {
        WriteRegToHW(VDispEnd2, Value);
    }

    return;
    
}


void SetVBlankingStart(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 1;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(VBnkSt1, Value);
    }
    else
    {
        WriteRegToHW(VBnkSt2, Value);
    }

    return;
    
}


void SetVBlankingEnd(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 1;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(VBnkEnd1, Value);
    }
    else
    {
        WriteRegToHW(VBnkEnd2, Value);
    }

    return;
    
}


void SetVSyncStart(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 1;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(VSyncSt1, Value);
    }
    else
    {
        WriteRegToHW(VSyncSt2, Value);
    }

    return;
    
}


void SetVSyncEnd(UCHAR DisplayPath, USHORT Value)
{
    
    Value -= 1;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(VSyncEnd1, Value);
    }
    else
    {
        WriteRegToHW(VSyncEnd2, Value);
    }

    return;
    
}


void SetPixelClock(UCHAR bDisplayPath, ULONG dwClock)
{
    PLL_Info PLLInfo;
    
    PLLInfo = ClockToPLLF9003A(dwClock);
    SetDPLL(bDisplayPath, PLLInfo);
}

void SetHSource(UCHAR bDisplayPath, USHORT wValue)
{
    wValue -= 1;
    
    if(bDisplayPath == DISP1)
    {
        WriteRegToHW(HSource1, wValue);
    }
    else
    {
        WriteRegToHW(HSource2, wValue);
    }
}



PLL_Info ClockToPLLF9003A(ULONG Clock)
{
    ULONG MSCount, NSCount, RSCount, FCKVCO, FCKOUT;    
    ULONG NearestClock = 0xFFFFFFFF;
    PLL_Info PLLInfo = {0};
    
    for (MSCount = 3; MSCount < 6; MSCount++)
    {
        for (NSCount = 1; NSCount < 256; NSCount++)
        {
            FCKVCO = PLLReferenceClock * NSCount / MSCount;
            
            if ( (MaxFCKVCO9003A >= FCKVCO) && (FCKVCO >= MinFCKVCO9003A) )
            {
                for (RSCount = 1; RSCount < 6; RSCount++)
                {
                    FCKOUT = FCKVCO >> RSCount;
                    if ( Difference(FCKOUT, Clock) < Difference(NearestClock, Clock) )
                    {
                        NearestClock = FCKOUT;
                        if (MSCount == 3)
                            PLLInfo.MS = 0x00;
                        if (MSCount == 4)
                            PLLInfo.MS = BIT3;
                        if (MSCount == 5)
                            PLLInfo.MS = BIT4+BIT3;
                        PLLInfo.NS = (UCHAR)NSCount;
                        PLLInfo.RS = (UCHAR)RSCount-1;
                    }
                }
            }
        }

    }

    return PLLInfo;

}


void SetDPLL(UCHAR DisplayPath, PLL_Info PLLInfo)
{
    UCHAR RetValue; 

    if (DisplayPath == DISP1)
    {
        SetCRReg(0xC1, PLLInfo.MS, BIT4+BIT3);
        SetCRReg(0xC0, PLLInfo.NS, 0xFF);
        SetCRReg(0xCF, PLLInfo.RS, BIT2+BIT1+BIT0);
    }
    else
    {
        SetCRReg(0xBF, PLLInfo.MS, BIT4+BIT3);
        SetCRReg(0xBE, PLLInfo.NS, 0xFF);
        SetCRReg(0xCE, PLLInfo.RS, BIT2+BIT1+BIT0);
    }

    
    RetValue = GetCRReg(0xBB);
    SetCRReg(0xBB, RetValue, 0xFF);

}


void SetPolarity(UCHAR DevicePort, UCHAR Value)
{
    Value ^= BIT2+BIT1; 
    
    switch (DevicePort)
    {
        case CRT_PORT:
            OutPort(MISC_WRITE, ((Value<<5) & 0xC0) | (InPort(MISC_READ) & 0x3F));
            break;
        
        case DVP1:
        case DVP12:
            SetSRReg(0x20, Value>>1, BIT1+BIT0);
            break;

        case DVP2:
            SetSRReg(0x20, Value<<2, BIT4+BIT3);
            break;
    }
}


void SetFIFO(UCHAR DisplayPath)
{
    if (DisplayPath == DISP1)
    {
        SetCRReg(0xA7, 0x5F, 0xFF);
        SetCRReg(0xA6, 0x3F, 0xFF);
    }
    else if (DisplayPath == DISP2)
    {
        SetCRReg(0x35, 0x3F, 0xFF);
        SetCRReg(0x34, 0x3F, 0xFF);
    }
}


void SetPitch(UCHAR DisplayPath, USHORT Value)
{
    
    Value += 7;
    Value >>= 3;

    if(DisplayPath == DISP1)
    {
        WriteRegToHW(Pitch1, Value);
    }
    else
    {
        WriteRegToHW(Pitch2, Value);
    }

    return;
    
}


USHORT GetPitch(UCHAR DisplayPath)
{
    USHORT wPitch;
    
    if(DisplayPath == DISP1)
    {
        wPitch = ReadRegFromHW(Pitch1);
    }
    else
    {
        wPitch = ReadRegFromHW(Pitch2);
    }

    wPitch <<= 3;

    return wPitch;
}


USHORT GetVDisplayEnd(UCHAR DisplayPath)
{
    USHORT  wDisplayEnd = 0x0;
    
    if(DisplayPath == DISP1)
    {
        wDisplayEnd = ReadRegFromHW(VDispEnd1);
    }
    else
    {
        wDisplayEnd = ReadRegFromHW(VDispEnd2);
    }
    
    
    wDisplayEnd += 1;

    return  wDisplayEnd;
}


void SetColorDepth(UCHAR DisplayPath, UCHAR Value)
{
    UCHAR bSetBit = 0x0;

    switch(Value)
    {
        case 8:
            bSetBit = (UCHAR)BIT0;
            break;
        case 16:
            bSetBit = (UCHAR)BIT2;
            break;
        case 32:
            bSetBit = (UCHAR)BIT3;
            break;
        default:
            return;
    }

    if(DisplayPath == DISP1)
    {
        SetCRReg(0xA3, bSetBit, 0x0F);
    }
    else
    {
        
        if(Value == 8)
            return;
        
        SetCRReg(0x33, bSetBit, 0x0F);
    }
    
}


void ConfigDigitalPort(UCHAR bDisplayPath)
{
    PORT_CONFIG *pDevicePortConfig;
    UCHAR bDeviceIndex;
    UCHAR bRegValue;

    bDeviceIndex = Get_DEV_ID(bDisplayPath);

    if(bDisplayPath == DISP1)
    {
        bRegValue = 0x3;
    }
    else
    {
        bRegValue = 0x4;
    }

    if (GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig))
    {
        switch(pDevicePortConfig->PortID)
        {
            case CRT_PORT:
                SetSRReg(0x1F, bRegValue, BIT2);
                break;
                
            case DVP1:
                Set12BitDVP();
                SetSRReg(0x1F, bRegValue, BIT0);
                break;
                
            case DVP2:
                Set12BitDVP();
                SetSRReg(0x1F, bRegValue, BIT1);
                break;
                
            case DVP12:
                Set24BitDVP();
                SetSRReg(0x1F, bRegValue, BIT0);
                break;
        }
    }
}


void LoadTiming(UCHAR bDisplayPath, USHORT wModeNum)
{
    UCHAR bDeviceIndex = Get_DEV_ID(bDisplayPath);

     

    switch(bDeviceIndex)
    {
       case CRT_ID:
       case CRT2_ID:
       case DVI_ID:
       case DVI2_ID:
            LoadVESATiming(bDisplayPath, wModeNum);
            break;

       case LCD_ID:
       case LCD2_ID:
            LoadLCDTiming(bDisplayPath, wModeNum);
            break;
    }

}


void LoadVESATiming(UCHAR bDisplayPath, USHORT wModeNum)
{
    UCHAR bR_Rate_value = 0x0;
    MODE_INFO *pModeInfo = NULL;
    RRATE_TABLE *pRRateTable = NULL;
    PRINTK(KERN_INFO "==Enter LoadVESATiming()==\n");
    bR_Rate_value = Get_RRATE_ID(bDisplayPath);
    
    if(GetModePointerFromVESATable(wModeNum, bR_Rate_value, &pModeInfo, &pRRateTable))
    {
        SetTimingRegs(bDisplayPath, pModeInfo, pRRateTable);
    }
    else
    {
        PRINTK(KERN_INFO "Mode not found!!\n");
        
    }
    PRINTK(KERN_INFO "==Exit LoadVESATiming()==\n");
}

void LoadLCDTiming(UCHAR bDisplayPath, USHORT wModeNum)
{
    UCHAR bDeviceIndex = Get_DEV_ID(bDisplayPath);
    MODE_INFO *pPanelModeInfo, *pUserModeInfo;
    PANEL_TABLE *pPanelTable;

    PRINTK(KERN_INFO "==Enter LoadLCDTiming()==\n");

    if(GetModePointerFromLCDTable(bDeviceIndex,  &pPanelModeInfo, &pPanelTable))
    {
        PRINTK(KERN_INFO "&pPanelTable->Timing = 0x%x\n", (unsigned int)&pPanelTable->Timing);
        SetTimingRegs(bDisplayPath, pPanelModeInfo, &pPanelTable->Timing);
        Get_MODE_INFO(wModeNum, &pUserModeInfo);
        SetScalingFactor(bDisplayPath, pUserModeInfo, pPanelModeInfo);
    }
    else
    {
        
    }

    PRINTK(KERN_INFO "==Exit LoadLCDTiming()==\n");
}

void SetScalingFactor(UCHAR bDisplayPath, MODE_INFO *pUserModeInfo, MODE_INFO *pPanelModeInfo)
{
    ULONG dwScalingFactor;

    USHORT usUserModeHSize, usUserModeVSize;
    USHORT usPanelModeHSize, usPanelModeVSize;

    PRINTK(KERN_INFO "==Enter SetScalingFactor()==\n");
    usUserModeHSize = pUserModeInfo->H_Size;

    usUserModeVSize = pUserModeInfo->V_Size;

    usPanelModeHSize = pPanelModeInfo->H_Size;

    usPanelModeVSize = pPanelModeInfo->V_Size;


    TurnOffHorScaler(bDisplayPath);

    TurnOffVerScaler(bDisplayPath);

    SetHSource(bDisplayPath, usUserModeHSize);


    if (usPanelModeHSize > usUserModeHSize)
    {
        PRINTK(KERN_INFO "Enable H scaler\n");
        dwScalingFactor = ((ULONG)usUserModeHSize << 12) / usPanelModeHSize;
        SetHorScalingFactor(bDisplayPath, dwScalingFactor);
        TurnOnHorScaler(bDisplayPath);
        TurnOnScaler(bDisplayPath);
    }

    if (usPanelModeVSize > usUserModeVSize)
    {
        PRINTK(KERN_INFO "Enable V scaler\n");
        dwScalingFactor = ((ULONG)usUserModeVSize << 11) / usPanelModeVSize;
        SetVerScalingFactor(bDisplayPath, dwScalingFactor);
        TurnOnVerScaler(bDisplayPath);
        TurnOnScaler(bDisplayPath);
    }
    PRINTK(KERN_INFO "==Exit SetScalingFactor()==\n");
}

void SetHorScalingFactor(UCHAR bDisplayPath, USHORT wValue)
{
    if(bDisplayPath == DISP1)
    {
        WriteRegToHW(HScalingFactor1, wValue);
    }
    else
    {
        WriteRegToHW(HScalingFactor2, wValue);
    }
}

void SetVerScalingFactor(UCHAR bDisplayPath, USHORT wValue)
{
    if(bDisplayPath == DISP1)
    {
        WriteRegToHW(VScalingFactor1, wValue);
    }
    else
    {
        WriteRegToHW(VScalingFactor2, wValue);
    }
}

CI_STATUS isLCDFitMode(UCHAR bDeviceIndex, USHORT wModeNum)
{
    MODE_INFO *pModeInfo;
    
    if (Get_MODE_INFO_From_LCD_Table(bDeviceIndex, &pModeInfo))
    {
        if ((wModeNum == pModeInfo->Mode_ID_8bpp) ||
            (wModeNum == pModeInfo->Mode_ID_16bpp) ||
            (wModeNum == pModeInfo->Mode_ID_32bpp))
        {
            return ci_true;
        }
        else
        {
            return ci_false;
        }
    }
    else
    {
        return ci_false;
    }
}

CI_STATUS GetModePointerFromVESATable(USHORT wModeNum, UCHAR ucRRIndex, MODE_INFO **ppModeInfo, RRATE_TABLE **ppRRateTable)
{
    int iRRateTableIndex;

    PRINTK(KERN_INFO "==Enter GetModePointerFromVESATable()==\n");
 
    
    if(Get_MODE_INFO_From_VESA_Table(wModeNum, ppModeInfo))
    {
        
        *ppRRateTable = (RRATE_TABLE*)((int)(*ppModeInfo) + sizeof(MODE_INFO));

        for(iRRateTableIndex = 0; iRRateTableIndex < (*ppModeInfo)->RRTableCount; iRRateTableIndex++, (*ppRRateTable)++)
        {
            PRINTK(KERN_INFO "*ppRRateTable = 0x%x\n", *ppRRateTable);

            if(((*ppRRateTable)->RRate_ID == ucRRIndex) && (!((*ppRRateTable)->Attribute & DISABLE)))
            {
                PRINTK(KERN_INFO "*ppRRateTable = 0x%x\n", *ppRRateTable);
                PRINTK(KERN_INFO "  Exit1 GetModePointerFromVESATable()== return success\n");
                return ci_true;
            }
        }
    }

    
    PRINTK(KERN_INFO "  Exit2 GetModePointerFromVESATable()== return fail!!\n");
    return ci_false;
}


CI_STATUS GetModePointerFromLCDTable(UCHAR bDeviceIndex, MODE_INFO **ppModeInfo, PANEL_TABLE **ppPanelTable)
{
    PRINTK(KERN_INFO "==Enter GetModePointerFromLCDTable()==\n");
    if (Get_MODE_INFO_From_LCD_Table(bDeviceIndex, ppModeInfo))
    {
        *ppPanelTable = (PANEL_TABLE*)((int)(*ppModeInfo) + sizeof(MODE_INFO));
        PRINTK(KERN_INFO "*ppPanelTable = 0x%x\n", *ppPanelTable);
        PRINTK(KERN_INFO "==Exit1 GetModePointerFromLCDTable()== return success\n");
        return ci_true;
    }
    else
    {
        PRINTK(KERN_INFO "==Exit1 GetModePointerFromLCDTable()== return fail\n");
        return ci_false;
    }
}

CI_STATUS Get_MODE_INFO(USHORT wModeNum, MODE_INFO **ppModeInfo)
{
    if (Get_MODE_INFO_From_VESA_Table(wModeNum, ppModeInfo))
    {
        return ci_true;
    }
    else if(isLCDFitMode(LCD_ID, wModeNum))
    {
        Get_MODE_INFO_From_LCD_Table(LCD_ID, ppModeInfo);
        return ci_true;
    }
    else if(isLCDFitMode(LCD2_ID, wModeNum))
    {
        Get_MODE_INFO_From_LCD_Table(LCD2_ID, ppModeInfo);
        return ci_true;
    }
    else
    {
        return ci_false;
    }
}

CI_STATUS Get_MODE_INFO_From_LCD_Table(UCHAR bDeviceIndex, MODE_INFO **ppModeInfo)
{
    UCHAR bLCDTableIndex;
     
    PRINTK(KERN_INFO "==Enter Get_MODE_INFO_From_LCD_Table()==\n");

    *ppModeInfo = pLCDTable;
    
    if (bDeviceIndex == LCD_ID)
    {
        bLCDTableIndex = Get_LCD_TABLE_INDEX();
    }
    else if (bDeviceIndex == LCD2_ID)
    {
        bLCDTableIndex = Get_LCD2_TABLE_INDEX();
    }
    else
    {
        PRINTK(KERN_INFO "==Exit1 Get_MODE_INFO_From_LCD_Table()== return fail!!\n");
        return ci_false;
    }


    if (bLCDTableIndex == 0)
    {
        PRINTK(KERN_INFO "LCD Index = 0\n");
        PRINTK(KERN_INFO "==Exit2 Get_MODE_INFO_From_LCD_Table()== return fail!!\n");
        return ci_false;
    }

    while ((*ppModeInfo)->H_Size != 0xFFFF)
    {
        PRINTK(KERN_INFO "(*ppModeInfo)->H_Size = %d\n", (*ppModeInfo)->H_Size);

        if (bLCDTableIndex == 1)
        {
            PRINTK(KERN_INFO "==Exit3 Get_MODE_INFO_From_LCD_Table()== return success\n");
            return ci_true;
        }
        (*ppModeInfo) = (MODE_INFO*)((int)*ppModeInfo + sizeof(MODE_INFO) + sizeof(PANEL_TABLE));
        bLCDTableIndex--;
    }

    PRINTK(KERN_INFO "==Exit4 Get_MODE_INFO_From_LCD_Table()== return fail!!\n");

    return ci_false;
    
}

#if 0
CI_STATUS Get_MODE_INFO_From_VESA_Table(USHORT wModeNum, MODE_INFO **ppModeInfo)
{
    int i;
    VESA_TABLE *pVESATable = VESATable;
    
    for( i = 0; i < (sizeof(VESATable)/sizeof(VESA_TABLE)); i++, pVESATable++)
    {
        if((pVESATable->ModeInfo.Mode_ID_8bpp == wModeNum) || (pVESATable->ModeInfo.Mode_ID_16bpp == wModeNum)|| (pVESATable->ModeInfo.Mode_ID_32bpp == wModeNum))
        {
            *ppModeInfo = &(pVESATable->ModeInfo);
            return ci_true;
        }
    }
    
    return ci_false;
}
#else
CI_STATUS Get_MODE_INFO_From_VESA_Table(USHORT wModeNum, MODE_INFO **ppModeInfo)
{
    UCHAR ucColorDepth;

    PRINTK(KERN_INFO "==Enter Get_MODE_INFO_From_VESA_Table()==\n");

    *ppModeInfo = pVESATable;

    while((*ppModeInfo)->H_Size != 0xFFFF)
    {
        PRINTK(KERN_INFO "*ppModeInfo = 0x%x\n", *ppModeInfo);
        PRINTK(KERN_INFO "(*ppModeInfo)->H_Size = %d\n", (*ppModeInfo)->H_Size);

        if (GetModeColorDepth(wModeNum, *ppModeInfo, &ucColorDepth))
        {
            PRINTK(KERN_INFO "*ppModeInfo = 0x%x\n", *ppModeInfo);
            PRINTK(KERN_INFO "  Exit1 Get_MODE_INFO_From_VESA_Table()== return success\n");
            return ci_true;
        }

        *ppModeInfo = (MODE_INFO*)((*ppModeInfo)->RRTableCount * sizeof(RRATE_TABLE) + sizeof(MODE_INFO) + (void*)(*ppModeInfo));
    }

    *ppModeInfo = (MODE_INFO*)(&CInt10VESATable);

    while((*ppModeInfo)->H_Size != 0xFFFF)
    {
        PRINTK(KERN_INFO "*ppModeInfo = 0x%x\n", *ppModeInfo);
        PRINTK(KERN_INFO "(*ppModeInfo)->H_Size = %d\n", (*ppModeInfo)->H_Size);

        if (GetModeColorDepth(wModeNum, *ppModeInfo, &ucColorDepth))
        {
            PRINTK(KERN_INFO "*ppModeInfo = 0x%x\n", *ppModeInfo);
            PRINTK(KERN_INFO "  Exit1 Get_MODE_INFO_From_VESA_Table()== return success\n");
            return ci_true;
        }

        *ppModeInfo = (MODE_INFO*)((*ppModeInfo)->RRTableCount * sizeof(RRATE_TABLE) + sizeof(MODE_INFO) + (void*)(*ppModeInfo));
    }

    PRINTK(KERN_INFO "  Exit2 Get_MODE_INFO_From_VESA_Table()== return fail!!\n");
    return ci_false;
}
#endif

CI_STATUS GetModeColorDepth(USHORT wModeNum, MODE_INFO *pModeInfo, UCHAR *pucColorDepth)
{
    PRINTK(KERN_INFO "==Enter GetModeColorDepth()==\n");
    PRINTK(KERN_INFO "pModeInfo->Mode_ID_8bpp = 0x%x\n", pModeInfo->Mode_ID_8bpp);
    PRINTK(KERN_INFO "pModeInfo->Mode_ID_16bpp = 0x%x\n", pModeInfo->Mode_ID_16bpp);
    PRINTK(KERN_INFO "pModeInfo->Mode_ID_32bpp = 0x%x\n", pModeInfo->Mode_ID_32bpp);

    if(pModeInfo->Mode_ID_8bpp == wModeNum)
    {
        *pucColorDepth = 8;

        PRINTK(KERN_INFO "==Exit1 GetModeColorDepth()== *pucColorDepth = %d\n", *pucColorDepth);

        return ci_true;
    }
    else if(pModeInfo->Mode_ID_16bpp == wModeNum)
    {
        *pucColorDepth = 16;

        PRINTK(KERN_INFO "==Exit2 GetModeColorDepth()== *pucColorDepth = %d\n", *pucColorDepth);

        return ci_true;
    }
    else if(pModeInfo->Mode_ID_32bpp == wModeNum)
    {
        *pucColorDepth = 32;
        PRINTK(KERN_INFO "==Exit3 GetModeColorDepth()== *pucColorDepth = %d\n", *pucColorDepth);
        return ci_true;
    }
    else
    {
        *pucColorDepth = 0;
        PRINTK(KERN_INFO "==Exit4 GetModeColorDepth()== *pucColorDepth = %d\n", *pucColorDepth);
        return ci_false;
    }    
}

CI_STATUS GetModePitch(USHORT ModeNum, USHORT *pPitch)
{
    MODE_INFO *pModeInfo = NULL;
    UCHAR ucColorDepth = 0;
    
    
    if(!Get_MODE_INFO(ModeNum, &pModeInfo))
    {
        return ci_false;
    }
    else
    {
        if(!GetModeColorDepth(ModeNum, pModeInfo, &ucColorDepth))
        {
            return ci_false;
        }
        else
        {
            ucColorDepth = ucColorDepth >> 4;
            *pPitch = ((pModeInfo->H_Size << ucColorDepth)+0x7)&0xFFF8;
        }
    }

    return ci_true;
}

USHORT ReadRegFromHW(REG_OP *pRegOp)
{
    USHORT wValue = 0x0;
    UCHAR    btemp = 0x0, bMasktemp = 0x0;

    while((pRegOp->RegGroup)!= NR)
    {        
        if(pRegOp->RegGroup == CR)
        {
            
            OutPort(COLOR_CRTC_INDEX,(pRegOp->RegIndex));
            btemp = (UCHAR)InPort(COLOR_CRTC_DATA);
        }
        else
        {
            
            OutPort(SEQ_INDEX,(pRegOp->RegIndex));
            btemp = (UCHAR)InPort(SEQ_DATA);
        }
        
        bMasktemp = (pRegOp->RegMask);

        
        btemp &= bMasktemp;

        
        while(!(bMasktemp & BIT0))
        {
            bMasktemp = bMasktemp >> 1;
            btemp = btemp >> 1;
        }

        
        wValue |= (((USHORT)btemp) << (pRegOp->RegShiftBit));
        
        
        pRegOp++;

    }

    return wValue;
}


void WriteRegToHW(REG_OP *pRegOp, USHORT value)
{
    UCHAR btemp, btemp1;
    UCHAR bCount;
    UCHAR bMasktemp;

    while((pRegOp->RegGroup)!= NR)
    {
        btemp = 0x0; btemp1 = 0x0;
        bCount = 0x0; bMasktemp = 0x0;
    
        bMasktemp = (pRegOp->RegMask);

        
        while(!(bMasktemp & BIT0))
        {
            bMasktemp = bMasktemp >> 1;
            bCount ++;
        }
    
        
        btemp = value >> (pRegOp->RegShiftBit);
        btemp &= (bMasktemp);

        
        if(!(pRegOp->RegMask & BIT0))
        {
            btemp = btemp << bCount;
        }
            
        if(pRegOp->RegGroup == CR)
        {
            
            OutPort(COLOR_CRTC_INDEX,(pRegOp->RegIndex));
            btemp1 = (UCHAR)InPort(COLOR_CRTC_DATA);
            btemp1 &= ~(pRegOp->RegMask);
            btemp1 |= btemp;
            OutPort(COLOR_CRTC_DATA,btemp1);
        }
        else
        {
            
            OutPort(SEQ_INDEX,(pRegOp->RegIndex));
            btemp1 = (UCHAR)InPort(SEQ_DATA);
            btemp1 &= ~(pRegOp->RegMask);
            btemp1 |= btemp;
            OutPort(SEQ_DATA,btemp1);            
        }

        pRegOp++;
        
    }
 
}


void UnLockCR0ToCR7()
{
    
    SetCRReg(0x11, 0x00, BIT7);
}


void LockCR0ToCR7()
{
    
    SetCRReg(0x11, 0x80, BIT7);
}


CI_STATUS CheckForModeAvailable(USHORT ModeNum)
{
    MODE_INFO *pModeInfo = NULL;
    return Get_MODE_INFO_From_VESA_Table(ModeNum, &pModeInfo);
}

CI_STATUS CheckForNewDeviceAvailable(UCHAR bDeviceIndex)
{
    PORT_CONFIG *pDevicePortConfig;
    
    return GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig);
}

void Display1TurnOnTX()
{

}

void Display1TurnOffTX()
{

}

void Display2TurnOnTX()
{

}

void Display2TurnOffTX()
{

}


void TurnOnDigitalPort(UCHAR bDeviceIndex)
{
    PORT_CONFIG *pDevicePortConfig;

    if (GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig))
    {
        switch(pDevicePortConfig->PortID)
        {
            case CRT_PORT:
                TurnOnDAC();
                TurnOnCRTPad();
                break;
                
            case DVP1:
                TurnOnDVP1Pad();
                break;
                
            case DVP2:
                TurnOnDVP2Pad();
                break;
                
            case DVP12:
                TurnOnDVP12Pad();
                break;
        }
    }
}       

void TurnOffDigitalPort(UCHAR bDeviceIndex)
{
    PORT_CONFIG *pDevicePortConfig;
    
    if (GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig))
    
    switch(pDevicePortConfig->PortID)
    {
        case CRT_PORT:
            TurnOffCRTPad();
//            TurnOffDAC();
            break;
            
        case DVP1:
            TurnOffDVP1Pad();
            break;
            
        case DVP2:
            TurnOffDVP2Pad();
            break;
            
        case DVP12:
            TurnOffDVP12Pad();
            break;
    }
}       


UCHAR GetPortConnectPath(UCHAR PortType)
{
    UCHAR SR1F, PortMask = 0;

    SR1F = GetSRReg(0x1F);
    SR1F ^= 0x03;
    
    switch(PortType)
    {
        case CRT_PORT:
            PortMask = BIT2;
            break;
            
        case DVP1:
        case DVP12:
            PortMask = BIT0;
            break;
            
        case DVP2:
            PortMask = BIT1;
            break;
    }
    
    return ((SR1F & PortMask) ? 1 : 0);

}


USHORT TransDevIDtoBit(UCHAR DeviceIndex)
{
    return (1 << (DeviceIndex - 1));
}

void TurnOnCRTPad()
{
    SetCRReg(0xA8, 0x00, BIT7);
}

void TurnOffCRTPad()
{
    SetCRReg(0xA8, 0x80, BIT7);
}

void TurnOnDVP1Pad()
{
    SetCRReg(0xA3, 0x80, BIT7);
}

void TurnOffDVP1Pad()
{
    SetCRReg(0xA3, 0x00, BIT7);
}

void TurnOnDVP2Pad()
{
    SetCRReg(0xA3, 0x40, BIT6);
}

void TurnOffDVP2Pad()
{
    SetCRReg(0xA3, 0x00, BIT6);
}

void TurnOnDVP12Pad()
{
    TurnOnDVP1Pad();
    TurnOnDVP2Pad();
}

void TurnOffDVP12Pad()
{
    TurnOffDVP1Pad();
    TurnOffDVP2Pad();
}

void TurnOnScaler(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
        SetSRReg(0x58, BIT0, BIT0);
    else
        SetSRReg(0x50, BIT0, BIT0);
}

void TurnOffScaler(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
        SetSRReg(0x58, 0, BIT0);
    else
        SetSRReg(0x50, 0, BIT0);
}

void TurnOnHorScaler(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
        SetSRReg(0x58, BIT2, BIT2);
    else
        SetSRReg(0x50, BIT2, BIT2);
}

void TurnOffHorScaler(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
        SetSRReg(0x58, 0, BIT2);
    else
        SetSRReg(0x50, 0, BIT2);
}

void TurnOnVerScaler(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
        SetSRReg(0x58, BIT1, BIT1);
    else
        SetSRReg(0x50, BIT1, BIT1);
}

void TurnOffVerScaler(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
        SetSRReg(0x58, 0, BIT1);
    else
        SetSRReg(0x50, 0, BIT1);
}


void Set12BitDVP()
{
    SetSRReg(0x1E, 0x00, BIT3);
}

void Set24BitDVP()
{
    SetSRReg(0x1E, 0x08, BIT3);
}

void TurnOnDAC()
{
    SetCRReg(0xDF, 0x00, BIT2);
}
void TurnOffDAC()
{
    SetCRReg(0xDF, 0x04, BIT2);
}

#if 0
void Display1HWResetOn()
{
    SetCRReg(0x17, 0x00, BIT7);
}
void Display1HWResetOff()
{
    SetCRReg(0x17, 0x80, BIT7);
}

void Display2HWResetOn()
{
    SetCRReg(0x33, 0x00, BIT4);
}
void Display2HWResetOn()
{
    SetCRReg(0x33, 0x10, BIT4);
}
#endif


void SerialLoadTable(UCHAR **ppucTablePointer, UCHAR ucI2Cport, UCHAR ucI2CAddr)
{
    UCHAR ucRegGroup;

    while (**ppucTablePointer != 0xFF)
    {
        ucRegGroup = **ppucTablePointer;
        (*ppucTablePointer)++;
        if (ucRegGroup & BITS)
        {
            SerialLoadRegBits(ppucTablePointer, ucRegGroup, 0, 0);
        }
        else
        {
            SerialLoadReg(ppucTablePointer, ucRegGroup, 0, 0);
        }
    }
    (*ppucTablePointer)++;
}


void SerialLoadRegBits(UCHAR **ppucTablePointer, UCHAR ucRegGroup, UCHAR ucI2Cport, UCHAR ucI2CAddr)
{
    UCHAR ucRegIndex, ucRegValue, ucMask;
    ucRegGroup &= 0x7F;

    while (**ppucTablePointer != 0xFF)
    {
        ucRegIndex = **ppucTablePointer;
        (*ppucTablePointer)++;

        ucRegValue = **ppucTablePointer;
        (*ppucTablePointer)++;

        ucMask = **ppucTablePointer;
        (*ppucTablePointer)++;

        switch(ucRegGroup)
        {
            case SR:
                SetSRReg(ucRegIndex, ucRegValue, ucMask);
                break;

            case CR:
                SetCRReg(ucRegIndex, ucRegValue, ucMask);
                break;
                
            case I2C:
                break;
        }
    }
    (*ppucTablePointer)++;
}


void SerialLoadReg(UCHAR **ppucTablePointer, UCHAR ucRegGroup, UCHAR ucI2Cport, UCHAR ucI2CAddr)
{
    UCHAR ucRegIndex, ucRegValue;

    while (**ppucTablePointer != 0xFF)
    {
        ucRegIndex = **ppucTablePointer;
        (*ppucTablePointer)++;

        ucRegValue = **ppucTablePointer;
        (*ppucTablePointer)++;

        switch(ucRegGroup)
        {
            case SR:
                SetSRReg(ucRegIndex, ucRegValue, 0xFF);
                break;

            case CR:
                SetCRReg(ucRegIndex, ucRegValue, 0xFF);
                break;
                
            case GR:
                SetGRReg(ucRegIndex, ucRegValue, 0xFF);
                break;

            case I2C:
                break;
        }
    }
    (*ppucTablePointer)++;
}


void LoadDisplay1VESAModeInitRegs()
{
    UCHAR *pucDisplay1VESAModeInitRegs = (UCHAR*)Display1VESAModeInitRegs;
    
    OutPort(MISC_WRITE, 0x2F);

    UnLockCR0ToCR7();
    
    PRINTK(KERN_INFO "&pucDisplay1VESAModeInitRegs = 0x%x\n", &pucDisplay1VESAModeInitRegs);
    SerialLoadTable(&pucDisplay1VESAModeInitRegs, 0, 0);

    ResetATTR();
    
    SetARReg(0x10, *pucDisplay1VESAModeInitRegs);
    pucDisplay1VESAModeInitRegs++;
    SetARReg(0x11, *pucDisplay1VESAModeInitRegs);
    pucDisplay1VESAModeInitRegs++;
    SetARReg(0x12, *pucDisplay1VESAModeInitRegs);
    pucDisplay1VESAModeInitRegs++;
    SetARReg(0x13, *pucDisplay1VESAModeInitRegs);
    pucDisplay1VESAModeInitRegs++;
    SetARReg(0x14, *pucDisplay1VESAModeInitRegs);
    
    EnableATTR();
}

void VESASetBIOSData(USHORT ModeNum)
{
    
}


void Disabl_EDIsplayPathAndDevice(UCHAR bDisplayPath)
{
    UCHAR bDeviceIndex = Get_DEV_ID(bDisplayPath);

    
    ControlPwrSeqOff(bDeviceIndex);

    

    
    TurnOffDigitalPort(bDeviceIndex);
    
    
    SequencerOff(bDisplayPath);
}

CI_STATUS GetDevicePortConfig(UCHAR bDeviceIndex, PORT_CONFIG **ppDevicePortConfig)
{
    PRINTK(KERN_INFO "==Enter GetDevicePortConfig()==\n");

    *ppDevicePortConfig = pPortConfig;
    
    while(((*ppDevicePortConfig)->DevID != 0xFF) && ((*ppDevicePortConfig)->Attribute & Dev_SUPPORT))
    {
        PRINTK(KERN_INFO "(*ppDevicePortConfig)->DevID = %x\n", (*ppDevicePortConfig)->DevID);
        
        if (bDeviceIndex == (*ppDevicePortConfig)->DevID)
        {
            PRINTK(KERN_INFO "  Exit1 GetDevicePortConfig()== return success\n");
            return ci_true;
        }
        (*ppDevicePortConfig)++;
    }

    PRINTK(KERN_INFO "  Exit1 GetDevicePortConfig()== return fail!!\n");
    return ci_false;
}

void PowerSequenceOn()
{
    
    if (!(GetSRReg(0x11) & BIT0))
    {
        
        SetSRReg(0x32, BIT1, BIT1);
        
        
        SetSRReg(0x11, BIT0, BIT0);
        
        
        WaitPowerSequenceDone();
    }
}

void PowerSequenceOff()
{
    
    if (GetSRReg(0x11) & BIT0)
    {
        
        SetSRReg(0x11, 0, BIT0);
        
        
        WaitPowerSequenceDone();
        
        
        SetSRReg(0x32, 0, BIT1);
    }
}

void SequencerOn(UCHAR DisplayPath)
{
    if (DisplayPath == DISP1)
        SetSRReg(0x01, 0, BIT5);
    else
        SetCRReg(0x33, 0, BIT0);
}

void SequencerOff(UCHAR bDisplayPath)
{
    if (bDisplayPath == DISP1)
    {
        if (!(GetSRReg(0x01) & BIT5))
        {
            LongWait();
        }
        SetSRReg(0x01, BIT5, BIT5);
    }
    else
        SetCRReg(0x33, BIT0, BIT0);
}

void ControlPwrSeqOn(UCHAR bDeviceIndex)
{
    PORT_CONFIG *pDevicePortConfig;
    
    if ((TransDevIDtoBit(bDeviceIndex) & (B_LCD+B_LCD2)) && (GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig)))
    {
        if (pDevicePortConfig->Attribute & TX_PS)
        {
            if (pDevicePortConfig->TX_Enc_ID == TX_VT1636)
            {

            }
        }
        else if ((pDevicePortConfig->PortID == DVP1) || (pDevicePortConfig->PortID == DVP12))
        {
            PowerSequenceOn();
        }
    }    
}

void ControlPwrSeqOff(UCHAR bDeviceIndex)
{
    PORT_CONFIG *pDevicePortConfig;
    
    if ((TransDevIDtoBit(bDeviceIndex) & (B_LCD+B_LCD2)) && (GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig)))
    {
        if (pDevicePortConfig->Attribute & TX_PS)
        {
            if (pDevicePortConfig->TX_Enc_ID == TX_VT1636)
            {

            }
        }
        else if ((pDevicePortConfig->PortID == DVP1) || (pDevicePortConfig->PortID == DVP12))
        {
            PowerSequenceOff();
        }
    }    
}

void LongWait()
{
     while (GetIS1Reg() & BIT3);
     
     while (!(GetIS1Reg() & BIT3));
}

UCHAR Get_DEV_ID(UCHAR DisplayPath)
{
    return ((DisplayPath == DISP1) ? ReadScratch(IDX_IGA1_DEV_ID) : ReadScratch(IDX_IGA2_DEV_ID));
}


void Set_DEV_ID(UCHAR DeviceID, UCHAR DisplayPath)
{
    if (DisplayPath == DISP1)
        WriteScratch(IDX_IGA1_DEV_ID, DeviceID);
    else
        WriteScratch(IDX_IGA2_DEV_ID, DeviceID);
}


UCHAR Get_NEW_DEV_ID(UCHAR DisplayPath)
{
    return ((DisplayPath == DISP1) ? ReadScratch(IDX_NEW_IGA1_DEV_ID) : ReadScratch(IDX_NEW_IGA2_DEV_ID));
}

void Set_NEW_DEV_ID(UCHAR DeviceID, UCHAR DisplayPath)
{
    if (DisplayPath == DISP1)
        WriteScratch(IDX_NEW_IGA1_DEV_ID, DeviceID);
    else
        WriteScratch(IDX_NEW_IGA2_DEV_ID, DeviceID);
}

USHORT Get_LCD_H_SIZE()
{
    USHORT wValue;

    wValue = (USHORT)ReadScratch(IDX_LCD_H_SIZE_OVERFLOW);
    wValue <<= 8;
    wValue |= (USHORT)ReadScratch(IDX_LCD_H_SIZE);
    
    return wValue;
}

USHORT Get_LCD_V_SIZE()
{
    USHORT wValue;

    wValue = (USHORT)ReadScratch(IDX_LCD_V_SIZE_OVERFLOW);
    wValue <<= 8;
    wValue |= (USHORT)ReadScratch(IDX_LCD_V_SIZE);
    
    return wValue;
}

ULONG Get_LCD_SIZE()
{
    ULONG dwValue;

    dwValue = (ULONG)Get_LCD_V_SIZE();
    dwValue <<= 16;
    dwValue |= (ULONG)Get_LCD_H_SIZE();
    
    return dwValue;
}

USHORT Get_LCD2_H_SIZE()
{
    USHORT wValue;

    wValue = (USHORT)ReadScratch(IDX_LCD2_H_SIZE_OVERFLOW);
    wValue <<= 8;
    wValue |= (USHORT)ReadScratch(IDX_LCD2_H_SIZE);
    
    return wValue;
}

USHORT Get_LCD2_V_SIZE()
{
    USHORT wValue;

    wValue = (USHORT)ReadScratch(IDX_LCD2_V_SIZE_OVERFLOW);
    wValue <<= 8;
    wValue |= (USHORT)ReadScratch(IDX_LCD2_V_SIZE);
    
    return wValue;
}

ULONG Get_LCD2_SIZE()
{
    ULONG dwValue;

    dwValue = (ULONG)Get_LCD2_V_SIZE();
    dwValue <<= 16;
    dwValue |= (ULONG)Get_LCD2_H_SIZE();
    
    return dwValue;
}

UCHAR Get_RRATE_ID(UCHAR DisplayPath)
{
    return ((DisplayPath == DISP1) ? ReadScratch(IDX_IGA1_RRATE_ID) : ReadScratch(IDX_IGA2_RRATE_ID));
}

void Set_RRATE_ID(UCHAR RRateID, UCHAR DisplayPath)
{
    if (DisplayPath == DISP1)
        WriteScratch(IDX_IGA1_RRATE_ID, RRateID);
    else
        WriteScratch(IDX_IGA2_RRATE_ID, RRateID);
}

UCHAR Get_LCD_TABLE_INDEX(void)
{
    return ReadScratch(IDX_LCD1_TABLE_INDEX);
}

UCHAR Get_LCD2_TABLE_INDEX(void)
{
    return ReadScratch(IDX_LCD2_TABLE_INDEX);
}

void Set_LCD_TABLE_INDEX(UCHAR bLCDIndex)
{
    WriteScratch(IDX_LCD1_TABLE_INDEX, bLCDIndex);
}


USHORT Get_VESA_MODE(UCHAR DisplayPath)
{
    UCHAR VESAMode, VESAModeOver;

    if (DisplayPath == DISP1)
    {
        VESAMode = ReadScratch(IDX_IGA1_VESA_MODE);
        VESAModeOver = ReadScratch(IDX_IGA1_VESA_MODE_OVERFLOW);
    }
    else
    {
        VESAMode = ReadScratch(IDX_IGA2_VESA_MODE);
        VESAModeOver = ReadScratch(IDX_IGA2_VESA_MODE_OVERFLOW);
    }
    
    return  ((USHORT)VESAModeOver) << 8 | (USHORT)VESAMode;
}

void Set_VESA_MODE(USHORT ModeNum, UCHAR DisplayPath)
{
    if (DisplayPath == DISP1)
    {
        WriteScratch(IDX_IGA1_VESA_MODE, (UCHAR)ModeNum);
        WriteScratch(IDX_IGA1_VESA_MODE_OVERFLOW, (UCHAR)(ModeNum >> 8));
    }
    else
    {
        WriteScratch(IDX_IGA2_VESA_MODE, (UCHAR)ModeNum);
        WriteScratch(IDX_IGA2_VESA_MODE_OVERFLOW, (UCHAR)(ModeNum >> 8));
    }
}

void ResetATTR()
{
    InPort(COLOR_INPUT_STATUS1_READ);
    InPort(MONO_INPUT_STATUS1_READ);
}

void EnableATTR()
{
    ResetATTR();
    OutPort(ATTR_DATA_WRITE, 0x20);
}


void SetCRReg(UCHAR bRegIndex, UCHAR bRegValue, UCHAR bMask)
{
    UCHAR btemp = 0x0;
    
    if(bMask != 0xFF)
    {
        OutPort(COLOR_CRTC_INDEX,bRegIndex);
        btemp = (UCHAR)InPort(COLOR_CRTC_DATA);
        bRegValue &= bMask;
        btemp &=~(bMask);
        btemp |= bRegValue;
        OutPort(COLOR_CRTC_DATA,btemp);
    }
    else
    {
        OutPort(COLOR_CRTC_INDEX,bRegIndex);
        OutPort(COLOR_CRTC_DATA,bRegValue);
    }

    return;
}


UCHAR GetCRReg(UCHAR bRegIndex)
{
    UCHAR btemp = 0x0;
    
    OutPort(COLOR_CRTC_INDEX,bRegIndex);
    btemp = (UCHAR)InPort(COLOR_CRTC_DATA);
        
    return btemp;
}


void SetSRReg(UCHAR bRegIndex, UCHAR bRegValue, UCHAR bMask)
{
    UCHAR btemp = 0x0;
    
    if(bMask != 0xFF)
    {
        OutPort(SEQ_INDEX,bRegIndex);
        btemp = (UCHAR)InPort(SEQ_DATA);
        bRegValue &= bMask;
        btemp &=~(bMask);
        btemp |= bRegValue;
        OutPort(SEQ_DATA,btemp);
    }
    else
    {
        OutPort(SEQ_INDEX,bRegIndex);
        OutPort(SEQ_DATA,bRegValue);
    }

    return;
}


UCHAR GetSRReg(UCHAR bRegIndex)
{
    UCHAR btemp = 0x0;
    
    OutPort(SEQ_INDEX,bRegIndex);
    btemp = (UCHAR)InPort(SEQ_DATA);
        
    return btemp;
}

void SetARReg(UCHAR index,UCHAR value)
{
    OutPort(ATTR_DATA_WRITE,index);
    OutPort(ATTR_DATA_WRITE,value);
}

UCHAR GetARReg(UCHAR index)
{
    UCHAR bTmp;
    InPort(COLOR_INPUT_STATUS1_READ);
    OutPort(ATTR_DATA_WRITE,index);
    bTmp = (UCHAR)InPort(ATTR_DATA_READ);
    InPort(COLOR_INPUT_STATUS1_READ);
    OutPort(ATTR_DATA_WRITE,BIT5);
    return bTmp;
}


void SetGRReg(UCHAR bRegIndex, UCHAR bRegValue, UCHAR bMask)
{
    UCHAR btemp = 0x0;
    
    if(bMask != 0xFF)
    {
        OutPort(GRAPH_INDEX,bRegIndex);
        btemp = (UCHAR)InPort(GRAPH_DATA);
        bRegValue &= bMask;
        btemp &=~(bMask);
        btemp |= bRegValue;
        OutPort(GRAPH_DATA,btemp);
    }
    else
    {
        OutPort(GRAPH_INDEX,bRegIndex);
        OutPort(GRAPH_DATA,bRegValue);
    }

    return;
}

void SetMSReg(UCHAR bRegValue)
{
    OutPort(MISC_WRITE,bRegValue);
}

UCHAR GetIS1Reg()
{
    return InPort(COLOR_INPUT_STATUS1_READ);
}


void ClearFrameBuffer(UCHAR DisplayPath,ULONG *pFrameBufferBase, MODE_INFO *pModeInfo, UCHAR ucColorDepth)
{
    ULONG dwWidth = (ULONG)pModeInfo->H_Size, dwHeight = (ULONG)pModeInfo->V_Size, dwFactor  = 0;
    ULONG i = 0;

    PRINTK(KERN_INFO "==Enter ClearFrameBuffer()==\n");
    
    
    
    switch(ucColorDepth)
    {
        case 8:
        case 16:
        case 32:    
          dwFactor = 32 / ucColorDepth;
          break;
          
        default:
            return;
    }

    
    for(i = 0;i<((dwWidth*dwHeight)/dwFactor);i++)
    {
        *(pFrameBufferBase+i) = 0x00000000;
    }
    PRINTK(KERN_INFO "==Exit ClearFrameBuffer()==\n");
    
}

ULONG Difference(ULONG Value1, ULONG Value2)
{
    if (Value1 > Value2)
        return (Value1 - Value2);
    else
        return (Value2 - Value1);
}


UCHAR ReadScratch(USHORT IndexMask)
{
    UCHAR Index = (UCHAR)(IndexMask >> 8);
    UCHAR Mask = (UCHAR)IndexMask;
    UCHAR RetValue;

    RetValue = GetCRReg(Index);
    RetValue &= Mask;
    
    RetValue = AlignDataToLSB(RetValue, Mask);

    return RetValue;
}


void WriteScratch(USHORT IndexMask, UCHAR Data)
{
    UCHAR Index = (UCHAR)(IndexMask >> 8);
    UCHAR Mask = (UCHAR)IndexMask;

    Data = AlignDataToMask(Data, Mask);
    Data &= Mask;
    SetCRReg(Index, Data, Mask);
}


UCHAR AlignDataToLSB(UCHAR bData, UCHAR bMask)
{
    bData &= bMask;
    
    while ((bMask & BIT0) == 0)
    {
        bData >>= 1;
        bMask >>= 1;
    }
    
    return bData;
}


UCHAR AlignDataToMask(UCHAR bData, UCHAR bMask)
{
    while ((bMask & BIT0) == 0)
    {
        bData <<= 1;
        bMask >>= 1;
    }
    
    return bData;
}


void SetDPMS(UCHAR DPMSState, UCHAR DisplayPath)
{
    UCHAR RegValue = 0;

    if (DPMSState > DPMS__OFF)
        RegValue = 3;
    else if (DPMSState <= DPMS__SUSPEND)
        RegValue = DPMSState;
        
    if (DisplayPath == DISP1)
        SetCRReg(0xB6, RegValue, BIT1+BIT0);
    else if (DisplayPath == DISP2)
        SetCRReg(0x3E, RegValue, BIT1+BIT0);

}

CI_STATUS DetectMonitor()
{
    CI_STATUS ConnectStatus;

    
    SetCRReg(0xA9, 0x80, 0xFF);

    
    SetCRReg(0xA8, BIT6, BIT6);

    
    WaitDisplayPeriod();

    
    ConnectStatus = ((InPort(INPUT_STATUS_0_READ) & BIT4) ? ci_true : ci_false);

    SetCRReg(0xA8, 0x00, BIT6);

    return ConnectStatus;
}

void WaitDisplayPeriod()
{
     while ((InPort(COLOR_INPUT_STATUS1_READ)&BIT0) == 1);
     
     while ((InPort(COLOR_INPUT_STATUS1_READ)&BIT0) == 0);
}


void WaitPowerSequenceDone()
{
    UCHAR SR32;
     
    SR32 = GetSRReg(0x32);

    
    while(SR32 == GetSRReg(0x32));
}

CI_STATUS CheckForDSTNPanel(UCHAR bDeviceIndex)
{
    PORT_CONFIG *pDevicePortConfig;
    
    if (GetDevicePortConfig(bDeviceIndex, &pDevicePortConfig))
    {
        if ((pDevicePortConfig->TX_Enc_ID == DSTN) && 
            ((pDevicePortConfig->PortID == DVP1) || (pDevicePortConfig->PortID == DVP12)))
        {
            return ci_true;
        }
        else
        {
            return ci_false;
        }
    }
    else
    {
        return ci_false;
    }
}

USHORT GetVESAMEMSize()
{
    UCHAR bHWStrapping;
    bHWStrapping = (GetCRReg(0xAA) & (BIT2+BIT1+BIT0));
    
    return (2 << (bHWStrapping+3));
}

void SetDeviceSupport()
{
    PORT_CONFIG *pDevicePortConfig = pPortConfig;
    
    if (GetDevicePortConfig(CRT_ID, &pDevicePortConfig))
    {
        PRINTK(KERN_INFO "CRT supported\n");
        bCRTSUPPORT = ci_true;
    }
    
    if (GetDevicePortConfig(LCD_ID, &pDevicePortConfig))
    {
        PRINTK(KERN_INFO "LCD supported\n");
        bLCDSUPPORT = ci_true;
    }

    if (GetDevicePortConfig(DVI_ID, &pDevicePortConfig))
    {
        PRINTK(KERN_INFO "DVI supported\n");
        bDVISUPPORT = ci_true;
    }
    
    if (GetDevicePortConfig(TV_ID, &pDevicePortConfig))
    {
        PRINTK(KERN_INFO "TV supported\n");
        bTVSUPPORT = ci_true;
    }
}

CI_STATUS VBE_SetMode(CBIOS_Extension *pCBIOSExtension)
{
    USHORT    wModeNum = pCBIOSExtension->CBiosArguments.reg.x.BX & 0x01FF;
    USHORT    wPitch = 0;
    MODE_INFO   *pModeInfo = NULL;
    UCHAR    bColorDepth = 0;
    UCHAR    bCurDeviceID, bNewDeviceID;

    PRINTK(KERN_INFO "==Entry VBE_SetMode Mode number 0x%x== \n",wModeNum);

    if (wModeNum < 0x100)
    {
        SetVBERerurnStatus(VBEFunctionCallFail, &pCBIOSExtension->CBiosArguments);
        return ci_true;
    }

    bCurDeviceID = Get_DEV_ID(DISP1);
    bNewDeviceID = Get_NEW_DEV_ID(DISP1);

    if(!Get_MODE_INFO(wModeNum, &pModeInfo))
    {
        SetVBERerurnStatus(VBEFunctionCallFail, &pCBIOSExtension->CBiosArguments);
        PRINTK(KERN_INFO "==Exit1 VBE_SetMode return 0x%x== \n",VBEFunctionCallFail);
        return ci_true;
    }
    
    Set_VESA_MODE(wModeNum, DISP1);
    
    
    SequencerOff(DISP1);

    
    TurnOffScaler(DISP1);
    
    if (bCurDeviceID != bNewDeviceID)
    {
        
        ControlPwrSeqOff(bCurDeviceID);

        

        
        TurnOffDigitalPort(bCurDeviceID);

        
        Set_DEV_ID(bNewDeviceID, DISP1);
    }

    

    
    LoadDisplay1VESAModeInitRegs();

    
    LoadTiming(DISP1, wModeNum);

    
    GetModePitch(wModeNum, &wPitch);
    SetPitch(DISP1, wPitch);

    
    PRINTK(KERN_INFO " \n");
    Get_MODE_INFO(wModeNum, &pModeInfo);
    GetModeColorDepth(wModeNum, pModeInfo, &bColorDepth);
    SetColorDepth(DISP1, bColorDepth);

    

    

    
    PRINTK(KERN_INFO " \n");
    if(!(pCBIOSExtension->CBiosArguments.reg.x.BX & BIT15))
    {
        ClearFrameBuffer(DISP1,(ULONG*)(pCBIOSExtension->VideoVirtualAddress),pModeInfo,bColorDepth);
    }

    
    PRINTK(KERN_INFO " \n");
    SetFIFO(DISP1);

    
    PRINTK(KERN_INFO " \n");
    ConfigDigitalPort(DISP1);

    TurnOnDigitalPort(bNewDeviceID);

    

    
    PRINTK(KERN_INFO " \n");
    ControlPwrSeqOn(bNewDeviceID);

    
    PRINTK(KERN_INFO " \n");
    SequencerOn(DISP1);
    
    SetVBERerurnStatus(VBEFunctionCallSuccessful, &pCBIOSExtension->CBiosArguments);
    
    PRINTK(KERN_INFO "==Exit2 VBE_SetMode return 0x%x== \n",VBEFunctionCallSuccessful);

    return ci_true;
}

CI_STATUS VBE_SetGetScanLineLength (CBIOS_ARGUMENTS *pCBiosArguments)
{
    USHORT    wModeNum;
    MODE_INFO   *pModeInfo = NULL;
    UCHAR    bColorDepth;
    UCHAR    bDispalyPath;
    ULONG   dwVESAMemSizeInBytes;
    USHORT    wMaxPitchInBytes, wCurrentVDispEnd;
    USHORT    VBEReturnStatus = VBEFunctionCallFail;
    USHORT    wPitchToBeSet = pCBiosArguments->reg.x.CX; 
    

    PRINTK(KERN_INFO "==Entry VBE_SetGetScanLineLength== \n");

    if (pCBiosArguments->reg.x.AX == 0x4f06)
    {
        bDispalyPath = DISP1;
    }
    else if ((pCBiosArguments->reg.x.AX == 0x4f14) && ((pCBiosArguments->reg.lh.BH == 0x87)||(pCBiosArguments->reg.lh.BH == 0x08)))
    {
        bDispalyPath = DISP2;
    }
    else
    {
        SetVBERerurnStatus(VBEFunctionCallFail, pCBiosArguments);

        PRINTK(KERN_INFO "==Exit1 VBE_SetGetScanLineLength return 0x%x== \n",VBEFunctionCallFail);

        return ci_true;
    }
    
    if (pCBiosArguments->reg.lh.BL <=3)
    {
        wModeNum = Get_VESA_MODE(bDispalyPath);
        
        Get_MODE_INFO(wModeNum, &pModeInfo);
        
        if (GetModeColorDepth(wModeNum, pModeInfo, &bColorDepth))
        {
            VBEReturnStatus = VBEFunctionCallNotSupported;

            dwVESAMemSizeInBytes = ((ULONG)GetVESAMEMSize()) << 20;
            
            wCurrentVDispEnd = GetVDisplayEnd(bDispalyPath);
            
            
            if (((ULONG)(dwVESAMemSizeInBytes / (ULONG)wCurrentVDispEnd) & 0xFFFF0000) == 0)
            {
                wMaxPitchInBytes = (USHORT)(dwVESAMemSizeInBytes / (ULONG)wCurrentVDispEnd) & 0xFFF8;
            }
            else
            {
                wMaxPitchInBytes = 0xFFF8;
            }

            PRINTK(KERN_INFO "==VBE_SetGetScanLineLength Color Depth = 0x%x== \n",bColorDepth);
            PRINTK(KERN_INFO "==VBE_SetGetScanLineLength Mem Size = 0x%x== \n",dwVESAMemSizeInBytes);
            PRINTK(KERN_INFO "==VBE_SetGetScanLineLength Current Disp End = 0x%x== \n",wCurrentVDispEnd);
            PRINTK(KERN_INFO "==VBE_SetGetScanLineLength Max Pitch = 0x%x== \n",wMaxPitchInBytes);

            if ((pCBiosArguments->reg.lh.BL == 0) || (pCBiosArguments->reg.lh.BL == 2))
            {
                if (pCBiosArguments->reg.lh.BL == 0)
                {
                    
                    wPitchToBeSet <<= (bColorDepth >> 4);
                }
                
                if (wPitchToBeSet <= wMaxPitchInBytes)
                {
                    SetPitch(bDispalyPath, wPitchToBeSet);
                }
                else
                {
                    SetVBERerurnStatus(VBEReturnStatus, pCBiosArguments);
                    return ci_true;
                }
            }

            if (pCBiosArguments->reg.lh.BL == 3)
            {
                
                pCBiosArguments->reg.x.BX = wMaxPitchInBytes;
            }
            else
            {
                
                pCBiosArguments->reg.x.BX =GetPitch(bDispalyPath);
            }

            
            
            pCBiosArguments->reg.x.CX = pCBiosArguments->reg.x.BX >> (bColorDepth >> 4);
            pCBiosArguments->reg.x.DX = (dwVESAMemSizeInBytes / (USHORT)pCBiosArguments->reg.x.BX);
        }
    }

    PRINTK(KERN_INFO "==Exit2 VBE_SetGetScanLineLength return 0x%x== \n",VBEReturnStatus);

    SetVBERerurnStatus(VBEReturnStatus, pCBiosArguments);
    return ci_true;
}


CI_STATUS VBE_SetGetDACPaletteFormat (CBIOS_ARGUMENTS *pCBiosArguments)
{
    USHORT  usModeNum;
    UCHAR   ucColorDepth;
    MODE_INFO *pModeInfo;

    PRINTK(KERN_INFO " VBE_SetGetDACPaletteFormat() \n");    

    if (pCBiosArguments->reg.lh.BL > 1)
    {
        SetVBERerurnStatus(VBEFunctionCallFail, pCBiosArguments);
        return ci_true;
    }

    SetVBERerurnStatus(VBEFunctionCallInvalid, pCBiosArguments);

    usModeNum = Get_VESA_MODE(DISP1);
    Get_MODE_INFO(usModeNum, &pModeInfo);

    GetModeColorDepth(usModeNum, pModeInfo, &ucColorDepth);

    if (pCBiosArguments->reg.lh.BL == 0)
    {
        
        if (pCBiosArguments->reg.lh.BL == 6)
        {
            SetCRReg(0xA8, 0, BIT1);
            PRINTK(KERN_INFO "SetCRReg(0xA8, 0, BIT1);\n");
        }
        else
        {
            
            SetCRReg(0xA8, 1, BIT1);
            PRINTK(KERN_INFO "SetCRReg(0xA8, 1, BIT1);\n");
        }
    }
    else
    {
        if (GetCRReg(0xA8) & BIT1)
            pCBiosArguments->reg.lh.BH = 8;
        else
            pCBiosArguments->reg.lh.BH = 6;
    }
    
    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);

    PRINTK(KERN_INFO "  VBE_SetGetDACPaletteFormat() return\n");
    return ci_true;
}


CI_STATUS VBE_LoadUnloadPaletteData (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR   ucStartIndex;
    rdcbool bWaitVSync = rdc_false;
    ULONG   *pulARGBData;
    short   sNumOfData;

    PRINTK(KERN_INFO " VBE_LoadUnloadPaletteData()\n");

    SetVBERerurnStatus(VBEFunctionCallNotSupported, pCBiosArguments);

    if (pCBiosArguments->reg.lh.BL & ~0x81)
        return ci_true;

    if (pCBiosArguments->reg.x.CX > 256)
        return ci_true;
        
    ucStartIndex = pCBiosArguments->reg.lh.DL;
    bWaitVSync = (pCBiosArguments->reg.lh.BL & 0x80) ? rdc_true : rdc_false;
    pulARGBData = (ULONG*)pCBiosArguments->reg.ex._EDI;
    sNumOfData = pCBiosArguments->reg.x.CX;
    
    if (pCBiosArguments->reg.lh.BL & 0x01)
    {
        
        OutPort(DAC_INDEX_READ, ucStartIndex);

        for ( ; sNumOfData > 0; sNumOfData--, pulARGBData++)
        {
            *pulARGBData = 0;
            *pulARGBData = InPort(DAC_DATA);
            *pulARGBData <<= 8;
            *pulARGBData |= InPort(DAC_DATA);
            *pulARGBData <<= 8;
            *pulARGBData |= InPort(DAC_DATA);
        }        
    }
    else
    {
        
        OutPort(DAC_INDEX_WRITE, ucStartIndex);
        PRINTK(KERN_INFO "  OutPort(0x%x, 0x%x);\n", DAC_INDEX_WRITE, ucStartIndex);        

        if (bWaitVSync)
            LongWait();

        for ( ; sNumOfData > 0; sNumOfData--, pulARGBData++)
        {
            OutPort(DAC_DATA, (UCHAR)(*pulARGBData >> 16));
            PRINTK(KERN_INFO "  OutPort(0x%x, 0x%x);\n", DAC_DATA, (UCHAR)(*pulARGBData >> 16));
            OutPort(DAC_DATA, (UCHAR)(*pulARGBData >> 8));
            PRINTK(KERN_INFO "  OutPort(0x%x, 0x%x);\n", DAC_DATA, (UCHAR)(*pulARGBData >> 8));
            OutPort(DAC_DATA, (UCHAR)*pulARGBData);
            PRINTK(KERN_INFO "  OutPort(0x%x, 0x%x);\n", DAC_DATA, (UCHAR)*pulARGBData);
        }        
    }

    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);
    PRINTK(KERN_INFO "  VBE_LoadUnloadPaletteData() return\n");
    return ci_true;
}


CI_STATUS OEM_QueryBiosInfo (CBIOS_ARGUMENTS *pCBiosArguments)
{
    USHORT    VBEReturnStatus = VBEFunctionCallFail;

    PRINTK(KERN_INFO "==Entry OEM_QueryBiosInfo()== \n");

    

    


    


    



    
    pCBiosArguments->reg.x.SI = 0;
    if(bCRTSUPPORT)
    {
        pCBiosArguments->reg.x.SI |= B_CRT;
    }
    if(bLCDSUPPORT)
    {
        pCBiosArguments->reg.x.SI |= B_LCD;
    }
    if(bTVSUPPORT)
    {
        pCBiosArguments->reg.x.SI |= B_TV;
    }
    if(bDVISUPPORT)
    {
        pCBiosArguments->reg.x.SI |= B_DVI;
    }
    
    

    VBEReturnStatus = VBEFunctionCallSuccessful;
    SetVBERerurnStatus(VBEReturnStatus, pCBiosArguments);

    PRINTK(KERN_INFO "==Exit OEM_QueryBiosInfo()== \n");

    return ci_true;
}
CI_STATUS OEM_QueryBiosCaps (CBIOS_ARGUMENTS *pCBiosArguments)
{
    

    PRINTK(KERN_INFO "==Entry OEM_QueryBiosCaps()== \n");
    PRINTK(KERN_INFO "==Exit OEM_QueryBiosCaps()== \n");

    return ci_true;
}

CI_STATUS OEM_QueryExternalDeviceInfo (CBIOS_ARGUMENTS *pCBiosArguments)
{
    PRINTK(KERN_INFO "==Entry OEM_QueryExternalDeviceInfo()== \n");

    pCBiosArguments->reg.lh.BL = ReadScratch(IDX_SCRATCH_20);
    pCBiosArguments->reg.ex._EBX <<= 16;

    pCBiosArguments->reg.lh.BH = ReadScratch(IDX_SCRATCH_21);

    pCBiosArguments->reg.lh.BL = ReadScratch(IDX_SCRATCH_22);

    pCBiosArguments->reg.ex._ECX = BIT16;

    pCBiosArguments->reg.x.DX = 0xFFFF;

    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);

    PRINTK(KERN_INFO "==Exit OEM_QueryExternalDeviceInfo()== \n");

    return ci_true;
}

CI_STATUS OEM_QueryDisplayPathInfo (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR ScratchTempData;

    PRINTK(KERN_INFO "==Entry OEM_QueryDisplayPathInfo()== \n");

    pCBiosArguments->reg.ex._EBX = 0;

    pCBiosArguments->reg.lh.BL |= Get_NEW_DEV_ID(DISP1);

    pCBiosArguments->reg.ex._EBX <<= 2;
    ScratchTempData = GetSRReg(0x58);               
    if (ScratchTempData & BIT0)
    {
        ScratchTempData &= (BIT2 + BIT1);             
        ScratchTempData >>= 1;
        pCBiosArguments->reg.lh.BL |= ScratchTempData;
    }
    
    pCBiosArguments->reg.ex._EBX <<= 4;
    pCBiosArguments->reg.lh.BL |= Get_DEV_ID(DISP1);

    pCBiosArguments->reg.ex._EBX <<= 7;
    pCBiosArguments->reg.lh.BL |= Get_RRATE_ID(DISP1);
    
    pCBiosArguments->reg.ex._EBX <<= 9;   
    pCBiosArguments->reg.x.BX |= Get_VESA_MODE(DISP1);
    

    pCBiosArguments->reg.ex._ECX = 0;

    pCBiosArguments->reg.lh.CL |= Get_NEW_DEV_ID(DISP1);

    pCBiosArguments->reg.ex._ECX <<= 2;
    ScratchTempData = GetSRReg(0x50);               
    if (ScratchTempData & BIT0)
    {
        ScratchTempData &= (BIT2 + BIT1);             
        ScratchTempData >>= 1;
        pCBiosArguments->reg.lh.BL |= ScratchTempData;
    }
    
    pCBiosArguments->reg.ex._ECX <<= 4;
    pCBiosArguments->reg.lh.CL |= Get_DEV_ID(DISP2);

    pCBiosArguments->reg.ex._ECX <<= 7;
    pCBiosArguments->reg.lh.CL |= Get_RRATE_ID(DISP2);
    
    pCBiosArguments->reg.ex._ECX <<= 9;   
    pCBiosArguments->reg.x.CX |= Get_VESA_MODE(DISP2);
    
    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);

    PRINTK(KERN_INFO "==Exit OEM_QueryDisplayPathInfo()== \n");

    return ci_true;

}

CI_STATUS OEM_QueryDeviceConnectStatus (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR *pucPCIDataStruct = (UCHAR*)PCIDataStruct;
    
    PRINTK(KERN_INFO "==Entry OEM_QueryDeviceConnectStatus()== \n");

    pCBiosArguments->reg.x.BX = 0;
    
    if (*(USHORT*)(pucPCIDataStruct + OFF_DID) == 0x2010)
    {
        if (GetSRReg(0x3c) & BIT0)
        {
            pCBiosArguments->reg.x.BX |= B_CRT;   
        }
    }
    else
    {
        
    }
    
    if (bLCDSUPPORT)
    {
        pCBiosArguments->reg.x.BX |= B_LCD;   
    }

    if (bDVISUPPORT)
    {
        if (*(USHORT*)(((UCHAR*)PCIDataStruct) + OFF_DID) == 0x2010)
        {
            if (GetSRReg(0x3c) & BIT1)
            {
                pCBiosArguments->reg.x.BX |= B_DVI;   
            }
        }
        else
        {
            
        }
    }

    if (bTVSUPPORT)
    {
        
    }
    
    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);

    PRINTK(KERN_INFO "==Exit OEM_QueryDeviceConnectStatus()== \n");

    return ci_true;
}

CI_STATUS OEM_QuerySupportedMode (CBIOS_ARGUMENTS *pCBiosArguments)
{
    MODE_INFO   *pModeInfo;
    RRATE_TABLE *pRRateTable;
    int RRateTableIndex = 0;
    int ModeNumIndex;
    USHORT wModeNum = 0;
    USHORT wSerialNumber;
    
    PRINTK(KERN_INFO "==Enter OEM_QuerySupportedMode()== \n");

    wSerialNumber = pCBiosArguments->reg.x.CX;
    PRINTK(KERN_INFO "wSerialNumber = %d\n", wSerialNumber);

    pModeInfo     = (MODE_INFO*)pVESATable;
    
    while (pModeInfo->H_Size != 0xFFFF)
    {
        PRINTK(KERN_INFO "pModeInfo->H_Size = %d \n", pModeInfo->H_Size);
        PRINTK(KERN_INFO "pModeInfo->V_Size = %d \n", pModeInfo->V_Size);
        PRINTK(KERN_INFO "sizeof(RRATE_TABLE) = %d \n", sizeof(RRATE_TABLE));

        for (ModeNumIndex = 0; ModeNumIndex < 3; ModeNumIndex++)
        {
            switch (ModeNumIndex)
            {
                case 0:
                    wModeNum = pModeInfo->Mode_ID_8bpp;
                        PRINTK(KERN_INFO "case 0: wModeNum = 0x%x \n", wModeNum);
                    break;
                case 1:
                    wModeNum = pModeInfo->Mode_ID_16bpp;
                        PRINTK(KERN_INFO "case 1: wModeNum = 0x%x \n", wModeNum);
                    break;
                case 2:
                    wModeNum = pModeInfo->Mode_ID_32bpp;
                        PRINTK(KERN_INFO "case 2: wModeNum = 0x%x \n", wModeNum);
                    break;
            }

            pRRateTable = (RRATE_TABLE*)((int)pModeInfo + sizeof(MODE_INFO));

            for (RRateTableIndex = 0; RRateTableIndex < pModeInfo->RRTableCount; RRateTableIndex++, pRRateTable++)
            {

                PRINTK(KERN_INFO "pRRateTable = 0x%x \n", pRRateTable);

                if ((pRRateTable->Attribute & DISABLE) == 0)
                {
                    if (wSerialNumber == 0)
                    {
                        
                        pCBiosArguments->reg.x.BX = wModeNum;
                        PRINTK(KERN_INFO "mode num = 0x%x \n", pCBiosArguments->reg.x.BX);

                        
                        GetModeColorDepth(wModeNum, pModeInfo, &pCBiosArguments->reg.lh.CL);
                        PRINTK(KERN_INFO "color depth = %d \n", pCBiosArguments->reg.lh.CL);
                        
                        
                        pCBiosArguments->reg.lh.CH = pRRateTable->RRate_ID;
                        PRINTK(KERN_INFO "RRate ID = %d \n", pCBiosArguments->reg.lh.CH);
                        
                        
                        pCBiosArguments->reg.ex._EDX = (((ULONG)pModeInfo->V_Size) << 16) | (ULONG)pModeInfo->H_Size;
                        PRINTK(KERN_INFO "H x V = %d x %d \n", pCBiosArguments->reg.x.DX, pCBiosArguments->reg.ex._EDX>>16);
                        
                        
                        pCBiosArguments->reg.x.SI = pRRateTable->Attribute;
                        
                        
                        pCBiosArguments->reg.ex._EDI = pRRateTable->Clock;
                        PRINTK(KERN_INFO "dot clk = %dkhz \n", pCBiosArguments->reg.ex._EDI);

                        SetVBERerurnStatus (VBEFunctionCallSuccessful, pCBiosArguments);

                        PRINTK(KERN_INFO "  Exit1 OEM_QuerySupportedMode() return 0x%x== \n", VBEFunctionCallSuccessful);

                        return ci_true;
                    }
                    else
                    {
                        wSerialNumber--;
                    }
                }
            }
        }
        pModeInfo = (MODE_INFO*)((int)pModeInfo + sizeof(MODE_INFO) + pModeInfo->RRTableCount*sizeof(RRATE_TABLE));
    }

    pModeInfo = (MODE_INFO*)(&CInt10VESATable);
    PRINTK(KERN_INFO "*pModeInfo = %X \n", *pModeInfo);
    
    while (pModeInfo->H_Size != 0xFFFF)
    {
        PRINTK(KERN_INFO "pModeInfo->H_Size = %d \n", pModeInfo->H_Size);
        PRINTK(KERN_INFO "pModeInfo->V_Size = %d \n", pModeInfo->V_Size);
        PRINTK(KERN_INFO "sizeof(RRATE_TABLE) = %d \n", sizeof(RRATE_TABLE));

        for (ModeNumIndex = 0; ModeNumIndex < 3; ModeNumIndex++)
        {
            switch (ModeNumIndex)
            {
                case 0:
                    wModeNum = pModeInfo->Mode_ID_8bpp;
                        PRINTK(KERN_INFO "case 0: wModeNum = 0x%x \n", wModeNum);
                    break;
                case 1:
                    wModeNum = pModeInfo->Mode_ID_16bpp;
                        PRINTK(KERN_INFO "case 1: wModeNum = 0x%x \n", wModeNum);
                    break;
                case 2:
                    wModeNum = pModeInfo->Mode_ID_32bpp;
                        PRINTK(KERN_INFO "case 2: wModeNum = 0x%x \n", wModeNum);
                    break;
            }

            pRRateTable = (RRATE_TABLE*)((int)pModeInfo + sizeof(MODE_INFO));

            for (RRateTableIndex = 0; RRateTableIndex < pModeInfo->RRTableCount; RRateTableIndex++, pRRateTable++)
            {

                PRINTK(KERN_INFO "pRRateTable = 0x%x \n", pRRateTable);

                if ((pRRateTable->Attribute & DISABLE) == 0)
                {
                    if (wSerialNumber == 0)
                    {
                        
                        pCBiosArguments->reg.x.BX = wModeNum;
                        PRINTK(KERN_INFO "mode num = 0x%x \n", pCBiosArguments->reg.x.BX);

                        
                        GetModeColorDepth(wModeNum, pModeInfo, &pCBiosArguments->reg.lh.CL);
                        PRINTK(KERN_INFO "color depth = %d \n", pCBiosArguments->reg.lh.CL);
                        
                        
                        pCBiosArguments->reg.lh.CH = pRRateTable->RRate_ID;
                        PRINTK(KERN_INFO "RRate ID = %d \n", pCBiosArguments->reg.lh.CH);
                        
                        
                        pCBiosArguments->reg.ex._EDX = (((ULONG)pModeInfo->V_Size) << 16) | (ULONG)pModeInfo->H_Size;
                        PRINTK(KERN_INFO "H x V = %d x %d \n", pCBiosArguments->reg.x.DX, pCBiosArguments->reg.ex._EDX>>16);
                        
                        
                        pCBiosArguments->reg.x.SI = pRRateTable->Attribute;
                        
                        
                        pCBiosArguments->reg.ex._EDI = pRRateTable->Clock;
                        PRINTK(KERN_INFO "dot clk = %dkhz \n", pCBiosArguments->reg.ex._EDI);

                        SetVBERerurnStatus (VBEFunctionCallSuccessful, pCBiosArguments);

                        PRINTK(KERN_INFO "  Exit1 OEM_QuerySupportedMode() return 0x%x== \n", VBEFunctionCallSuccessful);

                        return ci_true;
                    }
                    else
                    {
                        wSerialNumber--;
                    }
                }
            }
        }
        pModeInfo = (MODE_INFO*)((int)pModeInfo + sizeof(MODE_INFO) + pModeInfo->RRTableCount*sizeof(RRATE_TABLE));
    }        

    SetVBERerurnStatus (VBEFunctionCallFail, pCBiosArguments);

    PRINTK(KERN_INFO "  Exit2 OEM_QuerySupportedMode() return 0x%x== \n", VBEFunctionCallFail);

    return ci_true;
}

CI_STATUS OEM_QueryLCDPanelSizeMode (CBIOS_ARGUMENTS *pCBiosArguments)
{
    MODE_INFO   *pModeInfo;
    PANEL_TABLE *pPanelTable;
    UCHAR        bColorDepth = pCBiosArguments->reg.lh.CL;
    USHORT        wModeNum;

    PRINTK(KERN_INFO "==Enter OEM_QueryLCDPanelSizeMode()== \n");

    SetVBERerurnStatus (VBEFunctionCallFail, pCBiosArguments);
    
    if (!bLCDSUPPORT)
        return ci_true;
        
    if (!GetModePointerFromLCDTable(LCD_ID, &pModeInfo, &pPanelTable))
        return ci_true;

    switch (bColorDepth)
    {
        case 0:
            wModeNum = pModeInfo->Mode_ID_8bpp;
            pCBiosArguments->reg.lh.CL = 8;
            break;
        case 1:
            wModeNum = pModeInfo->Mode_ID_16bpp;
            pCBiosArguments->reg.lh.CL = 16;
            break;
        case 2:
            wModeNum = pModeInfo->Mode_ID_32bpp;
            pCBiosArguments->reg.lh.CL = 32;
            break;
        default:
            SetVBERerurnStatus (VBEFunctionCallFail, pCBiosArguments);

            PRINTK(KERN_INFO "==Exit2 OEM_QueryLCDPanelSizeMode() return 0x%x== \n", VBEFunctionCallFail);

            return ci_true;
    }
        
    
    pCBiosArguments->reg.x.BX = wModeNum;
    
    
    pCBiosArguments->reg.lh.CH = pPanelTable->Timing.RRate_ID;
    
    
    pCBiosArguments->reg.ex._EDX = ((ULONG)pModeInfo->H_Size)  | (((ULONG)pModeInfo->V_Size) << 16);
    
    
    pCBiosArguments->reg.x.SI = pPanelTable->Timing.Attribute;
    
    
    pCBiosArguments->reg.ex._EDI = pPanelTable->Timing.Clock;
    
    SetVBERerurnStatus (VBEFunctionCallSuccessful, pCBiosArguments);

    PRINTK(KERN_INFO "==Exit1 OEM_QueryLCDPanelSizeMode() return 0x%x== \n", VBEFunctionCallSuccessful);

    return ci_true;
}

CI_STATUS OEM_QueryLCDPWMLevel(CBIOS_ARGUMENTS *pCBiosArguments)
{
    PRINTK(KERN_INFO "==Entry OEM_QueryLCDPWMLevel()== \n");

    SetVBERerurnStatus (VBEFunctionCallFail, pCBiosArguments);
    
    if (!bLCDSUPPORT)
        return ci_true;

    if ((GetSRReg(0x30)&0x03) == 0x03 )
        pCBiosArguments->reg.lh.BL = GetSRReg(0x30);
    else
        pCBiosArguments->reg.lh.BL = 0;
        
    SetVBERerurnStatus (VBEFunctionCallSuccessful, pCBiosArguments);
    
    return ci_true;
}

CI_STATUS OEM_QueryTVConfiguration (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_QueryTV2Configuration (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_QueryHDTVConfiguration (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_QueryHDTV2Configuration (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_QueryHDMIConfiguration (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_QueryHDMI2Configuration (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetActiv_EDIsplayDevice (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR bDeviceIndex1 = Get_DEV_ID(DISP1);
    UCHAR bDeviceIndex2 = Get_DEV_ID(DISP2);
    UCHAR bNewDeviceIndex1 = pCBiosArguments->reg.lh.CL & 0x0F;
    UCHAR bNewDeviceIndex2 = (pCBiosArguments->reg.lh.CL >> 4) & 0x0F;
    
    PRINTK(KERN_INFO "==Entry OEM_SetActiv_EDIsplayDevice()== \n");

    
    if ((!CheckForNewDeviceAvailable(bNewDeviceIndex1)&& (bNewDeviceIndex1!=0)) || (!CheckForNewDeviceAvailable(bNewDeviceIndex2)&& (bNewDeviceIndex2!=0)))
    {
        SetVBERerurnStatus(VBEFunctionCallFail, pCBiosArguments);

        PRINTK(KERN_INFO "==Exit1 OEM_SetActiv_EDIsplayDevice() return 0x%x== \n", VBEFunctionCallFail);

        return ci_true;
    }
    
    if(CheckForDSTNPanel(bNewDeviceIndex1) || CheckForDSTNPanel(bNewDeviceIndex2))
    {
        bNewDeviceIndex1 = 0;
    }

    if (bDeviceIndex1 != bNewDeviceIndex1)
    {
        if (bNewDeviceIndex1 == 0)
        {
            Disabl_EDIsplayPathAndDevice(DISP1);
            Set_DEV_ID(bNewDeviceIndex1, DISP1);
        }
        Set_NEW_DEV_ID(bNewDeviceIndex1, DISP1);
    }

    if (bDeviceIndex2 != bNewDeviceIndex2)
    {
        if (bNewDeviceIndex2 == 0)
        {
            Disabl_EDIsplayPathAndDevice(DISP2);
            Set_DEV_ID(bNewDeviceIndex2, DISP2);
        }
        Set_NEW_DEV_ID(bNewDeviceIndex2, DISP2);
    }

    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);

    PRINTK(KERN_INFO "==Exit2 OEM_SetActiv_EDIsplayDevice() return 0x%x== \n", VBEFunctionCallSuccessful);

    return ci_true;
}
CI_STATUS OEM_SetVESAModeForDisplay2 (CBIOS_Extension *pCBIOSExtension)
{
    USHORT    wModeNum = pCBIOSExtension->CBiosArguments.reg.x.CX & 0x01FF;
    USHORT    wPitch = 0;
    MODE_INFO   *pModeInfo = NULL;
    UCHAR    bColorDepth = 0;
    UCHAR    bCurDeviceID, bNewDeviceID;

    PRINTK(KERN_INFO "==Entry OEM_SetVESAModeForDisplay2()== \n");

    if (wModeNum < 0x100)
    {
        SetVBERerurnStatus(VBEFunctionCallFail, &pCBIOSExtension->CBiosArguments);

        PRINTK(KERN_INFO "==Exit1 OEM_SetVESAModeForDisplay2() return 0x%x== \n", VBEFunctionCallFail);

        return ci_true;
    }
    
    bCurDeviceID = Get_DEV_ID(DISP2);
    bNewDeviceID = Get_NEW_DEV_ID(DISP2);

    
    if(!Get_MODE_INFO(wModeNum, &pModeInfo))
    {
        SetVBERerurnStatus(VBEFunctionCallFail, &pCBIOSExtension->CBiosArguments);

        PRINTK(KERN_INFO "==Exit2 OEM_SetVESAModeForDisplay2() return 0x%x== \n", VBEFunctionCallFail);

        return ci_true;
    }
    
    Set_VESA_MODE(wModeNum, DISP2);
    
    
    SequencerOff(DISP2);

    
    TurnOffScaler(DISP2);

    if (bCurDeviceID != bNewDeviceID)
    {
        
        ControlPwrSeqOff(bCurDeviceID);

        

        
        TurnOffDigitalPort(bCurDeviceID);

        
        Set_DEV_ID(bNewDeviceID, DISP2);
    }
    
    
    
    
    LoadTiming(DISP2, wModeNum);

    
    GetModePitch(wModeNum, &wPitch);
    SetPitch(DISP2, wPitch);

    
    Get_MODE_INFO(wModeNum, &pModeInfo);
    GetModeColorDepth(wModeNum, pModeInfo, &bColorDepth);
    SetColorDepth(DISP2, bColorDepth);

    

    

    
    if(!(pCBIOSExtension->CBiosArguments.reg.x.CX & BIT15))
    {
        ClearFrameBuffer(DISP2,(ULONG*)(pCBIOSExtension->VideoVirtualAddress),pModeInfo,bColorDepth);
    }

    
    SetFIFO(DISP2);

    
    ConfigDigitalPort(DISP2);

    TurnOnDigitalPort(bNewDeviceID);

    

    
    ControlPwrSeqOn(bNewDeviceID);
    
    SequencerOn(DISP2);
    
    SetVBERerurnStatus(VBEFunctionCallSuccessful, &pCBIOSExtension->CBiosArguments);
    
    PRINTK(KERN_INFO "==Exit3 OEM_SetVESAModeForDisplay2() return 0x%x== \n", VBEFunctionCallSuccessful);

    return ci_true;
}

CI_STATUS OEM_SetDevicePowerState (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR display1DeviceID, display2DeviceID, TargetDevice, DMPSState;
    USHORT VBEReturnStatus = VBEFunctionCallFail;
    
    PRINTK(KERN_INFO "==Entry OEM_SetDevicePowerState()== \n");

    TargetDevice = pCBiosArguments->reg.lh.CL & 0x0F;
    DMPSState = pCBiosArguments->reg.lh.DL & (BIT1+BIT0);
    display1DeviceID = Get_DEV_ID(DISP1);
    display2DeviceID = Get_DEV_ID(DISP2);

    if (display1DeviceID == TargetDevice)
    {
        SetDPMS(DMPSState, DISP1);
        VBEReturnStatus = VBEFunctionCallSuccessful;
    }
    else if (display2DeviceID == TargetDevice)
    {
        SetDPMS(DMPSState, DISP2);
        VBEReturnStatus = VBEFunctionCallSuccessful;
    }

    SetVBERerurnStatus(VBEReturnStatus, pCBiosArguments);
    
    PRINTK(KERN_INFO "==Exit OEM_SetDevicePowerState() return 0x%x== \n", VBEFunctionCallSuccessful);

    return ci_true;
}

CI_STATUS OEM_SetRefreshRate (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR bRRateID, bDisplayPath;
    bRRateID = pCBiosArguments->reg.lh.CL & 0x7F;
    
    PRINTK(KERN_INFO "==Entry OEM_SetRefreshRate()== \n");

    if ( pCBiosArguments->reg.x.BX == SetDisplay1RefreshRate)
        bDisplayPath = DISP1;
    else
        bDisplayPath = DISP2;

    Set_RRATE_ID(bRRateID, bDisplayPath);

    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);
    
    PRINTK(KERN_INFO "==Exit OEM_SetRefreshRate() return 0x%x== \n", VBEFunctionCallSuccessful);

    return ci_true;
}

CI_STATUS OEM_SetLCDPWMLevel (CBIOS_ARGUMENTS *pCBiosArguments)
{
    SetSRReg(0x2F,7,0xFF);
    SetSRReg(0x30,pCBiosArguments->reg.lh.CL,0xFF);            
    return ci_true;
}

CI_STATUS OEM_SetTVType (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetTV2Type (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetTVConnectType (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetTV2ConnectType (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetHDTVConnectType (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetHDTV2ConnectType (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetHDMIType (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetHDMIOutputSignal (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}
CI_STATUS OEM_SetHDMI2OutputSignal (CBIOS_ARGUMENTS *pCBiosArguments)
{
    return ci_true;
}

CI_STATUS OEM_VideoPOST (CBIOS_ARGUMENTS *pCBiosArguments)
{
    UCHAR i = 0,btemp = 0x0,btemp1 = 0x0;
    CI_STATUS bDDRII400 = ci_true;
    UCHAR *pucPOSTInItRegs = POSTInItRegs;
    UCHAR *pucDDRII400Tbl = DDRII400Tbl;
    UCHAR *pucDDRII533Tbl = DDRII533Tbl;
    UCHAR *pucExtendRegs2 = ExtendRegs2;

    PRINTK(KERN_INFO "==Entry OEM_VideoPOST()== \n");

    
    btemp = InPort(RIO_VGA_ENABLE); 
    btemp |= 1;
    OutPort(RIO_VGA_ENABLE,btemp);

    
    btemp = InPort(MISC_READ); 
    btemp |= 3;
    OutPort(MISC_WRITE,btemp);
    
    
    SetCRReg(0x80, 0xA8, 0xFF);
    
    
    for(i = 0x81;i < 0x9F;i++)
    {
        SetCRReg(i, 0x00, 0xFF);
    }

    
    
    btemp = GetCRReg(0xAB);
    if((btemp & 0x3) == 0x3)
    {
        bDDRII400 = ci_false;
    }

    if(bDDRII400)
    {
        
        SetCRReg(0xD9, 0x00, 0x80);
        SetCRReg(0xD8, 0x9B, 0xFF);
    }
    else
    {
        
        SetCRReg(0xD9, 0x80, 0x80);
        SetCRReg(0xD8, 0x78, 0xFF);
    }
    
    
    
    btemp = 0x00;
    SetCRReg(0xBB, btemp, 0xFF);
    
    
    

    
    SerialLoadTable(&pucPOSTInItRegs, 0, 0);

    
    Set_NEW_DEV_ID(0, DISP1);

    if(bDDRII400)
    {
        
        SerialLoadTable(&pucDDRII400Tbl, 0, 0);
    }
    else
    {
        
        SerialLoadTable(&pucDDRII533Tbl, 0, 0);    
    }

    
    do{
        btemp = GetCRReg(0x5D);
        btemp &= 0x80;
        btemp1 = GetCRReg(0x5E);
        btemp1 &= 0x01;
    }while((btemp != 0x80)||(btemp1 != 0x01));

    
    SerialLoadTable(&pucExtendRegs2, 0, 0);
    

    PRINTK(KERN_INFO "==Exit OEM_VideoPOST()== \n");

    return ci_true;
}

void* SearchString(char *pcKeyWord, UCHAR *from)
{
    int i, lenKeyWord;

    lenKeyWord = strlen(pcKeyWord);
    for(i = 0; i < BIOS_ROM_SIZE; i++)
    {
        if ((*pcKeyWord == *(from+i)) && !memcmp(pcKeyWord, from+i, lenKeyWord))
        {
            return from+i;
        }
    }

    return NULL;
}

void ParseTable(char* pcKeyWord, UCHAR *from, UCHAR **pointer)
{
    *pointer = (UCHAR*)SearchString(pcKeyWord, from);
    *pointer += strlen(pcKeyWord);
}

CI_STATUS OEM_CINT10DataInit (CBIOS_ARGUMENTS *pCBiosArguments)
{
    PRINTK(KERN_INFO "BIOS virtual = %x\n", (int)pCBiosArguments->reg.ex._ECX);
    
    ParseTable("PCFG", (UCHAR*)pCBiosArguments->reg.ex._ECX, (UCHAR**)(&pPortConfig));
    PRINTK(KERN_INFO "Port Config = %x\n", (int)pPortConfig);
    
    ParseTable("VPIT", (UCHAR*)pCBiosArguments->reg.ex._ECX, (UCHAR**)(&pVESATable));
    PRINTK(KERN_INFO "VESA Table = %x\n", (int)pVESATable);
    
    ParseTable("LCDTBL", (UCHAR*)pCBiosArguments->reg.ex._ECX, (UCHAR**)(&pLCDTable));
    PRINTK(KERN_INFO "LCD Table = %x\n", (int)pLCDTable);

    ParseTable("PCIR", (UCHAR*)pCBiosArguments->reg.ex._ECX, (UCHAR**)(&PCIDataStruct));
    PRINTK(KERN_INFO "PCI Data Struct = %x\n", (int)PCIDataStruct);

    ParseTable("D1INIT", (UCHAR*)pCBiosArguments->reg.ex._ECX, (UCHAR**)(&Display1VESAModeInitRegs));
    PRINTK(KERN_INFO "Display1 VESA Mode Init Regs = %x\n", (int)																																																																																									Display1VESAModeInitRegs);

    SetDeviceSupport();
#if 0 
    ParseTable("??????", &ExtendRegs);  
    ParseTable("??????", &ExtendRegs2); 
    ParseTable("??????", &DDRII400Tbl); 
    ParseTable("??????", &DDRII533Tbl); 
#endif
    SetVBERerurnStatus(VBEFunctionCallSuccessful, pCBiosArguments);

    return ci_true;
}

CI_STATUS CInt10(CBIOS_Extension *pCBIOSExtension)
{
    CI_STATUS CInt10_Status = ci_false;

    PRINTK(KERN_INFO "==Enter CInt10(_EAX = %x, _EBX = %x, _ECX = %x, _EDX = %x, _ESI = %x, _EDI = %x)==\n",
                   pCBIOSExtension->CBiosArguments.reg.ex._EAX, pCBIOSExtension->CBiosArguments.reg.ex._EBX,
                   pCBIOSExtension->CBiosArguments.reg.ex._ECX, pCBIOSExtension->CBiosArguments.reg.ex._EDX,
                   pCBIOSExtension->CBiosArguments.reg.ex._ESI, pCBIOSExtension->CBiosArguments.reg.ex._EDI);

    
    Relocate_IOAddress = pCBIOSExtension->IOAddress;

    switch(pCBIOSExtension->CBiosArguments.reg.x.AX)
    {
        case VBESetMode: 
            CInt10_Status = VBE_SetMode(pCBIOSExtension);
            break;

        case VBESetGetScanLineLength: 
            CInt10_Status = VBE_SetGetScanLineLength(&pCBIOSExtension->CBiosArguments);
            break;

        case VBESetGetDACPaletteFormat: 
            CInt10_Status = VBE_SetGetDACPaletteFormat(&pCBIOSExtension->CBiosArguments);
            break;

        case VBELoadUnloadPaletteData: 
            CInt10_Status = VBE_LoadUnloadPaletteData(&pCBIOSExtension->CBiosArguments);
            break;

            
        case OEMFunction:
            switch(pCBIOSExtension->CBiosArguments.reg.x.BX)
            {
                case QueryBiosInfo:             
                    CInt10_Status = OEM_QueryBiosInfo(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryBiosCaps:             
                    CInt10_Status = OEM_QueryBiosCaps(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryExternalDeviceInfo:   
                    CInt10_Status = OEM_QueryExternalDeviceInfo(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryDisplayPathInfo:      
                    CInt10_Status = OEM_QueryDisplayPathInfo(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryDeviceConnectStatus:  
                    CInt10_Status = OEM_QueryDeviceConnectStatus(&pCBIOSExtension->CBiosArguments);
                    break;
                case QuerySupportedMode:        
                    CInt10_Status = OEM_QuerySupportedMode(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryLCDPanelSizeMode:        
                    CInt10_Status = OEM_QueryLCDPanelSizeMode(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryLCDPWMLevel:          
                    CInt10_Status = OEM_QueryLCDPWMLevel(&pCBIOSExtension->CBiosArguments);
                    break;
                case QueryTVConfiguration:
                    break;
                case QueryTV2Configuration:
                    break;
                case QueryHDTVConfiguration:
                    break;
                case QueryHDTV2Configuration:
                    break;
                case QueryHDMIConfiguration:
                    break;
                case QueryHDMI2Configuration:
                    break;
                case QueryDisplay2Pitch:
                case GetDisplay2MaxPitch:
                    CInt10_Status = VBE_SetGetScanLineLength(&pCBIOSExtension->CBiosArguments);
                    break;

                case SetActiveDisplayDevice:    
                    CInt10_Status = OEM_SetActiv_EDIsplayDevice(&pCBIOSExtension->CBiosArguments);
                    break;
                case SetVESAModeForDisplay2:
                    CInt10_Status = OEM_SetVESAModeForDisplay2(pCBIOSExtension);
                    break;
                case SetDevicePowerState:       
                    CInt10_Status = OEM_SetDevicePowerState(&pCBIOSExtension->CBiosArguments);
                    break;
                case SetDisplay1RefreshRate:    
                case SetDisplay2RefreshRate:    
                    CInt10_Status = OEM_SetRefreshRate(&pCBIOSExtension->CBiosArguments);
                    break;
                case SetLCDPWMLevel:            
                    CInt10_Status = OEM_SetLCDPWMLevel(&pCBIOSExtension->CBiosArguments);
                    break;
                case SetTVType:
                    break;
                case SetTV2Type:
                    break;
                case SetTVConnectType:
                    break;
                case SetTV2ConnectType:
                    break;
                case SetHDTVConnectType:
                    break;
                case SetHDTV2ConnectType:
                    break;
                case SetHDMIType:
                    break;
                case SetHDMI2Type:
                    break;
                case SetHDMIOutputSignal:
                    break;
                case SetHDMI2OutputSignal:
                    break;
                case SetDisplay2PitchInPixels:
                case SetDisplay2PitchInBytes:
                    CInt10_Status = VBE_SetGetScanLineLength(&pCBIOSExtension->CBiosArguments);
                    break;
                case SetVideoPOST:
                    CInt10_Status = OEM_VideoPOST(&pCBIOSExtension->CBiosArguments);
                    break;
                case CINT10DataInit:
                    CInt10_Status = OEM_CINT10DataInit(&pCBIOSExtension->CBiosArguments);
                    break;
                default:
                    pCBIOSExtension->CBiosArguments.reg.x.AX = 0x014F;
                    break;
            }
            break;

        default:
            break;
    }

    PRINTK(KERN_INFO "==Exit CInt10(_EAX = %x, _EBX = %x, _ECX = %x, _EDX = %x, _ESI = %x, _EDI = %x)== \n",
                   pCBIOSExtension->CBiosArguments.reg.ex._EAX, pCBIOSExtension->CBiosArguments.reg.ex._EBX,
                   pCBIOSExtension->CBiosArguments.reg.ex._ECX, pCBIOSExtension->CBiosArguments.reg.ex._EDX,
                   pCBIOSExtension->CBiosArguments.reg.ex._ESI, pCBIOSExtension->CBiosArguments.reg.ex._EDI);
    
    return CInt10_Status;
} 

