// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#pragma once
#include "DEV9.h"

constexpr u32 DMA_STEP  = 0x1'0000;
constexpr u32 COMP_STEP = 0x2'0000;
constexpr u32 STEP1_COMP      = COMP_STEP;
constexpr u32 STEP2_CMD = 0x0100;
constexpr u32 STEP2_WRITE_DMA = DMA_STEP | STEP2_CMD;
constexpr u32 STEP2_COMP      = COMP_STEP | STEP2_CMD;
constexpr u32 STEP3_CMD = 0x0200;
constexpr u32 STEP3_READ_DMA = DMA_STEP | STEP3_CMD;
constexpr u32 STEP3_COMP     = COMP_STEP | STEP3_CMD;

u16 dvrp_read(u32 addr, int width);
void dvrp_write(u32 addr, u16 value, int width);

void dvrp_readDMA8Mem(u32* pMem, const int size);
void dvrp_writeDMA8Mem(u32* pMem, const int size);
void dvrp_handle_func(const u16 cmd, const u8 type, u32* pMem, const int size);

typedef struct
{
	u8 cmd_in_idx;
	u16 cmd_in_data[8];
	u8 cmd_out_idx;
	u16 cmd_out_data[8];
	u16 dma_cmd;
	u16 busy_cmd;

	u32 dma_out_size;
	u8 dma_out[0x4000];
	
	u32 file_fd;
	bool root_dir;
	u32 ret_val;
	u32 read_size;
	u16 dopen_idx;
	u8 tv_channel;
} dvrpStruct;

enum DvrpFunc : u16 {
	// https://github.com/ps2dev/ps2sdk/blob/dd7c4f51/iop/dvrp/dvripl/src/dvripl.c
    dvripl_update_nop        = 0x0101,
    dvripl_update_version    = 0x0102,
    dvripl_update_config     = 0x0106,
    dvripl_update_send_block = 0x0103,
    dvripl_update_checksum   = 0x0105,
	
	// https://github.com/ps2dev/ps2sdk/blob/dd7c4f51/iop/dvrp/dvrfile/src/dvrfile.c
    dvrf_chdir    = 0x1101,
    dvrf_chstat   = 0x1102,
    dvrf_close    = 0x1103,
    dvrf_dclose   = 0x1104,
    dvrf_devctl   = 0x1105,
    dvrf_dopen    = 0x1106,
    dvrf_dread    = 0x1107,
    dvrf_format   = 0x1108,
    dvrf_getstat  = 0x1109,
    dvrf_ioctl    = 0x110a,
    dvrf_ioctl2   = 0x110b,
    dvrf_lseek    = 0x110c,
    dvrf_lseek64  = 0x110d,
    dvrf_mkdir    = 0x110e,
    dvrf_mount    = 0x110f,
    dvrf_open     = 0x1110,
    dvrf_read     = 0x1111,
    dvrf_readlink = 0x1112,
    dvrf_remove   = 0x1113,
    dvrf_rename   = 0x1114,
    dvrf_rmdir    = 0x1115,
    dvrf_symlink  = 0x1116,
    dvrf_sync     = 0x1117,
    dvrf_umount   = 0x1118,
    dvrf_write    = 0x1119,
	
	// https://github.com/ps2dev/ps2sdk/blob/dd7c4f51/iop/dvrp/dvr/src/dvr.c
    dvr_rec_start           = 0x2101,
    dvr_rec_pause           = 0x2102,
    dvr_rec_stop            = 0x2103,
    dvr_rec_end_time        = 0x2104,
    dvr_get_rec_info        = 0x2105,
    dvr_get_rec_time        = 0x2106,
    dvr_get_ifo_time_entry  = 0x2107,
    dvr_get_ifo_vobu_entry  = 0x2108,
    dvr_read_resfile        = 0x2109,
    dvr_clear_resfile_flag  = 0x210a,
    dvr_rec_prohibit        = 0x210e,
    dvr_epg_test            = 0x210f,
    dvr_send_timer_event    = 0x2110,
    dvr_epg_cancel          = 0x2111,
    dvr_start_hdd_test      = 0x2112,
    dvr_stop_hdd_test       = 0x2113,
    dvr_get_hdd_test_stat   = 0x2114,
    dvr_pre_update_a        = 0x2115,
    dvr_pre_update_b        = 0x2116,
    dvr_get_rec_vro_pckn    = 0x2117,
    dvr_enc_dec_test        = 0x2118,
    dvr_make_menu           = 0x2119,
    dvr_re_enc_start        = 0x211a,
    dvr_recv_dma            = 0x211b,
    dvr_finish_auto_process = 0x211c,
    dvr_rec_pictclip        = 0x211d,
	
	// https://github.com/ps2dev/ps2sdk/blob/dd7c4f51/iop/dvrp/dvrav/src/dvrav.c
    dvrav_get_tun_offset    = 0x3102,
    dvrav_tun_offset_up     = 0x3103,
    dvrav_tun_offset_down   = 0x3104,
    dvrav_tun_scan_ch       = 0x3105,
    dvrav_get_bs_gain       = 0x3106,
    dvrav_set_preset_info   = 0x3107,
    dvrav_change_sound      = 0x3108,
    dvrav_set_d_audio_sel   = 0x3109,
    dvrav_set_d_video_sel   = 0x310a,
    dvrav_get_av_src        = 0x310b,
    dvrav_get_preset_info   = 0x310c,
    dvrav_select_position   = 0x310e,
    dvrav_position_up       = 0x310f,
    dvrav_position_down     = 0x3110,
    dvrav_get_position      = 0x3111,
    dvrav_set_position_info = 0x3112,
    dvrav_get_position_info = 0x3113,
    dvrav_tun_scan_mode     = 0x3114,
    dvrav_f_select_position = 0x3115,
    dvrav_select_rec_src    = 0x3116,
    dvrav_get_rec_src       = 0x3117,
	
	// https://github.com/ps2dev/ps2sdk/blob/dd7c4f51/iop/dvrp/dvrdv/src/dvrdv.c
    dvr_dv_dubb_start     = 0x4101,
    dvr_dv_dubb_stop      = 0x4102,
    dvr_dv_dubb_rec_start = 0x4103,
    dvr_dv_dubb_rec_stop  = 0x4104,
    dvr_get_dvcam_info    = 0x4107,
    dvr_get_dvcam_name    = 0x4108,
	
	// https://github.com/ps2dev/ps2sdk/blob/dd7c4f51/iop/dvrp/dvrmisc/src/dvrmisc.c
    dvr_nop              = 0x5101,
    dvr_version          = 0x5102,
    dvr_led_hdd_rec      = 0x5104,
    dvr_led_dvd_rec      = 0x5106,
    dvr_get_sircs        = 0x5107,
    dvr_get_time         = 0x5108,
    dvr_set_timezone     = 0x5109,
    dvr_save_preset_info = 0x510a,
    dvr_load_preset_info = 0x510b,
    dvr_test_dev_rst     = 0x510c,
    dvr_test_sdram_chk   = 0x510d,
    dvr_test_mpe_chk     = 0x510e,
    dvr_test_mpd_chk     = 0x510f,
    dvr_test_vdec_chk    = 0x5110,
    dvr_buzzer           = 0x5111,
    dvr_clr_preset_info  = 0x5112,
    dvr_get_vbi_err_rate = 0x5113,
    dvr_update_dvrp_firmware_FLASH_DATA_TOTALSIZE        = 0x5114,
    dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_DOWNLOAD = 0x5115,
    dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_CHECKSUM = 0x5116,
    dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_WRITE    = 0x5117,
    dvr_flash_write_status                      = 0x5118,
    dvr_set_device_key_MISCCMD_SAVE_DEVKEY_INFO = 0x5119,
    dvr_get_device_key_MISCCMD_GET_DEVKEY_INFO  = 0x511a,
    dvr_set_device_key_DEVKEY_TOTALSIZE         = 0x511b,
    dvr_set_device_key_MISCCMD_DEVKEY_DOWNLOAD  = 0x511c,
    dvr_set_dv_nodeid_MISCCMD_SAVE_DV_NODEID    = 0x511d,
    dvr_get_dv_nodeid_MISCCMD_GET_DV_NODEID     = 0x511e,
    dvr_diag_test        = 0x511f
};
