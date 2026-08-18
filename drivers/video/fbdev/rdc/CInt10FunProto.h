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



void SetVBERerurnStatus(USHORT VBEReturnStatus, CBIOS_ARGUMENTS *pCBiosArguments);

void SetTimingRegs(UCHAR ucDisplayPath, MODE_INFO *pModeInfo, RRATE_TABLE *pRRateTable);
void SetHTotal(UCHAR DisplayPath, USHORT Value);
void SetHDisplayEnd(UCHAR DisplayPath, USHORT Value);
void SetHBlankingStart(UCHAR DisplayPath, USHORT Value);
void SetHBlankingEnd(UCHAR DisplayPath, USHORT Value);
void SetHSyncStart(UCHAR DisplayPath, USHORT Value);
void SetHSyncEnd(UCHAR DisplayPath, USHORT Value);
void SetVTotal(UCHAR DisplayPath, USHORT Value);
void SetVDisplayEnd(UCHAR DisplayPath, USHORT Value);
void SetVBlankingStart(UCHAR DisplayPath, USHORT Value);
void SetVBlankingEnd(UCHAR DisplayPath, USHORT Value);
void SetVSyncStart(UCHAR DisplayPath, USHORT Value);
void SetVSyncEnd(UCHAR DisplayPath, USHORT Value);
void SetPixelClock(UCHAR DisplayPath, ULONG Clock);
void SetHSource(UCHAR bDisplayPath, USHORT wValue);
PLL_Info ClockToPLLF9003A(ULONG Clock);
void SetDPLL(UCHAR DisplayPath, PLL_Info PLLInfo);
void SetPolarity(UCHAR DevicePort, UCHAR Value);
void SetFIFO(UCHAR DisplayPath);
void SetPitch(UCHAR DisplayPath, USHORT Value);
USHORT GetPitch(UCHAR DisplayPath);
USHORT GetVDisplayEnd(UCHAR DisplayPath);
void SetColorDepth(UCHAR DisplayPath, UCHAR Value);
void ConfigDigitalPort(UCHAR bDisplayPath);
void LoadTiming(UCHAR DisplayPath, USHORT ModeNum);
void LoadVESATiming(UCHAR bDisplayPath, USHORT wModeNum);
void LoadLCDTiming(UCHAR bDisplayPath, USHORT wModeNum);
void SetScalingFactor(UCHAR bDisplayPath, MODE_INFO *pUserModeInfo, MODE_INFO *pPanelModeInfo);
void SetHorScalingFactor(UCHAR bDisplayPath, USHORT wValue);
void SetVerScalingFactor(UCHAR bDisplayPath, USHORT wValue);
CI_STATUS isLCDFitMode(UCHAR bDeviceIndex, USHORT wModeNum);
CI_STATUS GetModePointerFromVESATable(USHORT wModeNum, UCHAR bRRIndex, MODE_INFO **ppModeInfo, RRATE_TABLE **ppRRateTable);
CI_STATUS GetModePointerFromLCDTable(UCHAR bDeviceIndex, MODE_INFO **ppModeInfo, PANEL_TABLE **ppPanelTable);
CI_STATUS Get_MODE_INFO(USHORT wModeNum, MODE_INFO **ppModeInfo);
CI_STATUS Get_MODE_INFO_From_LCD_Table(UCHAR bDeviceIndex, MODE_INFO **ppModeInfo);
CI_STATUS Get_MODE_INFO_From_VESA_Table(USHORT wModeNum, MODE_INFO **ppModeInfo);
CI_STATUS GetModeColorDepth(USHORT wModeNum, MODE_INFO *pModeInfo, UCHAR *pbColorDepth);

CI_STATUS GetModePitch(USHORT ModeNum, USHORT *pPitch);
USHORT ReadRegFromHW(REG_OP *pRegOp);
void WriteRegToHW(REG_OP *pRegOp, USHORT value);
void UnLockCR0ToCR7(void);
void LockCR0ToCR7(void);
CI_STATUS CheckForModeAvailable(USHORT ModeNum);
CI_STATUS CheckForNewDeviceAvailable(UCHAR bDeviceIndex);
void Display1TurnOnTX(void);
void Display1TurnOffTX(void);
void Display2TurnOnTX(void);
void Display2TurnOffTX(void);
void TurnOnDigitalPort(UCHAR bDeviceIndex);
void TurnOffDigitalPort(UCHAR bDeviceIndex);
UCHAR GetPortConnectPath(UCHAR PortType);
USHORT TransDevIDtoBit(UCHAR DeviceIndex);
void TurnOnCRTPad(void);
void TurnOffCRTPad(void);
void TurnOnDVP1Pad(void);
void TurnOffDVP1Pad(void);
void TurnOnDVP2Pad(void);
void TurnOffDVP2Pad(void);
void TurnOnDVP12Pad(void);
void TurnOffDVP12Pad(void);
void TurnOnScaler(UCHAR bDisplayPath);
void TurnOffScaler(UCHAR bDisplayPath);
void TurnOnHorScaler(UCHAR bDisplayPath);
void TurnOffHorScaler(UCHAR bDisplayPath);
void TurnOnVerScaler(UCHAR bDisplayPath);
void TurnOffVerScaler(UCHAR bDisplayPath);

void Set12BitDVP(void);
void Set24BitDVP(void);
void TurnOnDAC(void);
void TurnOffDAC(void);
void SerialLoadTable(UCHAR **ppucDisplay1VESAModeInitRegs, UCHAR ucI2Cport, UCHAR ucI2CAddr);
void SerialLoadRegBits(UCHAR **ppucDisplay1VESAModeInitRegs, UCHAR ucRegGroup, UCHAR ucI2Cport, UCHAR ucI2CAddr);
void SerialLoadReg(UCHAR **ppucDisplay1VESAModeInitRegs, UCHAR ucRegGroup, UCHAR ucI2Cport, UCHAR ucI2CAddr);
void LoadDisplay1VESAModeInitRegs(void);
void VESASetBIOSData(USHORT ModeNum);
void DisableDisplayPathAndDevice(UCHAR DisplayPath);
CI_STATUS GetDevicePortConfig(UCHAR bDeviceIndex, PORT_CONFIG **ppPortConfig);
void PowerSequenceOn(void);
void PowerSequenceOff(void);
void SequencerOn(UCHAR DisplayPath);
void SequencerOff(UCHAR bDisplayPath);
void ControlPwrSeqOn(UCHAR bDeviceIndex);
void ControlPwrSeqOff(UCHAR bDeviceIndex);
void LongWait(void);
UCHAR Get_DEV_ID(UCHAR DisplayPath);
void Set_DEV_ID(UCHAR DeviceID, UCHAR DisplayPath);
UCHAR Get_NEW_DEV_ID(UCHAR DisplayPath);
void Set_NEW_DEV_ID(UCHAR DeviceID, UCHAR DisplayPath);
USHORT Get_LCD_H_SIZE(void);
USHORT Get_LCD_V_SIZE(void);
ULONG Get_LCD_SIZE(void);
USHORT Get_LCD2_H_SIZE(void);
USHORT Get_LCD2_V_SIZE(void);
ULONG Get_LCD2_SIZE(void);
UCHAR Get_RRATE_ID(UCHAR DisplayPath);
void Set_RRATE_ID(UCHAR RRateID, UCHAR DisplayPath);
UCHAR Get_LCD_TABLE_INDEX(void);
UCHAR Get_LCD2_TABLE_INDEX(void);
void Set_LCD_TABLE_INDEX(UCHAR bLCDIndex);
USHORT Get_VESA_MODE(UCHAR DisplayPath);
void Set_VESA_MODE(USHORT ModeNum, UCHAR DisplayPath);
void ResetATTR(void);
void EnableATTR(void);
void SetCRReg(UCHAR bRegIndex, UCHAR bRegValue, UCHAR bMask);
UCHAR GetCRReg(UCHAR bRegIndex);
void SetSRReg(UCHAR bRegIndex, UCHAR bRegValue, UCHAR bMask);
UCHAR GetSRReg(UCHAR bRegIndex);
void SetARReg(UCHAR index,UCHAR value);
UCHAR GetARReg(UCHAR index);
void SetGRReg(UCHAR bRegIndex, UCHAR bRegValue, UCHAR bMask);
void SetMSReg(UCHAR bRegValue);
UCHAR GetIS1Reg(void);
void ClearFrameBuffer(UCHAR DisplayPath,ULONG *pFrameBufferBase,MODE_INFO *pModeInfo, UCHAR bColorDepth);
ULONG Difference(ULONG Value1, ULONG Value2);
UCHAR ReadScratch(USHORT IndexMask);
void WriteScratch(USHORT IndexMask, UCHAR Data);
UCHAR AlignDataToLSB(UCHAR bData, UCHAR bMask);
UCHAR AlignDataToMask(UCHAR bData, UCHAR bMask);
void SetDPMS(UCHAR DPMSState, UCHAR DisplayPath);
CI_STATUS DetectMonitor(void);
void WaitDisplayPeriod(void);
void WaitPowerSequenceDone(void);
CI_STATUS CheckForDSTNPanel(UCHAR bDeviceIndex);
USHORT GetVESAMEMSize(void);
void SetDeviceSupport(void);


void I2CWriteClock(UCHAR I2CPort, UCHAR data);
void I2CDelay(UCHAR I2CPort);
void I2CWriteData(UCHAR I2CPort, UCHAR data);
void I2CStart(UCHAR I2CPort);
void SendI2CDataByte(UCHAR I2CPort, UCHAR Data);
rdcbool CheckACK(UCHAR I2CPort);
UCHAR ReceiveI2CDataByte(UCHAR I2CPort, UCHAR I2CSlave);
void SendNACK(UCHAR I2CPort);
void I2CStop(UCHAR I2CPort);
UCHAR ReadI2C(UCHAR I2CPort, UCHAR I2CSlave, UCHAR RegIdx, UCHAR* RegData);
UCHAR WriteI2C(UCHAR I2CPort, UCHAR I2CSlave, UCHAR RegIdx, UCHAR RegData);
