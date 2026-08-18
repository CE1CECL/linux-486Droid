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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/svga.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/console.h> 
#include <video/vga.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#include "typedef.h"
#include "CInt10.h"

#define     RDCFB_DEBUG             1

#if RDCFB_DEBUG
    #define PRINTK(fmt, args...)    printk(fmt, ##args)
#else
    #define PRINTK(fmt, args...)
#endif


#define MASK_1ST_FLIP_BASE          0x0FFFFFF8

  

#define MMIOREG_SRC_BASE    (par->mmio_virt + 0x8000)                
#define MMIOREG_SRC_PITCH   (par->mmio_virt + 0x8004)
#define MMIOREG_DST_BASE    (par->mmio_virt + 0x8008)
#define MMIOREG_DST_PITCH   (par->mmio_virt + 0x800C)
#define MMIOREG_DST_XY      (par->mmio_virt + 0x8010)
#define MMIOREG_SRC_XY      (par->mmio_virt + 0x8014)
#define MMIOREG_RECT_XY     (par->mmio_virt + 0x8018)
#define MMIOREG_FG          (par->mmio_virt + 0x801C)
#define MMIOREG_BG          (par->mmio_virt + 0x8020)
#define MMIOREG_FG_SRC      (par->mmio_virt + 0x8024)
#define MMIOREG_BG_SRC      (par->mmio_virt + 0x8028)
#define MMIOREG_MONO1       (par->mmio_virt + 0x802C)
#define MMIOREG_MONO2       (par->mmio_virt + 0x8030)
#define MMIOREG_CLIP1       (par->mmio_virt + 0x8034)
#define MMIOREG_CLIP2       (par->mmio_virt + 0x8038)
#define MMIOREG_CMD         (par->mmio_virt + 0x803C)
#define MMIOREG_1ST_FLIP    (par->mmio_virt + 0x8040)
#define MMIOREG_CMDQ_SET    (par->mmio_virt + 0x8044)
#define MMIOREG_WRITE_PTR   (par->mmio_virt + 0x8048)
#define MMIOREG_ENG_STATUS  (par->mmio_virt + 0x804C)
#define MMIOREG_CFC         (par->mmio_virt + 0x8050)

#define MMIOREG_PAT         (par->mmio_virt + 0x8100)    


#define CR_CTRL                     (par->mmio_virt + 0x400)
#define CRCTRL_ENABLE               BIT0
#define CRCTRL_RESET                BIT1
#define CRCTRL_VPOSTMMIO            BIT2
#define CRCTRL_DMAMMIO              BIT3
#define CRCTRL_VDISPMMIO            BIT4
#define CRCTRL_READPORT_DATABACK    BIT7
#define CRCTRL_THRESHOLD            0x20
#define CR_BUFFER_START             (par->mmio_virt + 0x404)
#define CR_BUFFER_WRITEPORT         (par->mmio_virt + 0x408)
#define CR_BUFFER_READPORT          (par->mmio_virt + 0x40C)
#define CR_BUFFER_END               (par->mmio_virt + 0x410)
#define CR_ENG_STATUS               (par->mmio_virt + 0x414)
#define CRENG_CRRESET               BIT11
#define CRENG_2DIDLE                BIT12
#define CRENG_DMAIDLE               BIT13
#define CRENG_VPOSTIDEL             BIT14
#define CRENG_CRIDLE                BIT15


#define PCI_VENDOR_RDC      0x17F3

#define CHIP_UNKNOWN        0x00
#define CHIP_M2010          0x01
#define CHIP_M2012          0x02

#define CHIP_MASK           0xFF
#define CHIP_UNDECIDED_FLAG 0x80


#define CMD_QUEUE_SIZE_256K     0x00040000
#define CMD_QUEUE_SIZE_512K     0x00080000
#define CMD_QUEUE_SIZE_1M       0x00100000
#define CMD_QUEUE_SIZE_2M       0x00200000
#define DEFAULT_CMDQ_SIZE       CMD_QUEUE_SIZE_1M

#define CMD_QUEUE_GUARD_BAND    0X20

#define VM_CMD_QUEUE            0
#define VM_CMD_MMIO             2

#define PKT_NULL_CMD            0x00009561
#define PKT_SINGLE_LENGTH       8
#define PKT_SINGLE_CMD_HEADER   0x00009562

#define CMDQREG_SRC_BASE        (0x00 << 24)                       
#define CMDQREG_SRC_PITCH       (0x01 << 24)
#define CMDQREG_DST_BASE        (0x02 << 24)
#define CMDQREG_DST_PITCH       (0x03 << 24)
#define CMDQREG_DST_XY          (0x04 << 24)
#define CMDQREG_SRC_XY          (0x05 << 24)
#define CMDQREG_RECT_XY         (0x06 << 24)
#define CMDQREG_FG              (0x07 << 24)
#define CMDQREG_BG              (0x08 << 24)
#define CMDQREG_FG_SRC          (0x09 << 24)
#define CMDQREG_BG_SRC          (0x0A << 24)
#define CMDQREG_MONO1           (0x0B << 24)
#define CMDQREG_MONO2           (0x0C << 24)
#define CMDQREG_CLIP1           (0x0D << 24)
#define CMDQREG_CLIP2           (0x0E << 24)
#define CMDQREG_CMD             (0x0F << 24)
#define CMDQREG_PAT             (0x40 << 24)
#define CMDQREQ_2DFENCE         (0x16 << 24)


#define MAX_SRC_X               0x7FF
#define MAX_SRC_Y               0x7FF
#define MAX_DST_X               0x7FF
#define MAX_DST_Y               0x7FF

#define MASK_SRC_PITCH          0x1FFF
#define MASK_DST_PITCH          0x1FFF
#define MASK_DST_HEIGHT         0x7FF
#define MASK_SRC_X              0xFFF
#define MASK_SRC_Y              0xFFF
#define MASK_DST_X              0xFFF
#define MASK_DST_Y              0xFFF
#define MASK_RECT_WIDTH         0x7FF
#define MASK_RECT_HEIGHT        0x7FF
#define MASK_CLIP               0xFFF
#define MASK_1ST_FLIP_BASE      0x0FFFFFF8

typedef struct  _PKT_SC
{
    ULONG    PKT_SC_dwHeader;
    ULONG    PKT_SC_dwData[1];
    
} PKT_SC, *PPKT_SC;

typedef struct {

    u32     ulCMDQSize;
    u32     ulCMDQType;
    
    u32     ulCMDQOffsetAddr;
    u8      *pjCMDQVirtualAddr;
    
    u8      *pjCmdQCtrlPort;    
    u8      *pjCmdQBasePort;
    u8      *pjWritePort;
    u8      *pjReadPort;
    u8      *pjCmdQEndPort;     
    u8      *pjEngStatePort;
          
    u32     ulCMDQMask;
    u32     ulCurCMDQueueLen;
                
    u32     ulWritePointer;
    u32     ulReadPointer;
    
    u32     ulReadPointer_OK;       

    void (*Disable2D)(struct rdcfb_info *par);
    void (*Enable2D)(struct fb_info *info);
    void (*WaitEngIdle)(struct fb_info *info);

    
} CMDQINFO, *PCMDQINFO;


typedef struct {
    u32     ulHorMaxResolution;
    u32     ulVerMaxResolution;
} MONITORSIZE;

typedef struct {
    bool    EnableHorScaler;
    bool    EnableVerScaler;
    int     ulHorScalingFactor;
    int     ulVerScalingFactor;
} SCALER;

typedef struct {
    u8          ucDeviceID;     
    u8          ucDisplayPath;
    MONITORSIZE MonitorSize;
    SCALER      ScalerConfig;
    u8          ucNewDeviceID;  
    
} DEVICEINFO;

struct rdcfb_info {
    int chip, rev;
    int mtrr_reg;
    u32 u32ROMType;

    struct vgastate state;
    struct mutex open_lock;
    unsigned int ref_count;
    u32 pseudo_palette[16];
    char __iomem    *mmio_virt;     
    char __iomem    *bios_virt;     
    unsigned short  usrio;          
    DEVICEINFO      DeviceInfo;
    unsigned short  usvar_mode_num;
    unsigned char   ucvar_rrate_index;
    CMDQINFO        CMDQInfo;
    bool            b2DEnabled;
    struct fb_videomode *modedb;
    int modedb_cnt;

};

#define CAPTURE_RESERVED    (3*1024*1024)



void vEnable2D(struct fb_info *info);
void vCREnable2D(struct fb_info *info);
void vDisable2D(struct rdcfb_info *par);
void vCRDisable2D(struct rdcfb_info *par);
void vWaitEngIdle(struct rdcfb_info *par);
void vCRWaitEngIdle(struct rdcfb_info *par);
bool bInitCMDQInfo(struct fb_info *info);
bool bCRInitCMDQInfo(struct fb_info *info);
bool bEnableCMDQ(struct rdcfb_info *par);
bool bCREnableCMDQ(struct rdcfb_info *par);
u8 *pjRequestCMDQ(struct rdcfb_info *par, u32 ulDataLen);
static void rdcfb_imageblit(struct fb_info *info, const struct fb_image *image);
static void rdcfb_copyarea(struct fb_info *info, const struct fb_copyarea *area);
static void rdcfb_iplan_fillrect(struct fb_info *info, const struct fb_fillrect *rect);
static void rdcfb_fillrect(struct fb_info *info, const struct fb_fillrect *rect);
static int rdcfb_sync(struct fb_info *info);
static int rdcfb_open(struct fb_info *info, int user);
static int rdcfb_release(struct fb_info *info, int user);
static int rdcfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info);
static int rdcfb_set_par(struct fb_info *info);
static int rdcfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue, u_int transp, struct fb_info *info);
static int rdcfb_blank(int blank_mode, struct fb_info *info);
static int rdcfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
static int  rdc_identification(int chip);
static int  rdc_pci_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void  rdc_pci_remove(struct pci_dev *dev);
static int rdc_pci_suspend(struct pci_dev* dev, pm_message_t state);
static int rdc_pci_resume(struct pci_dev* dev);
static int  __init rdcfb_setup(char *options);
static void __exit rdcfb_cleanup(void);
static int __init rdcfb_init(void);

#define GetIndexReg(base,index,val)     \
    do {                                \
        outb(index, par->usrio + base);               \
        val = inb(par->usrio + base+1);              \
    } while (0)
                                        
#define SetIndexReg(base,index, val)    \
    do {                                \
        outb(index, par->usrio + base);               \
        outb(val, par->usrio + base+1);                 \
    } while (0)
                                        
#define GetIndexRegMask(base,index, and, val)   \
    do {                                        \
        outb(index, par->usrio + base);                       \
        val = (inb( par->usrio + base+1) & and);              \
    } while (0)
    
#define SetIndexRegMask(base,index, and, val)   \
    do {                                        \
        u8 __Temp;                           \
        outb(index, par->usrio + base);                       \
         __Temp = (inb((par->usrio + base)+1)&(and))|(val);  \
        SetIndexReg(base,index,__Temp);         \
    } while (0)

 
#define mUpdateWritePointer *(u32 *) (par->CMDQInfo.pjWritePort) = (par->CMDQInfo.ulWritePointer >>3)


#define RDCSetupSRCBase(addr, base) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_SRC_BASE);     \
        addr->PKT_SC_dwData[0] = (ULONG)(base);                    \
      }
#define RDCSetupSRCPitch(addr, pitch) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_SRC_PITCH);     \
        addr->PKT_SC_dwData[0] = (ULONG)(pitch << 16);                    \
      }
#define RDCSetupDSTBase(addr, base) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_DST_BASE);     \
        addr->PKT_SC_dwData[0] = (ULONG)(base);                    \
      }      
#define RDCSetupDSTPitchHeight(addr, pitch, height) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_DST_PITCH);     \
        addr->PKT_SC_dwData[0] = (ULONG)((pitch << 16) + ((height) & MASK_DST_HEIGHT));                    \
      }      
#define RDCSetupDSTXY(addr, x, y) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_DST_XY);     \
        addr->PKT_SC_dwData[0] = (ULONG)(((x & MASK_DST_X) << 16) + (y & MASK_DST_Y));                    \
      }           
#define RDCSetupSRCXY(addr, x, y) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_SRC_XY);     \
        addr->PKT_SC_dwData[0] = (ULONG)(((x & MASK_SRC_X) << 16) + (y & MASK_SRC_Y));                    \
      }             
#define RDCSetupRECTXY(addr, x, y) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_RECT_XY);     \
        addr->PKT_SC_dwData[0] = (ULONG)(((x & MASK_RECT_WIDTH) << 16) + (y & MASK_RECT_WIDTH));                    \
      }  
#define RDCSetupFG(addr, color) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_FG);     \
        addr->PKT_SC_dwData[0] = (ULONG)(color);                    \
      }
#define RDCSetupBG(addr, color) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_BG);     \
        addr->PKT_SC_dwData[0] = (ULONG)(color);                    \
      }
#define RDCSetupMONO1(addr, pat) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_MONO1);     \
        addr->PKT_SC_dwData[0] = (ULONG)(pat);                \
      }            
#define RDCSetupMONO2(addr, pat) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_MONO2);     \
        addr->PKT_SC_dwData[0] = (ULONG)(pat);                \
      }     
#define RDCSetupCLIP1(addr, left, top) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_CLIP1);     \
        addr->PKT_SC_dwData[0] = (ULONG)(((left & MASK_CLIP) << 16) + (top & MASK_CLIP));    \
      }            
#define RDCSetupCLIP2(addr, right, bottom) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_CLIP2);     \
        addr->PKT_SC_dwData[0] = (ULONG)(((right & MASK_CLIP) << 16) + (bottom & MASK_CLIP));    \
      }                                                                                                    
#define RDCSetupCMDReg(addr, reg) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + CMDQREG_CMD);     \
        addr->PKT_SC_dwData[0] = (ULONG)(reg);                    \
      }
#define RDCSetupPatReg(addr, patreg, pat) \
      { \
        addr->PKT_SC_dwHeader  = (ULONG)(PKT_SINGLE_CMD_HEADER + (CMDQREG_PAT + (patreg << 24)));     \
        addr->PKT_SC_dwData[0] = (ULONG)(pat);                \
      }    




static const char * const rdc_names[] = {"RDC Unknown", "RDC M2010", "RDC M2012"};



long    rdcrefresh_index[]=
{
    60000,
    0,
    0,
    50000,
    56000,
    59940,
    24000,
    70000,
    75000,
    80000,
    85000,
    90000,
    100000,
    120000,
    72000,
    65000
};

static char *mode_option  = "640x480-8@60";
#ifdef CONFIG_MTRR
static int mtrr  = 1;
#endif

static bool mmio2d  = false;



static struct pci_device_id rdc_devices[]  = {
    {PCI_DEVICE(PCI_VENDOR_ID_RDC, 0x2010), .driver_data = CHIP_M2010},
    {PCI_DEVICE(PCI_VENDOR_ID_RDC, 0x2012), .driver_data = CHIP_M2012},
    {0, 0, 0, 0, 0, 0, 0}
};

static struct fb_ops rdcfb_ops = {
    .owner        = THIS_MODULE,
    .fb_open    = rdcfb_open,
    .fb_release    = rdcfb_release,
    .fb_check_var    = rdcfb_check_var,
    .fb_set_par    = rdcfb_set_par,
    .fb_setcolreg    = rdcfb_setcolreg,
    .fb_blank    = rdcfb_blank,
    .fb_pan_display    = rdcfb_pan_display,
    .fb_fillrect    = rdcfb_fillrect,
    .fb_copyarea    = rdcfb_copyarea,
    .fb_imageblit    = rdcfb_imageblit,
    .fb_get_caps    = NULL, 
    .fb_sync        = rdcfb_sync
};

static struct pci_driver rdcfb_pci_driver = {
    .name        = "rdcfb",
    .id_table    = rdc_devices,
    .probe        = rdc_pci_probe,
    .remove        = (rdc_pci_remove),
    .suspend    = rdc_pci_suspend,
    .resume        = rdc_pci_resume,
};

static struct fb_var_screeninfo  rdcfb_default_var = {
	.xres		= 640,
	.yres		= 480,
	.xres_virtual	= 640,
	.yres_virtual	= 480,
	.bits_per_pixel	= 8,
	.red		= {0, 8, 0},
	.green		= {0, 8, 0},
	.blue		= {0, 8, 0},
	.transp		= {0, 0, 0},
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.pixclock	= 39721,
	.left_margin	= 40,
	.right_margin	= 24,
	.upper_margin	= 32,
	.lower_margin	= 11,
	.hsync_len	= 96,
	.vsync_len	= 2,
	.vmode		= FB_VMODE_NONINTERLACED
};


extern const struct fb_videomode vesa_modes[];

