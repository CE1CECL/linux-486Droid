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


#include "rdcfb.h"

#ifdef MODULE
MODULE_AUTHOR("(c) 2010 Peter Chen rdc_fb@rdc.com.tw>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("fbdev driver V0.0.1 for M2010/M2012");

module_param(mode_option, charp, 0444);
MODULE_PARM_DESC(mode_option, "Default video mode ('640x480-8@60', etc)");
module_param_named(mode, mode_option, charp, 0444);

MODULE_PARM_DESC(mode, "Default video mode ('640x480-8@60', etc) (deprecated)");

module_param(mmio2d, bool, 0444);
MODULE_PARM_DESC(mmio2d, "Default (false)");

#ifdef CONFIG_MTRR
module_param(mtrr, int, 0444);
MODULE_PARM_DESC(mtrr, "Enable write-combining with MTRR (1=enable, 0=disable, default=1)");
#endif
#endif
static __inline__ char* get_opt_string(const char *opt, const char *name)
{
    const char *p;
    int i;
    char *ret = NULL;

    p = opt + strlen(name);
    i=0;

    while(p[i] && p[i]!=',' && p[i]!=' ')
        i++;

    ret = kmalloc(i+1, GFP_KERNEL);

    if (ret)
    {
        strncpy(ret,p,i);
        ret[i]='\0';
    }
    return ret;
}


void vWaitEngIdle(struct rdcfb_info *par)
{
    u32  ulEngState, ulEngState2;

    if(par->b2DEnabled)
    {
        do  
        {
            ulEngState = *((volatile u32*)(MMIOREG_ENG_STATUS));    
            ulEngState2 = *((volatile u32*)(MMIOREG_WRITE_PTR));        
        } while ((ulEngState & 0x80000000) || 
                ((ulEngState&0x3ffff) != (ulEngState2&0x3ffff)));
    }
}

void vCRWaitEngIdle(struct rdcfb_info *par)
{
    u32 ulReadPointor, ulWritePointor, ulEngState;

    if(par->b2DEnabled)
    {
        ulWritePointor = *((volatile u32*)(par->CMDQInfo.pjWritePort));
        ulWritePointor &= 0x3ffff;
    
        do  
        {
            ulReadPointor = *((volatile u32*)(par->CMDQInfo.pjReadPort));
            ulEngState = *((volatile u32*)(par->CMDQInfo.pjEngStatePort));
        }
        while ((ulEngState & BIT12) || (ulWritePointor != ulReadPointor));
    }
}
void vEnable2D(struct fb_info *info)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;

    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA4, 0xFE, 0x01);        

    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA3, ~0x20, 0x20);           

    writel(readl(MMIOREG_1ST_FLIP)|0x80000000, MMIOREG_1ST_FLIP); 

    if (!bInitCMDQInfo(info))
    {
        par->CMDQInfo.Disable2D(par);      
        return;
    }
        
    if (!bEnableCMDQ(par))
    {
        par->CMDQInfo.Disable2D(par);      
        return;
    }
}

void vCREnable2D(struct fb_info *info)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;

    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA4, 0xFE, 0x01);        

    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA3, ~0x20, 0x20);           

    writel(readl(MMIOREG_1ST_FLIP)|0x80000000, MMIOREG_1ST_FLIP); 

    if (!bCRInitCMDQInfo(info))
    {
        vCRDisable2D(par);      
        return;
    }
        
    if (!bCREnableCMDQ(par))
    {
        vCRDisable2D(par);      
        return;
    }
    par->b2DEnabled = true; 
}


void vDisable2D(struct rdcfb_info *par)
{
    vWaitEngIdle(par);
    
    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA4, 0xFE, 0x00);        

    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA3, ~0x20, 0x00);       

    writel(readl(MMIOREG_1ST_FLIP)&(~0x80000000), MMIOREG_1ST_FLIP); 
}

void vCRDisable2D(struct rdcfb_info *par)
{
    vCRWaitEngIdle(par);
    
    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA4, 0xFE, 0x00);        

    SetIndexRegMask(COLOR_CRTC_INDEX, 0xA3, ~0x20, 0x00);       

    writel(readl(MMIOREG_1ST_FLIP)&(~0x80000000), MMIOREG_1ST_FLIP); 

    par->b2DEnabled = false;
}


bool bInitCMDQInfo(struct fb_info *info)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;

    par->CMDQInfo.pjCmdQBasePort    = MMIOREG_CMDQ_SET; 
    par->CMDQInfo.pjWritePort       = MMIOREG_WRITE_PTR;
    par->CMDQInfo.pjReadPort        = MMIOREG_ENG_STATUS;
    par->CMDQInfo.pjEngStatePort    = MMIOREG_ENG_STATUS;

    
    if (!mmio2d) 
    {
        par->CMDQInfo.ulCMDQType = VM_CMD_QUEUE;    
        par->CMDQInfo.pjCMDQVirtualAddr = info->screen_base + par->CMDQInfo.ulCMDQOffsetAddr;
        par->CMDQInfo.ulCurCMDQueueLen = par->CMDQInfo.ulCMDQSize - CMD_QUEUE_GUARD_BAND;
        par->CMDQInfo.ulCMDQMask = par->CMDQInfo.ulCMDQSize - 1 ; 
    }
    else if (mmio2d)
    {        
        par->CMDQInfo.ulCMDQType = VM_CMD_MMIO;        
    }
       
    return true;
}

bool bCRInitCMDQInfo(struct fb_info *info)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;

    par->CMDQInfo.pjCmdQCtrlPort    = CR_CTRL; 
    par->CMDQInfo.pjCmdQBasePort    = CR_BUFFER_START; 
    par->CMDQInfo.pjWritePort       = CR_BUFFER_WRITEPORT;
    par->CMDQInfo.pjReadPort        = CR_BUFFER_READPORT;
    par->CMDQInfo.pjCmdQEndPort     = CR_BUFFER_END; 
    par->CMDQInfo.pjEngStatePort    = CR_ENG_STATUS;

    
    if (!mmio2d) 
    {
        par->CMDQInfo.ulCMDQType = VM_CMD_QUEUE;    
        par->CMDQInfo.pjCMDQVirtualAddr = info->screen_base + par->CMDQInfo.ulCMDQOffsetAddr;
        par->CMDQInfo.ulCurCMDQueueLen = par->CMDQInfo.ulCMDQSize - CMD_QUEUE_GUARD_BAND;
        par->CMDQInfo.ulCMDQMask = par->CMDQInfo.ulCMDQSize - 1 ; 
    }
    else if (mmio2d)
    {        
        par->CMDQInfo.ulCMDQType = VM_CMD_MMIO;        
    }

       
    return true;
}

bool bEnableCMDQ(struct rdcfb_info *par)
{
    ULONG ulVMCmdQBasePort = 0;
    
    vWaitEngIdle(par);  

    
    switch (par->CMDQInfo.ulCMDQType)
    {
    case VM_CMD_QUEUE:
        ulVMCmdQBasePort  = par->CMDQInfo.ulCMDQOffsetAddr >> 3;
 
        
        ulVMCmdQBasePort |= 0xF0000000;               

        
        switch (par->CMDQInfo.ulCMDQSize)
        {
        case CMD_QUEUE_SIZE_256K:
            ulVMCmdQBasePort |= 0x00000000;   
            break;

        case CMD_QUEUE_SIZE_512K:
            ulVMCmdQBasePort |= 0x04000000;   
            break;
      
        case CMD_QUEUE_SIZE_1M:
            ulVMCmdQBasePort |= 0x08000000;       
            break;
            
        case CMD_QUEUE_SIZE_2M:
            ulVMCmdQBasePort |= 0x0C000000;       
            break;        
            
        default:
            return false;
            break;
        }     
                                 
        *(ULONG *) (par->CMDQInfo.pjCmdQBasePort) = ulVMCmdQBasePort;         
        par->CMDQInfo.ulWritePointer = *(ULONG *) (par->CMDQInfo.pjWritePort) << 3;                 
        break;
        
    case VM_CMD_MMIO:
        
        ulVMCmdQBasePort |= 0xF0000000;               
        ulVMCmdQBasePort |= 0x02000000;            
        *(ULONG *) (par->CMDQInfo.pjCmdQBasePort) = ulVMCmdQBasePort;                                
        break;
        
    default:
        return false;
        break;
    }

    return true;
}

bool bCREnableCMDQ(struct rdcfb_info *par)
{
    ULONG ulVMCmdQCtrlPort = 0;
    
    vCRWaitEngIdle(par);  

    
    ulVMCmdQCtrlPort = CRCTRL_ENABLE | (CRCTRL_THRESHOLD << 8);

    
    switch (par->CMDQInfo.ulCMDQType)
    {
        case VM_CMD_QUEUE:
            *(ULONG *) (par->CMDQInfo.pjCmdQBasePort) = par->CMDQInfo.ulCMDQOffsetAddr;
            *(ULONG *) (par->CMDQInfo.pjCmdQEndPort) = par->CMDQInfo.ulCMDQOffsetAddr + DEFAULT_CMDQ_SIZE - 8;
            *(ULONG *) (par->CMDQInfo.pjCmdQCtrlPort) = ulVMCmdQCtrlPort;

            par->CMDQInfo.ulWritePointer = *(ULONG *) (par->CMDQInfo.pjWritePort) << 3;                 
            break;
            
        case VM_CMD_MMIO:
            ulVMCmdQCtrlPort &= ~(CRCTRL_ENABLE);

            *(ULONG *) (par->CMDQInfo.pjCmdQCtrlPort) = ulVMCmdQCtrlPort;
            break;
            
        default:
            return false;
            break;
    }

    return true;
}


u32 query_vesa_mode_serial(CBIOS_Extension *pCBiosExtension, u16 u16serial_num)
{
    CBIOS_ARGUMENTS *pCBiosArguments = &(pCBiosExtension->CBiosArguments);
    pCBiosArguments->reg.x.AX = OEMFunction;
    pCBiosArguments->reg.x.BX = QuerySupportedMode;
    pCBiosArguments->reg.x.CX = u16serial_num;

    CInt10(pCBiosExtension);

    return pCBiosArguments->reg.x.AX;
}


bool find_modedb_item(struct fb_videomode *modedb, int *modedb_cnt, u32 xres, u32 yres, u32 refresh)
{
    int     i;
    
    for (i = 0; i <= *modedb_cnt; i++)
    {
        if ((refresh == modedb[i].refresh) && (xres == modedb[i].xres) && (yres == modedb[i].yres))
            return true;
    }

    return false;
}

bool build_modedb(struct fb_info *info)
{
    struct rdcfb_info   *par = (struct rdcfb_info *)info->par;
    struct fb_videomode *modedb;
    u16     u16serial_num = 0;
    int     modedb_cnt = 0;
    u32     refresh, xres, yres;

    CBIOS_Extension CBiosExtension;
    CBIOS_ARGUMENTS *pCBiosArguments = &CBiosExtension.CBiosArguments;
    
    modedb = (struct fb_videomode*)kzalloc(64 * sizeof(struct fb_videomode), GFP_KERNEL);
    if(!modedb)
    {
        return false;
    }

    CBiosExtension.IOAddress = (USHORT)(par->usrio);
    CBiosExtension.VideoVirtualAddress = (ULONG)(info->screen_base);

    
    while ((query_vesa_mode_serial(&CBiosExtension, u16serial_num) == VBEFunctionCallSuccessful) && (modedb_cnt < 64))
    {
        refresh = rdcrefresh_index[pCBiosArguments->reg.lh.CH];
        xres = pCBiosArguments->reg.ex._EDX & 0xffff;
        yres = pCBiosArguments->reg.ex._EDX >>16;
        
        if (!find_modedb_item(modedb, &modedb_cnt, xres, yres, refresh))
        {
            modedb[modedb_cnt].xres = xres;
            modedb[modedb_cnt].yres = yres;
            modedb[modedb_cnt].refresh = refresh;
            modedb_cnt++;
        }
        
        u16serial_num++;
    } 

    
    if (((par->DeviceInfo.ucDeviceID == LCD_ID)||(par->DeviceInfo.ucDeviceID == LCD2_ID)) && (modedb_cnt < 64))
    {
        pCBiosArguments->reg.x.AX = OEMFunction;
        pCBiosArguments->reg.x.BX = QueryLCDPanelSizeMode;
        pCBiosArguments->reg.lh.CL = 0;
        CInt10(&CBiosExtension);
            
        refresh = rdcrefresh_index[pCBiosArguments->reg.lh.CH];
        xres = pCBiosArguments->reg.ex._EDX & 0xffff;
        yres = pCBiosArguments->reg.ex._EDX >>16;
        
        if (!find_modedb_item(modedb, &modedb_cnt, xres, yres, refresh))
        {
            modedb[modedb_cnt].xres = xres;
            modedb[modedb_cnt].yres = yres;
            modedb[modedb_cnt].refresh = refresh;
            modedb_cnt++;
        }
    }

    par->modedb = modedb;
    par->modedb_cnt = modedb_cnt;
    return true;
}

__inline ULONG ulGetCMDQLength(struct rdcfb_info *par, u32 ulWritePointer, u32 ulCMDQMask)
{
    ULONG ulReadPointer;

    ulReadPointer  = *((volatile ULONG *)(par->CMDQInfo.pjReadPort)) & 0x0003FFFF;        

    return ((ulReadPointer << 3) - ulWritePointer - CMD_QUEUE_GUARD_BAND) & ulCMDQMask;
}

u8 *pjRequestCMDQ(struct rdcfb_info *par, u32 ulDataLen)
{
    u8      *pjBuffer;
    ULONG   i, ulWritePointer, ulCMDQMask, ulCurCMDQLen, ulContinueCMDQLen;

    ulWritePointer = par->CMDQInfo.ulWritePointer;
    ulContinueCMDQLen = par->CMDQInfo.ulCMDQSize - ulWritePointer;
    ulCMDQMask = par->CMDQInfo.ulCMDQMask;        
    
    if (ulContinueCMDQLen >= ulDataLen)
    {
                        
        if (par->CMDQInfo.ulCurCMDQueueLen < ulDataLen)
        {
            do
            {
                ulCurCMDQLen = ulGetCMDQLength(par, ulWritePointer, ulCMDQMask);
            } while (ulCurCMDQLen < ulDataLen);
            
            par->CMDQInfo.ulCurCMDQueueLen = ulCurCMDQLen;
        }
        
        pjBuffer = par->CMDQInfo.pjCMDQVirtualAddr + ulWritePointer;
        par->CMDQInfo.ulCurCMDQueueLen -= ulDataLen;            
        par->CMDQInfo.ulWritePointer = (ulWritePointer + ulDataLen) & ulCMDQMask;
        return pjBuffer;            
    }
    else
    {   

        
        if (par->CMDQInfo.ulCurCMDQueueLen < ulContinueCMDQLen)
        {
            do
            {
                ulCurCMDQLen = ulGetCMDQLength(par, ulWritePointer, ulCMDQMask);
            } while (ulCurCMDQLen < ulContinueCMDQLen);
            
            par->CMDQInfo.ulCurCMDQueueLen = ulCurCMDQLen;
        }
    
        pjBuffer = par->CMDQInfo.pjCMDQVirtualAddr + ulWritePointer;
        
        for (i = 0; i<ulContinueCMDQLen/8; i++, pjBuffer+=8)
        {
            *(ULONG *)pjBuffer = (ULONG) PKT_NULL_CMD;
            *(ULONG *) (pjBuffer+4) = 0;
        }
        
        par->CMDQInfo.ulCurCMDQueueLen -= ulContinueCMDQLen;
        par->CMDQInfo.ulWritePointer = ulWritePointer = 0;
            
            
        if (par->CMDQInfo.ulCurCMDQueueLen < ulDataLen)
        {
            do
            {
                ulCurCMDQLen = ulGetCMDQLength(par, ulWritePointer, ulCMDQMask);
            } while (ulCurCMDQLen < ulDataLen);

            par->CMDQInfo.ulCurCMDQueueLen = ulCurCMDQLen;
        }

        par->CMDQInfo.ulCurCMDQueueLen -= ulDataLen;
        pjBuffer = par->CMDQInfo.pjCMDQVirtualAddr + ulWritePointer;
        par->CMDQInfo.ulWritePointer = (ulWritePointer + ulDataLen) & ulCMDQMask;
        return pjBuffer;            
    }
   
} 


static int rdcfb_sync(struct fb_info *info)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;

    PRINTK(KERN_INFO "rdcfb: rdcfb_sync()\n");

    
    par->CMDQInfo.WaitEngIdle(par);
    return 0;
}

static void rdcfb_imageblit(struct fb_info *info, const struct fb_image *image)
{
    PRINTK(KERN_INFO "rdcfb: imageblit(dx=%d, dy=%d, w=%d, h=%d, fg=0x%08x, bg=0x%08x, depth=%d\n", image->dx,image->dy, image->width,image->height,image->fg_color,image->bg_color,image->depth);
    cfb_imageblit(info, image);
}

static void rdcfb_copyarea(struct fb_info *info, const struct fb_copyarea *area)
{
    PKT_SC  *pSingleCMD;
    u32 dy = area->dy, sy = area->sy, direction = 0x0;
    u32 sx = area->sx, dx = area->dx, width = area->width, height = area->height;
    int pitch;
    u32 color_mode;
    u32 cmdreg;
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;

    PRINTK(KERN_INFO "rdcfb: copyarea(dx=%d, dy=%d, w=%d, h=%d, sx=%d, sy=%d\n", area->dx,area->dy, area->width,area->height,area->sx,area->sy);

    if (!width || !height)
        return;

    if (sy < dy) {
        dy += height - 1;
        sy += height - 1;
        direction |= 0x100000;
    }

    if (sx < dx) {
        dx += width - 1;
        sx += width - 1;
        direction |= 0x200000;
    }

    switch (info->var.bits_per_pixel)
    {
        case 8:
            color_mode = 0x00;
            break;
        case 16:
            color_mode = 0x10;
            break;
        case 24:
        case 32:
            color_mode = 0x20;
            break;
        default:
            return;
    }

    pitch = info->fix.line_length;

    cmdreg = direction | color_mode | (0xCC << 8);
    
    if (!mmio2d)
    {
        pSingleCMD = (PKT_SC *) pjRequestCMDQ(par, PKT_SINGLE_LENGTH*8);

        RDCSetupSRCBase(pSingleCMD, 0x0);
        pSingleCMD++;
        RDCSetupDSTBase(pSingleCMD, 0x0);
        pSingleCMD++;
        RDCSetupSRCPitch(pSingleCMD, pitch);
        pSingleCMD++;    
        RDCSetupDSTPitchHeight(pSingleCMD, pitch, -1);
        pSingleCMD++;    
        RDCSetupSRCXY(pSingleCMD, sx, sy);
        pSingleCMD++;    
        RDCSetupDSTXY(pSingleCMD, dx, dy);
        pSingleCMD++;    
        RDCSetupRECTXY(pSingleCMD, width, height);
        pSingleCMD++;    
        RDCSetupCMDReg(pSingleCMD, cmdreg);        
      
        
        mUpdateWritePointer;

    }
    else
    {
        
        par->CMDQInfo.WaitEngIdle(par);

        
        writel(0x0, MMIOREG_SRC_BASE);
        
        
        writel(0x0, MMIOREG_DST_BASE);
        
        
        writel(pitch, MMIOREG_SRC_PITCH);
        
        
        writel((pitch << 16) | 0x7FF, MMIOREG_DST_PITCH);
                     
        
        writel((sx << 16) | sy,  MMIOREG_SRC_XY);
        
        
        writel((dx << 16) | dy,  MMIOREG_DST_XY);
        
        
        writel((width << 16) | height, MMIOREG_RECT_XY);

        
        writel(cmdreg, MMIOREG_CMD);
    }

}

static void rdcfb_iplan_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;
    u32     dwColorDepth;
    u32     rop;
    u32     dstbase, col;
    u32     dwDstHeight = info->var.height;
    PKT_SC  *pSingleCMD;
    u32     cmdreg;

    PRINTK(KERN_INFO "rdcfb: rdcfb_iplan_fillrect()\n");

    if (info->var.bits_per_pixel == 8)
        dwColorDepth = 0x00;
    if (info->var.bits_per_pixel == 16)
        dwColorDepth = 0x10;
    else if (info->var.bits_per_pixel == 24 || info->var.bits_per_pixel == 32)
        dwColorDepth = 0x20;

    switch (rect->rop)
    {
        case ROP_XOR:
            rop = 0x5A;
            break;
        case ROP_COPY:
        default:
            rop = 0xF0;
            break;
    }

    switch (info->var.bits_per_pixel)
    {
        case 8:
            col = rect->color;
            break;
        case 16:
        case 32:
            col = ((u32 *) (info->pseudo_palette))[rect->color];
            break;
    }

    dstbase = info->var.xoffset * info->var.bits_per_pixel + info->var.yoffset * info->fix.line_length;
    
    cmdreg = dwColorDepth | (rop << 8);

    if (!mmio2d)
    {
        pSingleCMD = (PKT_SC *) pjRequestCMDQ(par, PKT_SINGLE_LENGTH*6);

        RDCSetupDSTPitchHeight(pSingleCMD, info->fix.line_length, -1);
        pSingleCMD++;
        RDCSetupFG(pSingleCMD, col);
        pSingleCMD++;
        RDCSetupDSTBase(pSingleCMD, dstbase);
        pSingleCMD++;    
        RDCSetupDSTXY(pSingleCMD, rect->dx, rect->dy);
        pSingleCMD++;    
        RDCSetupRECTXY(pSingleCMD, rect->width, rect->height);
        pSingleCMD++;    
        RDCSetupCMDReg(pSingleCMD, cmdreg);        
      
        
        mUpdateWritePointer;

    }
    else
    {
        
        par->CMDQInfo.WaitEngIdle(par);

        writel(dstbase, MMIOREG_DST_BASE);
        PRINTK(KERN_INFO " *MMIOREG_DST_BASE = 0x%08x\n", info->var.xoffset * info->var.bits_per_pixel + info->var.yoffset * info->fix.line_length);

        writel((info->fix.line_length << 16) | (dwDstHeight & 0xFFFF), MMIOREG_DST_PITCH);
        PRINTK(KERN_INFO " *MMIOREG_DST_PITCH = 0x%08x\n", ((info->fix.line_length) << 16) | (dwDstHeight & 0xFFFF));

        writel((rect->dx << 16) | (rect->dy & 0xFFFF), MMIOREG_DST_XY);
        PRINTK(KERN_INFO " *MMIOREG_DST_XY = 0x%08x\n", ((rect->dx << 16) | (rect->dy & 0xFFFF)));

        writel((rect->width << 16) | (rect->height & 0xFFFF), MMIOREG_RECT_XY);
        PRINTK(KERN_INFO " *MMIOREG_RECT_XY = 0x%08x\n", ((rect->width << 16) | (rect->height & 0xFFFF)));

        writel(col, MMIOREG_FG);
        PRINTK(KERN_INFO " *MMIOREG_FG = 0x%08x\n", col);

        writel(cmdreg, MMIOREG_CMD);                                          
        PRINTK(KERN_INFO " *MMIOREG_CMD = 0x%08x\n", cmdreg);
    }
}

static void rdcfb_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
    PRINTK(KERN_INFO "rdcfb: fillrect(dx=%d, dy=%d, w=%d, h=%d, color=0x%08x, rop=0x%08x\n", rect->dx,rect->dy, rect->width,rect->height,rect->color,rect->rop);

    
    if (info->var.bits_per_pixel == 8 ||
        info->var.bits_per_pixel == 16 || 
        info->var.bits_per_pixel == 24 || 
        info->var.bits_per_pixel == 32)
        rdcfb_iplan_fillrect(info, rect);
    else
        cfb_fillrect(info, rect);
}






static int rdcfb_open(struct fb_info *info, int user)
{
    PRINTK(KERN_INFO "rdcfb: rdcfb_open()\n");

    return 0;
}



static int rdcfb_release(struct fb_info *info, int user)
{
    PRINTK(KERN_INFO "rdcfb: rdcfb_release()\n");

    return 0;
}


static int rdcfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
    struct rdcfb_info *par = (struct rdcfb_info *)info->par;
    int var_h_total, var_v_total;
    unsigned short  usserial_num = 0;
    long            lrefresh_rate;
    unsigned char   uccolor_depth;

    CBIOS_Extension CBiosExtension;
    CBIOS_ARGUMENTS *pCBiosArguments = &CBiosExtension.CBiosArguments;

    PRINTK(KERN_INFO "rdcfb: rdcfb_check_var()\n");

    PRINTK(KERN_INFO "  var->xres = %d\n", var->xres);
    PRINTK(KERN_INFO "  var->yres = %d\n", var->yres);
    PRINTK(KERN_INFO "  var->bits_per_pixel = %d\n", var->bits_per_pixel);
    PRINTK(KERN_INFO "  var->pixclock = %d\n", var->pixclock);
    
    CBiosExtension.IOAddress = (USHORT)(par->usrio);
    CBiosExtension.VideoVirtualAddress = (ULONG)(info->screen_base);

    
    if (var->vmode & FB_VMODE_INTERLACED || var->vmode & FB_VMODE_DOUBLE)
        return -EINVAL;

    
    if (24 == var->bits_per_pixel)
        var->bits_per_pixel = 32;
        
    
    if (var->bits_per_pixel != 8 && var->bits_per_pixel != 16 &&
        var->bits_per_pixel != 32)
        return -EINVAL;

    
if(0)
{
    var_h_total = var->xres + var->left_margin + var->hsync_len + var->right_margin;
    PRINTK(KERN_INFO "  var_h_total = %d\n", var_h_total);
    var_v_total = var->yres + var->upper_margin + var->vsync_len + var->lower_margin;
    PRINTK(KERN_INFO "  var_v_total = %d\n", var_v_total);
    lrefresh_rate = (long)(((((1000000000ul / (unsigned long)var->pixclock) * 1000ul) / (unsigned long)var_h_total) * 1000ul) / (unsigned long)var_v_total);
}
else
{
    lrefresh_rate = 60000;
}
    PRINTK(KERN_INFO "  lrefresh_rate = %ld\n", lrefresh_rate);

    
    do {
        pCBiosArguments->reg.x.AX = OEMFunction;
        pCBiosArguments->reg.x.BX = QuerySupportedMode;
        pCBiosArguments->reg.x.CX = usserial_num;

        CInt10(&CBiosExtension);

        PRINTK(KERN_INFO "  pCBiosArguments->reg.ex._EDX & 0x0000FFFF(x) = %d\n", (int)pCBiosArguments->reg.ex._EDX & 0x0000FFFF);
        PRINTK(KERN_INFO "  pCBiosArguments->reg.ex._EDX >> 16(y) = %d\n", (int)pCBiosArguments->reg.ex._EDX >> 16);
        PRINTK(KERN_INFO "  (__u32)pCBiosArguments->reg.lh.CL(bpp) = %d\n", (__u32)pCBiosArguments->reg.lh.CL);
        PRINTK(KERN_INFO "  abs(lrefresh_rate - rdcrefresh_index[pCBiosArguments->reg.lh.CH]) = %d\n", abs(lrefresh_rate - rdcrefresh_index[pCBiosArguments->reg.lh.CH]));
        
        
        if ((var->xres == (pCBiosArguments->reg.ex._EDX & 0x0000FFFF)) &&
            (var->yres == (pCBiosArguments->reg.ex._EDX >> 16)) &&
            (var->bits_per_pixel == (__u32)pCBiosArguments->reg.lh.CL) &&
            (abs(lrefresh_rate - rdcrefresh_index[pCBiosArguments->reg.lh.CH]) <= 330))
        {
            par->usvar_mode_num = pCBiosArguments->reg.x.BX;
            par->ucvar_rrate_index = pCBiosArguments->reg.lh.CH;
            PRINTK(KERN_INFO "  find match VESA mode 0x%x\n", par->usvar_mode_num);
            break;
        }
        usserial_num++;
    } while (pCBiosArguments->reg.x.AX == VBEFunctionCallSuccessful);

    
    if (((par->DeviceInfo.ucDeviceID == LCD_ID)||(par->DeviceInfo.ucDeviceID == LCD2_ID)) && (pCBiosArguments->reg.x.AX != VBEFunctionCallSuccessful))
    {
        uccolor_depth = 0;
        do {
            pCBiosArguments->reg.x.AX = OEMFunction;
            pCBiosArguments->reg.x.BX = QueryLCDPanelSizeMode;
            pCBiosArguments->reg.lh.CL = uccolor_depth;
            CInt10(&CBiosExtension);
            
            
            if ((var->xres == (pCBiosArguments->reg.ex._EDX & 0x0000FFFF)) &&
                (var->yres == (pCBiosArguments->reg.ex._EDX >> 16)) &&
                (var->bits_per_pixel == (__u32)pCBiosArguments->reg.lh.CL) &&
                (abs(lrefresh_rate - rdcrefresh_index[pCBiosArguments->reg.lh.CH]) <= 330))
            {
                par->usvar_mode_num = pCBiosArguments->reg.x.BX;
                par->ucvar_rrate_index = pCBiosArguments->reg.lh.CH;
                PRINTK(KERN_INFO "  find match LCD mode 0x%x\n", par->usvar_mode_num);
                break;
            }
            uccolor_depth++;
        } while (pCBiosArguments->reg.x.AX == VBEFunctionCallSuccessful);
    }

    if (pCBiosArguments->reg.x.AX != VBEFunctionCallSuccessful)
    {
        PRINTK(KERN_INFO "  return -EINVAL\n");
        return -EINVAL;
    }
    
    
    
    
    
    
    
    PRINTK(KERN_INFO "  var->xres = %d\n", var->xres);
    PRINTK(KERN_INFO "  var->xres_virtual = %d\n", var->xres_virtual);
    PRINTK(KERN_INFO "  var->yres = %d\n", var->yres);
    PRINTK(KERN_INFO "  var->yres_virtual = %d\n", var->yres_virtual);

    if (var->xres > var->xres_virtual)
    {
        var->xres_virtual = var->xres;
        info->fix.line_length = (var->xres_virtual * (info->var.bits_per_pixel >> 3) + 7) & ~7;
    }

    if (var->yres > var->yres_virtual)
        var->yres_virtual = var->yres;

    PRINTK(KERN_INFO "  var->xres_virtual = %d\n", var->xres_virtual);
    PRINTK(KERN_INFO "  var->yres_virtual = %d\n", var->yres_virtual);

    switch (var->bits_per_pixel)
    {
        case 8:
            var->red.offset = var->green.offset = var->blue.offset = 0;
            var->red.length = var->green.length = var->blue.length = 8;
            var->transp.offset = var->transp.length = 0;
            break;
        case 15:
            var->red.offset = 10;
            var->green.offset = 5;
            var->blue.offset = 0;
            var->red.length = var->green.length = var->blue.length = 5;
            var->transp.offset = var->transp.length = 0;
            break;
        case 16:
            var->red.offset = 11;
            var->green.offset = 5;
            var->blue.offset = 0;
            var->red.length = 5;
            var->green.length = 6;
            var->blue.length = 5;
            var->transp.offset = var->transp.length = 0;
            break;
        case 24:
            var->red.offset = 16;
            var->green.offset = 8;
            var->blue.offset = 0;
            var->red.length = var->green.length = var->blue.length = 8;
            var->transp.offset = var->transp.length = 0;
            break;
        case 32:
            var->red.offset = 16;
            var->green.offset = 8;
            var->blue.offset = 0;
            var->red.length = var->green.length = var->blue.length = 8;
            var->transp.offset = 24;
            var->transp.length = 8;
            break;
    }

    if (var->xoffset > (var->xres_virtual - var->xres))
        var->xoffset = var->xres_virtual - var->xres;
    if (var->yoffset > (var->yres_virtual - var->yres))
        var->yoffset = var->yres_virtual - var->yres;

    var->red.msb_right = var->green.msb_right = var->blue.msb_right =
              var->transp.msb_right = 0;

    PRINTK(KERN_INFO " rdcfb_check_var() return 0\n");
    return 0;
}



static int rdcfb_set_par(struct fb_info *info)
{
    CBIOS_Extension             CBiosExtension;
    CBIOS_ARGUMENTS             *pCBiosArguments = &CBiosExtension.CBiosArguments;
    struct rdcfb_info           *par = (struct rdcfb_info *)info->par;
    CI_STATUS                   status;

    PRINTK(KERN_INFO "rdcfb: rdcfb_set_par()\n");

    CBiosExtension.IOAddress = (USHORT)(par->usrio);
    CBiosExtension.VideoVirtualAddress = (ULONG)(info->screen_base);

    PRINTK(KERN_INFO "  par->usvar_mode_num = 0x%x\n", par->usvar_mode_num);
    

    
    par->CMDQInfo.Disable2D(par);
    
    
    pCBiosArguments->reg.x.AX = OEMFunction;
    pCBiosArguments->reg.x.BX = SetDisplay1RefreshRate;
    pCBiosArguments->reg.lh.CL = par->ucvar_rrate_index;
    status = CInt10(&CBiosExtension);

    
    pCBiosArguments->reg.x.AX = VBESetMode;
    pCBiosArguments->reg.x.BX = (0x4000 | par->usvar_mode_num);
    status = CInt10(&CBiosExtension);

    
    if ((status == ci_false) || (pCBiosArguments->reg.x.AX != VBEFunctionCallSuccessful))
    {
        PRINTK(KERN_INFO " rdcfb_set_par() return -EINVAL\n");
        return -EINVAL;
    }

    info->fix.visual = (info->var.bits_per_pixel == 8) ? FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_DIRECTCOLOR;

    
    pCBiosArguments->reg.x.AX = VBESetGetScanLineLength;
    pCBiosArguments->reg.lh.BL = 0x00;
    pCBiosArguments->reg.x.CX = (USHORT)info->var.xres_virtual;
    CInt10(&CBiosExtension);

    info->fix.line_length = (info->var.xres_virtual * (info->var.bits_per_pixel >> 3) + 7) & ~7;

    
    par->CMDQInfo.Enable2D(info);
    
    

    
    pCBiosArguments->reg.x.AX = OEMFunction;
    pCBiosArguments->reg.x.BX = QueryDisplayPathInfo;
    CInt10(&CBiosExtension);
    if (pCBiosArguments->reg.x.AX == VBEFunctionCallSuccessful)
    {
        par->DeviceInfo.ucDeviceID = (pCBiosArguments->reg.ex._EBX & 0x000F0000) >> 16;
        par->DeviceInfo.ucDisplayPath = DISP1;
        par->DeviceInfo.ScalerConfig.EnableHorScaler = ((pCBiosArguments->reg.ex._EBX & 0x00200000) ? true : false);
        par->DeviceInfo.ScalerConfig.EnableVerScaler = ((pCBiosArguments->reg.ex._EBX & 0x00100000) ? true : false);
    }
    else
    {

    }

    PRINTK(KERN_INFO " rdcfb_set_par() return 0\n");

    return 0;
}



static int rdcfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
                u_int transp, struct fb_info *info)
{
    CBIOS_Extension     CBiosExtension;
    CBIOS_ARGUMENTS     *pCBiosArguments = &CBiosExtension.CBiosArguments;
    u32                 ulARGBData;
    u32                 LUTformat;
    struct rdcfb_info   *par = (struct rdcfb_info *)info->par;

    PRINTK(KERN_INFO "rdcfb: rdcfb_setcolreg(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n", regno, red, green, blue, transp);

    CBiosExtension.IOAddress = (USHORT)(par->usrio);
    CBiosExtension.VideoVirtualAddress = (ULONG)(info->screen_base);


    
    pCBiosArguments->reg.x.AX = VBESetGetDACPaletteFormat;
    pCBiosArguments->reg.lh.BL = 0x01;
    CInt10(&CBiosExtension);

    if (pCBiosArguments->reg.x.AX != VBEFunctionCallSuccessful)
    {
        PRINTK(KERN_INFO "    pCBiosArguments->reg.x.AX(=0x%x) != VBEFunctionCallSuccessful\n", pCBiosArguments->reg.x.AX);
        return -EINVAL;
    }

    LUTformat = (u32)pCBiosArguments->reg.lh.BH;
     
    if (LUTformat == 6) 
    {
        if (info->var.bits_per_pixel == 8)
        {
            if(regno > 256) return 1;
        }
        else if((info->var.bits_per_pixel == 16) ||(info->var.bits_per_pixel == 32))
        {
            if(regno >= 16) return 1;
        }
        else
            return 1;


        switch(info->var.bits_per_pixel)
        {
            case 8:
                red &= 0xfc00;
                green &= 0xfc00;
                blue &= 0xfc00;
                red >>= 2;
                green >>= 2;
                blue >>= 2;

                ulARGBData = (red<<8) | green | (blue>>8);

                
                pCBiosArguments->reg.x.AX = VBELoadUnloadPaletteData;
                pCBiosArguments->reg.lh.BL = 0x00;
                pCBiosArguments->reg.x.CX = 0x01;
                pCBiosArguments->reg.x.DX = (USHORT)regno;
                pCBiosArguments->reg.ex._EDI = (ULONG)&ulARGBData;
                CInt10(&CBiosExtension);
                break;
            case 16:
                ((u32 *)(info->pseudo_palette))[regno] =
                    (red & 0xf800)|
                    ((green & 0xfc00) >> 5) |
                    ((blue & 0xf800) >> 11);
                break;
            case 32:

                red >>= 8;
                green >>= 8;
                blue >>= 8;
                ((u32 *)(info->pseudo_palette))[regno] =
                    (red << 16) | (green << 8) | (blue);
                break;
        }

    }
    else 
    {
        if(regno > 256)
            return 1;

        ulARGBData = (red<<8) | green | (blue>>8);
        
        pCBiosArguments->reg.x.AX = VBELoadUnloadPaletteData;
        pCBiosArguments->reg.lh.BL = 0x00;
        pCBiosArguments->reg.x.CX = 0x01;
        pCBiosArguments->reg.x.DX = (USHORT)regno;
        pCBiosArguments->reg.ex._EDI = (ULONG)&ulARGBData;
        CInt10(&CBiosExtension);
    }

    PRINTK(KERN_INFO " rdcfb_setcolreg() return 0\n");    
    
    return 0;
}



static int rdcfb_blank(int blank_mode, struct fb_info *info)
{
    CBIOS_Extension     CBiosExtension;
    CBIOS_ARGUMENTS     *pcbios_arguments = &CBiosExtension.CBiosArguments;
    struct rdcfb_info   *par = (struct rdcfb_info *)info->par;
    
    PRINTK(KERN_INFO "rdcfb: rdcfb_blank(blank_mode = %d)\n", blank_mode);

    CBiosExtension.IOAddress = (USHORT)(par->usrio);
    CBiosExtension.VideoVirtualAddress = (ULONG)(info->screen_base);

    pcbios_arguments->reg.x.AX = OEMFunction;
    pcbios_arguments->reg.x.BX = SetDevicePowerState;
    pcbios_arguments->reg.lh.CL = par->DeviceInfo.ucDeviceID;
    
    switch (blank_mode)
    {
        case FB_BLANK_UNBLANK:
            PRINTK(KERN_INFO "  FB_BLANK_UNBLANK");
            pcbios_arguments->reg.lh.DL = DPMS__ON;
            break;
        case FB_BLANK_NORMAL:
            PRINTK(KERN_INFO "  FB_BLANK_NORMAL");
            pcbios_arguments->reg.lh.DL = DPMS__ON;
            break;
        case FB_BLANK_HSYNC_SUSPEND:
            PRINTK(KERN_INFO "  FB_BLANK_HSYNC_SUSPEND");
            pcbios_arguments->reg.lh.DL = DPMS__STANDBY;
            break;
        case FB_BLANK_VSYNC_SUSPEND:
            PRINTK(KERN_INFO "  FB_BLANK_VSYNC_SUSPEND");
            pcbios_arguments->reg.lh.DL = DPMS__SUSPEND;
            break;
        case FB_BLANK_POWERDOWN:
            PRINTK(KERN_INFO "  FB_BLANK_VSYNC_SUSPEND");
            pcbios_arguments->reg.lh.DL = DPMS__OFF;
            break;
    }
    
    if (CInt10(&CBiosExtension))
    {
        if (pcbios_arguments->reg.x.AX == VBEFunctionCallSuccessful)
            return 0;
        else
            return -EINVAL;
    }
    else
        return -EINVAL;

}




static int rdcfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
    unsigned int        offset;
    unsigned long       ul1stFlippingCmdReg;
    struct rdcfb_info   *par = (struct rdcfb_info *) info->par;
    
    PRINTK(KERN_INFO "rdcfb_pan_display(var->xoffset = %d, var->yoffset = %d)\n", var->xoffset, var->yoffset);

    offset = (var->yoffset * ((var->xres_virtual * (var->bits_per_pixel >>3) +7) & (~7))) +
             var->xoffset * (var->bits_per_pixel >> 3);

    
    ul1stFlippingCmdReg = *(ULONG *)MMIOREG_1ST_FLIP & (~MASK_1ST_FLIP_BASE);

    
    ul1stFlippingCmdReg |= (offset & MASK_1ST_FLIP_BASE);

    *(ULONG *)MMIOREG_1ST_FLIP = ul1stFlippingCmdReg;

    PRINTK(KERN_INFO " rdcfb_pan_display() return 0\n");
    
    return 0;
}


static int  rdc_identification(int chip)
{
    PRINTK(KERN_INFO "rdcfb: rdc_identification()\n");
    return CHIP_UNKNOWN;
}

static int  rdcfb_set_fbinfo(struct fb_info *info)
{
    struct rdcfb_info   *par = (struct rdcfb_info *) info->par;
    int                 i;
    struct list_head    *pos;
    struct fb_modelist  *modelist;
    PRINTK("  rdcfb_set_fbinfo()\n");

    info->flags = 0
        | FBINFO_HWACCEL_IMAGEBLIT
        | FBINFO_HWACCEL_FILLRECT
        | FBINFO_HWACCEL_COPYAREA
        | FBINFO_HWACCEL_YPAN;

#if(0) 
    fb_videomode_to_modelist(vesa_modes, 34, &info->modelist);

    for(i = 0, pos=&info->modelist; i<34; i++, pos = pos->next)
    {
    modelist = (struct fb_modelist*)pos;
    PRINTK("%02d: %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
               i,
               modelist->mode.name,
               modelist->mode.refresh,
               modelist->mode.xres,
               modelist->mode.yres,
               modelist->mode.pixclock,
               modelist->mode.left_margin,
               modelist->mode.right_margin,
               modelist->mode.upper_margin,
               modelist->mode.lower_margin,
               modelist->mode.hsync_len,
               modelist->mode.vsync_len,
               modelist->mode.sync,
               modelist->mode.vmode);
    }
#endif

    info->var = rdcfb_default_var;
    info->monspecs.modedb = NULL;
    info->fbops = &rdcfb_ops;
    info->pseudo_palette = (void*) (par->pseudo_palette);
    
    info->pixmap.size = 64*1024;
    info->pixmap.buf_align = 8;
    info->pixmap.access_align = 32;
    info->pixmap.flags = FB_PIXMAP_SYSTEM;
    info->pixmap.scan_align = 1;

    strcpy(info->fix.id, rdc_names [par->chip]);
    info->fix.visual = (info->var.bits_per_pixel == 8) ? FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;
    info->fix.type = FB_TYPE_PACKED_PIXELS;
    info->fix.type_aux = 0;
    info->fix.xpanstep = 1;
    info->fix.ypanstep = 1;
    info->fix.ywrapstep = 0;
    info->fix.accel = FB_ACCEL_NONE;
    info->fix.line_length = (info->var.xres_virtual * (info->var.bits_per_pixel >> 3) + 7) & ~7;

    return 0;
}


static int  rdc_pci_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    struct fb_info      *info;
    struct rdcfb_info   *par;
    int                 rc = 0;
    CBIOS_Extension     CBiosExtension;
    CBIOS_ARGUMENTS     *pCBiosArguments = &CBiosExtension.CBiosArguments;
//    FILE                *fpVBIOS;
//    int                 i;
    u16                 cmd;

    PRINTK(KERN_INFO "rdcfb: rdc_pci_probe()\n");

    
    if (! svga_primary_device(dev)) {
        dev_info(&(dev->dev), "rdcfb: ignoring secondary device\n");
        return -ENODEV;
    }

    
    info = framebuffer_alloc(sizeof(struct rdcfb_info), &(dev->dev));
    
    if (!info)
    {
        dev_err(&(dev->dev), "rdcfb: cannot allocate memory\n");
        return -ENOMEM;
    }

    par = (struct rdcfb_info *)info->par;
    mutex_init(&par->open_lock);

    
    info->pixmap.addr = kzalloc(64 * 1024, GFP_KERNEL);
    
    if (info->pixmap.addr == NULL)
    {
        dev_err(info->device, "Cannot reserve pixmap memory.\n");
        goto err_alloc_pixmap;
    }

    
    rc = pci_enable_device(dev);
    
    if (rc < 0)
    {
        dev_err(info->device, "rdcfb: cannot enable PCI device\n");
        goto err_enable_device;
    }

    rc = pci_request_regions(dev, "rdcfb");
    if (rc < 0) {
        dev_err(info->device, "rdcfb: cannot reserve framebuffer region\n");
        goto err_request_regions;
    }

    
    info->fix.smem_start = pci_resource_start(dev, 0);
    info->fix.smem_len = info->screen_size = pci_resource_len(dev, 0) - CAPTURE_RESERVED;

    
    pci_read_config_word(dev, PCI_COMMAND, &cmd);
    cmd |= (PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
    pci_write_config_word(dev, PCI_COMMAND, cmd);

    
    info->screen_base = pci_iomap(dev, 0, 0);
    if (! info->screen_base)
    {
        rc = -ENOMEM;
        dev_err(info->device, "rdcfb: iomap for framebuffer failed\n");
        goto err_fb_map;
    }

    
    info->fix.mmio_start = pci_resource_start(dev, 1);
    info->fix.mmio_len = pci_resource_len(dev, 1);

    
    par->mmio_virt = pci_iomap(dev, 1, 0);
    if (! par->mmio_virt)
    {
        rc = -ENOMEM;
        dev_err(info->device, "rdcfb: iomap for framebuffer failed\n");
        goto err_mmio_map;
    }

    
    par->bios_virt = NULL;
    par->u32ROMType = 0;
    
    
    if (!par->bios_virt)
    {
        if (par->bios_virt = ioremap(0xC0000, BIOS_ROM_SIZE))
        {                                                                                                                                                                                                                                                                                                                                    
            PRINTK (KERN_INFO " par->bios_virt = 0x%08x\n", (int)par->bios_virt);
            par->u32ROMType = 1;
            if (!((*(u16*)par->bios_virt == 0xAA55) && (*(u16*)(par->bios_virt+0x40) == PCI_VENDOR_RDC)))
            {
                PRINTK (KERN_INFO " Not RDC VBIOS\n");
                pci_iounmap(dev, par->bios_virt);
                par->bios_virt = NULL;
                par->u32ROMType = 0;
            }                                                                                                                                                
        }
    }


    
    par->usrio = pci_resource_start(dev, 2);
    PRINTK(KERN_INFO "  par->usrio = 0x%x\n", par->usrio);

    
    CBiosExtension.IOAddress = (USHORT)(par->usrio);
    pCBiosArguments->reg.x.AX = OEMFunction;
    pCBiosArguments->reg.x.BX = CINT10DataInit;
    pCBiosArguments->reg.ex._ECX = (unsigned long)par->bios_virt;
    CInt10(&CBiosExtension);

    
    par->CMDQInfo.ulCMDQSize = DEFAULT_CMDQ_SIZE;
    info->fix.smem_len = info->screen_size = info->screen_size - par->CMDQInfo.ulCMDQSize;
    par->CMDQInfo.ulCMDQOffsetAddr = info->fix.smem_len;
    par->CMDQInfo.ulCMDQType = VM_CMD_QUEUE;


    par->chip = id->driver_data & CHIP_MASK;

    if (par->chip & CHIP_M2012)//(par->ENGCaps & ENG_CAP_CR_SUPPORT)
    {
        par->CMDQInfo.Disable2D = vCRDisable2D;
        par->CMDQInfo.Enable2D = vCREnable2D ;
        par->CMDQInfo.WaitEngIdle = vCRWaitEngIdle;
    }
    else
    {
        par->CMDQInfo.Disable2D = vDisable2D;
        par->CMDQInfo.Enable2D = vEnable2D;
        par->CMDQInfo.WaitEngIdle = vWaitEngIdle;
    }

    
    par->rev = 0;
    if (par->chip & CHIP_UNDECIDED_FLAG)
        par->chip = rdc_identification(par->chip);


    rdcfb_set_fbinfo(info);

    pCBiosArguments->reg.x.AX = OEMFunction;
    pCBiosArguments->reg.x.BX = QueryDisplayPathInfo;
    CInt10(&CBiosExtension);

    par->DeviceInfo.ucDeviceID = par->DeviceInfo.ucNewDeviceID = (u8)((pCBiosArguments->reg.ex._EBX & 0xf0000) >>16);
    PRINTK(KERN_INFO "  pCBiosArguments->reg.ex._EBX = 0x%x\n", pCBiosArguments->reg.ex._EBX);
    par->DeviceInfo.ucDisplayPath = DISP1;

    
    build_modedb(info);

    rc = fb_find_mode(&(info->var), info, mode_option, par->modedb, par->modedb_cnt, NULL, 8);
    if (! ((rc == 1) || (rc == 2)))
    {
        dev_err(info->device, "mode %s not found\n", mode_option);
        goto err_find_mode;
    }

    
    rc = fb_alloc_cmap(&info->cmap, 256, 0);
    if (rc < 0) {
        dev_err(info->device, "cannot allocate colormap\n");
        goto err_alloc_cmap;
    }

    
    rc = register_framebuffer(info);
    if (rc < 0) {
        dev_err(info->device, "cannot register framebuffer\n");
        goto err_reg_fb;
    }

    PRINTK(KERN_INFO "rdcfb: fb%d, %s on %s, %d MB RAM\n", info->node, info->fix.id,
         pci_name(dev), info->fix.smem_len >> 20);

    if (par->chip == CHIP_UNKNOWN)
        PRINTK(KERN_INFO "rdcfb: fb%d, unknown chip\n",    info->node);

    
    pci_set_drvdata(dev, info);

#ifdef CONFIG_MTRR
    PRINTK(KERN_INFO "rdcfb: CONFIG_MTRR is defined\n");

    if (mtrr) {
        par->mtrr_reg = -1;
        par->mtrr_reg = mtrr_add(info->fix.smem_start, info->fix.smem_len, MTRR_TYPE_WRCOMB, 1);
    }
#endif

    return 0;

    
err_reg_fb:
    fb_dealloc_cmap(&info->cmap);
err_find_mode:
err_alloc_cmap:
    if (par->u32ROMType == 1)
        iounmap(par->bios_virt);
    else if (par->u32ROMType == 2)
        kfree(par->bios_virt);
//err_bios_map:
    pci_iounmap(dev, par->mmio_virt);
err_mmio_map:
    pci_iounmap(dev, info->screen_base);
err_fb_map:
    pci_release_regions(dev);
err_request_regions:

err_enable_device:
    kfree(info->pixmap.addr);
err_alloc_pixmap:
    framebuffer_release(info);
    return rc;
}




static void  rdc_pci_remove(struct pci_dev *dev)
{
    PRINTK(KERN_INFO "rdcfb: rdc_pci_remove()\n");
}



static int rdc_pci_suspend(struct pci_dev* dev, pm_message_t state)
{
    PRINTK(KERN_INFO "rdcfb: rdc_pci_suspend()\n");
    return 0;
}




static int rdc_pci_resume(struct pci_dev* dev)
{
    PRINTK(KERN_INFO "rdcfb: rdc_pci_resume()\n");
    return 0;
}


MODULE_DEVICE_TABLE(pci, rdc_devices);




#ifndef MODULE
static int  __init rdcfb_setup(char *options)
{
    char *opt;

    if (!options || !*options)
        return 0;

    PRINTK(KERN_INFO "rdcfb: opetions = %s\n", options);
    
    while ((opt = strsep(&options, ",")) != NULL) {

        if (!*opt)
            continue;
#ifdef CONFIG_MTRR
        else if (!strncmp(opt, "mtrr:", 5))
            mtrr = simple_strtoul(opt + 5, NULL, 0);
#endif
        else if (!strncmp(opt, "mmio2d", 6))
            mmio2d = true;
        else if (!strncmp(opt, "mode=", 5))
            mode_option = get_opt_string(opt, "mode=");
    }

    return 0;
}
#endif



static void __exit rdcfb_cleanup(void)
{
    PRINTK(KERN_INFO "rdcfb: rdcfb_cleanup()\n");
    pci_unregister_driver(&rdcfb_pci_driver);
}



static int __init rdcfb_init(void)
{
#ifndef MODULE
    char *option = NULL;

    if (fb_get_options("rdcfb", &option))
        return -ENODEV;
    rdcfb_setup(option);
#endif
    PRINTK(KERN_INFO "rdcfb: rdcfb_init()\n");

    return pci_register_driver(&rdcfb_pci_driver);
}





module_init(rdcfb_init);
module_exit(rdcfb_cleanup);

