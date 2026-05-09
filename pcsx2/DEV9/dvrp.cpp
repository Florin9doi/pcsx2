// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "dvrp.h"
#include "common/FileSystem.h"
#include "common/ByteSwap.h"
#include "Host.h"
#include "IopBios.h"

dvrpStruct dvrp;

static std::unordered_map<DvrpFunc, std::string> dvrp_funcs = {
    {DvrpFunc::dvripl_update_nop,        "dvripl_update_nop"},
    {DvrpFunc::dvripl_update_version,    "dvripl_update_version"},
    {DvrpFunc::dvripl_update_config,     "dvripl_update_config"},
    {DvrpFunc::dvripl_update_send_block, "dvripl_update_send_block"},
    {DvrpFunc::dvripl_update_checksum,   "dvripl_update_checksum"},
    {DvrpFunc::dvrf_chdir,    "dvrf_chdir"},
    {DvrpFunc::dvrf_chstat,   "dvrf_chstat"},
    {DvrpFunc::dvrf_close,    "dvrf_close"},
    {DvrpFunc::dvrf_dclose,   "dvrf_dclose"},
    {DvrpFunc::dvrf_devctl,   "dvrf_devctl"},
    {DvrpFunc::dvrf_dopen,    "dvrf_dopen"},
    {DvrpFunc::dvrf_dread,    "dvrf_dread"},
    {DvrpFunc::dvrf_format,   "dvrf_format"},
    {DvrpFunc::dvrf_getstat,  "dvrf_getstat"},
    {DvrpFunc::dvrf_ioctl,    "dvrf_ioctl"},
    {DvrpFunc::dvrf_ioctl2,   "dvrf_ioctl2"},
    {DvrpFunc::dvrf_lseek,    "dvrf_lseek"},
    {DvrpFunc::dvrf_lseek64,  "dvrf_lseek64"},
    {DvrpFunc::dvrf_mkdir,    "dvrf_mkdir"},
    {DvrpFunc::dvrf_mount,    "dvrf_mount"},
    {DvrpFunc::dvrf_open,     "dvrf_open"},
    {DvrpFunc::dvrf_read,     "dvrf_read"},
    {DvrpFunc::dvrf_readlink, "dvrf_readlink"},
    {DvrpFunc::dvrf_remove,   "dvrf_remove"},
    {DvrpFunc::dvrf_rename,   "dvrf_rename"},
    {DvrpFunc::dvrf_rmdir,    "dvrf_rmdir"},
    {DvrpFunc::dvrf_symlink,  "dvrf_symlink"},
    {DvrpFunc::dvrf_sync,     "dvrf_sync"},
    {DvrpFunc::dvrf_umount,   "dvrf_umount"},
    {DvrpFunc::dvrf_write,    "dvrf_write"},
    {DvrpFunc::dvr_rec_start,           "dvr_rec_start"},
    {DvrpFunc::dvr_rec_pause,           "dvr_rec_pause"},
    {DvrpFunc::dvr_rec_stop,            "dvr_rec_stop"},
    {DvrpFunc::dvr_rec_end_time,        "dvr_rec_end_time"},
    {DvrpFunc::dvr_get_rec_info,        "dvr_get_rec_info"},
    {DvrpFunc::dvr_get_rec_time,        "dvr_get_rec_time"},
    {DvrpFunc::dvr_get_ifo_time_entry,  "dvr_get_ifo_time_entry"},
    {DvrpFunc::dvr_get_ifo_vobu_entry,  "dvr_get_ifo_vobu_entry"},
    {DvrpFunc::dvr_read_resfile,        "dvr_read_resfile"},
    {DvrpFunc::dvr_clear_resfile_flag,  "dvr_clear_resfile_flag"},
    {DvrpFunc::dvr_rec_prohibit,        "dvr_rec_prohibit"},
    {DvrpFunc::dvr_epg_test,            "dvr_epg_test"},
    {DvrpFunc::dvr_send_timer_event,    "dvr_send_timer_event"},
    {DvrpFunc::dvr_epg_cancel,          "dvr_epg_cancel"},
    {DvrpFunc::dvr_start_hdd_test,      "dvr_start_hdd_test"},
    {DvrpFunc::dvr_stop_hdd_test,       "dvr_stop_hdd_test"},
    {DvrpFunc::dvr_get_hdd_test_stat,   "dvr_get_hdd_test_stat"},
    {DvrpFunc::dvr_pre_update_a,        "dvr_pre_update_a"},
    {DvrpFunc::dvr_pre_update_b,        "dvr_pre_update_b"},
    {DvrpFunc::dvr_get_rec_vro_pckn,    "dvr_get_rec_vro_pckn"},
    {DvrpFunc::dvr_enc_dec_test,        "dvr_enc_dec_test"},
    {DvrpFunc::dvr_make_menu,           "dvr_make_menu"},
    {DvrpFunc::dvr_re_enc_start,        "dvr_re_enc_start"},
    {DvrpFunc::dvr_recv_dma,            "dvr_recv_dma"},
    {DvrpFunc::dvr_finish_auto_process, "dvr_finish_auto_process"},
    {DvrpFunc::dvr_rec_pictclip,        "dvr_rec_pictclip"},
    {DvrpFunc::dvrav_get_tun_offset,    "dvrav_get_tun_offset"},
    {DvrpFunc::dvrav_tun_offset_up,     "dvrav_tun_offset_up"},
    {DvrpFunc::dvrav_tun_offset_down,   "dvrav_tun_offset_down"},
    {DvrpFunc::dvrav_tun_scan_ch,       "dvrav_tun_scan_ch"},
    {DvrpFunc::dvrav_get_bs_gain,       "dvrav_get_bs_gain"},
    {DvrpFunc::dvrav_set_preset_info,   "dvrav_set_preset_info"},
    {DvrpFunc::dvrav_change_sound,      "dvrav_change_sound"},
    {DvrpFunc::dvrav_set_d_audio_sel,   "dvrav_set_d_audio_sel"},
    {DvrpFunc::dvrav_set_d_video_sel,   "dvrav_set_d_video_sel"},
    {DvrpFunc::dvrav_get_av_src,        "dvrav_get_av_src"},
    {DvrpFunc::dvrav_get_preset_info,   "dvrav_get_preset_info"},
    {DvrpFunc::dvrav_select_position,   "dvrav_select_position"},
    {DvrpFunc::dvrav_position_up,       "dvrav_position_up"},
    {DvrpFunc::dvrav_position_down,     "dvrav_position_down"},
    {DvrpFunc::dvrav_get_position,      "dvrav_get_position"},
    {DvrpFunc::dvrav_set_position_info, "dvrav_set_position_info"},
    {DvrpFunc::dvrav_get_position_info, "dvrav_get_position_info"},
    {DvrpFunc::dvrav_tun_scan_mode,     "dvrav_tun_scan_mode"},
    {DvrpFunc::dvrav_f_select_position, "dvrav_f_select_position"},
    {DvrpFunc::dvrav_select_rec_src,    "dvrav_select_rec_src"},
    {DvrpFunc::dvrav_get_rec_src,       "dvrav_get_rec_src"},
    {DvrpFunc::dvr_dv_dubb_start,     "dvr_dv_dubb_start"},
    {DvrpFunc::dvr_dv_dubb_stop,      "dvr_dv_dubb_stop"},
    {DvrpFunc::dvr_dv_dubb_rec_start, "dvr_dv_dubb_rec_start"},
    {DvrpFunc::dvr_dv_dubb_rec_stop,  "dvr_dv_dubb_rec_stop"},
    {DvrpFunc::dvr_get_dvcam_info,    "dvr_get_dvcam_info"},
    {DvrpFunc::dvr_get_dvcam_name,    "dvr_get_dvcam_name"},
    {DvrpFunc::dvr_nop,              "dvr_nop"},
    {DvrpFunc::dvr_version,          "dvr_version"},
    {DvrpFunc::dvr_led_hdd_rec,      "dvr_led_hdd_rec"},
    {DvrpFunc::dvr_led_dvd_rec,      "dvr_led_dvd_rec"},
    {DvrpFunc::dvr_get_sircs,        "dvr_get_sircs"},
    {DvrpFunc::dvr_get_time,         "dvr_get_time"},
    {DvrpFunc::dvr_set_timezone,     "dvr_set_timezone"},
    {DvrpFunc::dvr_save_preset_info, "dvr_save_preset_info"},
    {DvrpFunc::dvr_load_preset_info, "dvr_load_preset_info"},
    {DvrpFunc::dvr_test_dev_rst,     "dvr_test_dev_rst"},
    {DvrpFunc::dvr_test_sdram_chk,   "dvr_test_sdram_chk"},
    {DvrpFunc::dvr_test_mpe_chk,     "dvr_test_mpe_chk"},
    {DvrpFunc::dvr_test_mpd_chk,     "dvr_test_mpd_chk"},
    {DvrpFunc::dvr_test_vdec_chk,    "dvr_test_vdec_chk"},
    {DvrpFunc::dvr_buzzer,           "dvr_buzzer"},
    {DvrpFunc::dvr_clr_preset_info,  "dvr_clr_preset_info"},
    {DvrpFunc::dvr_get_vbi_err_rate, "dvr_get_vbi_err_rate"},
    {DvrpFunc::dvr_update_dvrp_firmware_FLASH_DATA_TOTALSIZE,        "dvr_update_dvrp_firmware_FLASH_DATA_TOTALSIZE"},
    {DvrpFunc::dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_DOWNLOAD, "dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_DOWNLOAD"},
    {DvrpFunc::dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_CHECKSUM, "dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_CHECKSUM"},
    {DvrpFunc::dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_WRITE,    "dvr_update_dvrp_firmware_MISCCMD_FLASH_DATA_WRITE"},
    {DvrpFunc::dvr_flash_write_status,                      "dvr_flash_write_status"},
    {DvrpFunc::dvr_set_device_key_MISCCMD_SAVE_DEVKEY_INFO, "dvr_set_device_key_MISCCMD_SAVE_DEVKEY_INFO"},
    {DvrpFunc::dvr_get_device_key_MISCCMD_GET_DEVKEY_INFO,  "dvr_get_device_key_MISCCMD_GET_DEVKEY_INFO"},
    {DvrpFunc::dvr_set_device_key_DEVKEY_TOTALSIZE,         "dvr_set_device_key_DEVKEY_TOTALSIZE"},
    {DvrpFunc::dvr_set_device_key_MISCCMD_DEVKEY_DOWNLOAD,  "dvr_set_device_key_MISCCMD_DEVKEY_DOWNLOAD"},
    {DvrpFunc::dvr_set_dv_nodeid_MISCCMD_SAVE_DV_NODEID,    "dvr_set_dv_nodeid_MISCCMD_SAVE_DV_NODEID"},
    {DvrpFunc::dvr_get_dv_nodeid_MISCCMD_GET_DV_NODEID,     "dvr_get_dv_nodeid_MISCCMD_GET_DV_NODEID"},
    {DvrpFunc::dvr_diag_test,        "dvr_diag_test"},
};
std::string get_dvrp_name(const u16 id) {
	const DvrpFunc func = static_cast<DvrpFunc>((id & 0xf0ff) | 0x0100); // ignore the cmd phase
    auto const it = dvrp_funcs.find(func);
    if (it != dvrp_funcs.end())
        return it->second;
    return "Unknown";
}
static std::unordered_map<u16, const char*> av_presets = {
	{0x01, "ext_video_in"},
	{0x10, "auto_stereo_reception"},
	{0x11, "mute"},
	{0x12, "recorded_nr"},
	{0x13, "bilingual_recording"},
	{0x15, "ext_audio_in"},
};
const char* get_preset_name(const u16 id) {
    auto const it = av_presets.find(id);
    if (it != av_presets.end())
        return it->second;
    return "Unknown";
}

void dvrp_set_resp_count(u16 count)
{
	dev9Ru16(DVRP_RET_COUNT) = (count << 2);
	dvrp.cmd_out_idx = 0;
	std::memset(dvrp.cmd_out_data, 0x00, sizeof(dvrp.cmd_out_data));
}

void dvrp_set_resp_count_val16(u16 count, u16 retval)
{
	dvrp_set_resp_count(count);
	dvrp.cmd_out_data[1] = retval;
}

void dvrp_set_resp_count_val(u16 count, u32 retval)
{
	dvrp_set_resp_count(count);
	dvrp.cmd_out_data[1] = retval >> 16;
	dvrp.cmd_out_data[2] = retval & 0xffff;
}

void dvrp_submit_resp(u16 cmd, u16 stat)
{
	dev9Ru16(DVRP_INTR_CAUSE) = cmd;
	dev9Ru16(DVRP_INTR_STAT) = stat;
	_DEV9irq(DVRP_INTR_INTRQ, 1);
}

void dvrp_shedule_cmd_comp(const u16 cmd)
{
	dvrp.busy_cmd = cmd;
	std::jthread t([cmd] { // <<-- TODO: fix this
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		dvrp_handle_func(cmd, 2, nullptr, 0);
		dvrp.busy_cmd = 0;
	});
	t.detach();
}

std::string dvrp_get_host_path(char* path) {
	std::string guest_path = Path::SanitizeFileName(path, false);
	std::string host_path = Path::Combine(EmuFolders::AppRoot, "dvrp");
	return Path::Combine(host_path, guest_path);
}

void dvrp_handle_func(const u16 cmd, const u8 type, u32* pMem, const int size)
{
	const u32 cmd_base = (cmd & 0xf0ff) | 0x0100;
	const u32 cmd_step = (type << 16) | cmd;
	//if (cmd != dvr_get_sircs) Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_CMD=%x(%s) (type=%d/%s)",
	//	cmd, get_dvrp_name(cmd).c_str(),
	//	type,
	//	type == 0 ? "cmd"
	//	 : type == 1 ? "dma"
	//	 : type == 2 ? "timer"
	//	 :  ""
	//);
	switch (cmd_step)
	{
		default: // log unhandled commands
			if (type) break; // do not send cmd_ack for dma
			{
				u8 off = 0;
				char buf[128] = {0};
				for (int i = 0; i < dvrp.cmd_in_idx; i++)
				{
					off += snprintf(buf + off, sizeof(buf) - off, "%02x, ", dvrp.cmd_in_data[i]);
				}
				buf[off - 2] = 0;
				Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_CMD=%x(%s) / %s (type=%d)", cmd, buf, get_dvrp_name(cmd).c_str(), type);
			}
			[[fallthrough]];

		// ack and silence spammy commands
		case dvrf_getstat:
		case dvrf_chstat:
		case dvrf_devctl:
		case dvrf_mkdir:
		case dvrf_open:
		case dvrf_dopen:
		case dvrf_remove:
		case dvrf_sync:
		case dvrf_umount:
		case dvr_rec_prohibit:
		case dvr_send_timer_event:
		case dvrav_set_d_audio_sel:
		case dvr_get_sircs:
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;

		// dummy implementations
		case dvrf_dread:
			dvrp.file_fd = (dvrp.cmd_in_data[0] << 16) | dvrp.cmd_in_data[1];
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrf_read:
		case dvrf_write:
			dvrp.file_fd = (dvrp.cmd_in_data[0] << 16) | dvrp.cmd_in_data[1];
			dvrp.read_size = (dvrp.cmd_in_data[2] << 16) | dvrp.cmd_in_data[3];
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_select_position:
			dvrp.tv_channel = dvrp.cmd_in_data[0];
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_get_position:
			dvrp_set_resp_count_val16(2, dvrp.tv_channel);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_get_position_info:
			dvrp_set_resp_count(4);
			// TODO: fill in data
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_position_up:
			dvrp.tv_channel++;
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_position_down:
			dvrp.tv_channel--;
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_get_bs_gain:
			dvrp_set_resp_count_val16(2, 100 - (dvrp.tv_channel) / 10); // %
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_set_preset_info:
			{
				const u16 prop = dvrp.cmd_in_data[0];
				const u16 val = dvrp.cmd_in_data[1];
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMD=%x(%s) : prop=0x%02x(%-22s), val=0x%x", cmd, get_dvrp_name(cmd).c_str(), prop, get_preset_name(prop), val);
				const u16 oldval = Host::GetBaseIntSettingValue("DVRP", get_preset_name(prop));
				if (oldval != val)
				{
					Host::SetBaseIntSettingValue("DVRP", get_preset_name(prop), val);
					Host::CommitBaseSettingChanges();
				}
			}
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;
		case dvrav_get_preset_info:
			{
				const u16 prop = dvrp.cmd_in_data[0];
				const u16 val = Host::GetBaseIntSettingValue("DVRP", get_preset_name(prop));
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMD=%x(%s) : prop=0x%02x(%-22s), val=0x%x", cmd, get_dvrp_name(cmd).c_str(), prop, get_preset_name(prop), val);
				dvrp_set_resp_count_val16(2, val);
			}
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			break;

		// DvrdrvExecCmdAckComp step 1/1
		case dvr_rec_start:
		case dvrav_tun_scan_ch:
		case dvr_save_preset_info:
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			dvrp_shedule_cmd_comp(cmd);
			break;
			
		case dvrf_close:
		case dvrf_dclose:
			dvrp.file_fd = (dvrp.cmd_in_data[0] << 16) | dvrp.cmd_in_data[1];
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			dvrp_shedule_cmd_comp(cmd);
			break;

		case dvrf_lseek:
			{
				dvrp.file_fd = (dvrp.cmd_in_data[0] << 16) | dvrp.cmd_in_data[1];
				u32 offset   = (dvrp.cmd_in_data[2] << 16) | dvrp.cmd_in_data[3];
				u32 mode     = (dvrp.cmd_in_data[4] << 16) | dvrp.cmd_in_data[5];
				if (IOManFile* file = R3000A::ioman::getfd<IOManFile>(dvrp.file_fd))
					dvrp.ret_val = file->lseek(offset, mode);
				else
					dvrp.ret_val = -1;
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_DMA=%x(%s)   : fd=0x%x, offset=0x%x mode=0x%x ret=%d", cmd, get_dvrp_name(cmd).c_str(), dvrp.file_fd, offset, mode, dvrp.ret_val);
			}
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK);
			dvrp_shedule_cmd_comp(cmd);
			break;
			
		// DvrdrvExecCmdAckDmaSendComp step 2/2
		case STEP2_CMD + dvrf_dopen:
		case STEP2_CMD + dvrf_mkdir:
		case STEP2_CMD + dvrf_open:
		case STEP2_CMD + dvrf_write:
		case STEP2_CMD + dvrf_remove:
		case STEP2_CMD + dvrf_sync:
		case STEP2_CMD + dvrf_umount:
		case STEP2_CMD + dvr_send_timer_event:
			dvrp_shedule_cmd_comp(cmd);
			//Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMD=%x(%s)", cmd, get_dvrp_name(cmd).c_str());
			[[fallthrough]];

		// DvrdrvExecCmdAckDma2Comp step 2/3
		case STEP2_CMD + dvrf_getstat:
		case STEP2_CMD + dvrf_chstat:
		case STEP2_CMD + dvrf_devctl:
			dvrp.dma_cmd = cmd;
			dvrp_set_resp_count(1);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK | DVRP_DMA_ACK);
			break;

		// DvrdrvExecCmdAckDmaRecvComp step 3/3
		case STEP3_CMD | dvrf_dread:
		case STEP3_CMD | dvrf_read:
		// DvrdrvExecCmdAckDma2Comp step 3/3
		case STEP3_CMD | dvrf_getstat:
		case STEP3_CMD | dvrf_chstat:
		case STEP3_CMD | dvrf_devctl:
			dvrp.dma_cmd = cmd;
			//Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMD=%x(%s)", cmd, get_dvrp_name(cmd).c_str());
			dvrp_set_resp_count_val(3, 0x0000'4000);
			dvrp_submit_resp(cmd, DVRP_CMD_ACK | DVRP_DMA_ACK);
			break;

		// dma read
		case STEP3_READ_DMA + dvrf_chstat:
		case STEP3_READ_DMA + dvrf_devctl:
			dvrp_shedule_cmd_comp(cmd);
			break;

		case STEP3_READ_DMA + dvrf_getstat:
			{
				std::string path = dvrp_get_host_path(reinterpret_cast<char*>(dvrp.dma_out));
				
				char buf[sizeof(fxio_dirent_t)];
				dvrp.ret_val = R3000A::host_stat(path, (fxio_stat_t*)&buf, true);

				for (size_t i = 0; i < sizeof(fxio_dirent_t); i += 4)
				{
					pMem[i/4] = buf[i]
							  | buf[i+1] << 8
							  | buf[i+2] << 16
							  | buf[i+3] << 24;
				}
				pMem[0] = ByteSwap(pMem[0]);
				pMem[1] = ByteSwap(pMem[1]);
				pMem[2] = ByteSwap(pMem[2]);
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_DMA=%x(%s) : path=%s, ret=%d, mode=0x%x attr=0x%x sz=%d", cmd, get_dvrp_name(cmd).c_str(), path.c_str(),
					dvrp.ret_val, pMem[0], pMem[1], pMem[2]);
			}
			dvrp_shedule_cmd_comp(cmd);
			break;

		case STEP3_READ_DMA + dvrf_dread:
			{
				//Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_DMA=%x(%s) : fd=0x%x sz=%d", cmd, get_dvrp_name(cmd).c_str(), dvrp.file_fd, size);
				if (IOManDir* dir = R3000A::ioman::getfd<IOManDir>(dvrp.file_fd))
				{
					char buf[sizeof(fxio_dirent_t)];
					dvrp.ret_val = dir->read(&buf, true, true);

					for (size_t i = 0; i < sizeof(fxio_dirent_t); i += 4)
					{
						pMem[i/4] = buf[i]
								  | buf[i+1] << 8
								  | buf[i+2] << 16
								  | buf[i+3] << 24;
					}
					pMem[0] = ByteSwap(pMem[0]);
					pMem[1] = ByteSwap(pMem[1]);
					pMem[2] = ByteSwap(pMem[2]);

					if (dvrp.root_dir)
					{
						pMem[0] = ByteSwap(0x100); // mode
						pMem[1] = ByteSwap(0x0); // attr
					}
				}
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_DMA=%x(%s) : fd=0x%x, ret=%d, mode=0x%x attr=0x%x sz=%d", cmd, get_dvrp_name(cmd).c_str(),
					dvrp.file_fd, dvrp.ret_val, pMem[0], pMem[1], pMem[2]);
			}
			dvrp_shedule_cmd_comp(cmd);
			break;

		case STEP3_READ_DMA + dvrf_read:
			if (IOManFile* file = R3000A::ioman::getfd<IOManFile>(dvrp.file_fd))
			{
				dvrp.ret_val = file->read(pMem, dvrp.read_size);
			}
			dvrp_shedule_cmd_comp(cmd);
			break;

		// cmd compl
		case STEP1_COMP + dvr_rec_start:
		case STEP1_COMP + dvr_save_preset_info:
		case STEP2_COMP + dvr_send_timer_event:
			Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)", cmd, get_dvrp_name(cmd).c_str());
			dvrp_set_resp_count(3);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;
			
		case STEP1_COMP + dvrf_lseek:
			dvrp_set_resp_count_val(3, dvrp.ret_val);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;
			
		case STEP1_COMP + dvrf_close:
			if (R3000A::ioman::getfd<IOManFile>(dvrp.file_fd))
			{
				R3000A::ioman::freefd(dvrp.file_fd);
				char buf[sizeof(fxio_dirent_t)];
			}
			Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)   : fd=%x", cmd, get_dvrp_name(cmd).c_str(), dvrp.file_fd);
			dvrp_set_resp_count(3);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;

		case STEP1_COMP + dvrf_dclose:
			if (R3000A::ioman::getfd<IOManDir>(dvrp.file_fd))
			{
				R3000A::ioman::freefd(dvrp.file_fd);
				char buf[sizeof(fxio_dirent_t)];
			}
			Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s) : fd=%x", cmd, get_dvrp_name(cmd).c_str(), dvrp.file_fd);
			dvrp_set_resp_count(3);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;

		case STEP1_COMP + dvrav_tun_scan_ch:
			Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)", cmd, get_dvrp_name(cmd).c_str());
			dvrp_set_resp_count(6);
			dvrp.cmd_out_data[1] = 0x1234;
			dvrp.cmd_out_data[2] = 0x5678;
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;

		case STEP3_COMP + dvrf_devctl:
			{
				const u32 ctloff = (dvrp.dma_out[0] << 24) | (dvrp.dma_out[1] << 16) | (dvrp.dma_out[2] << 8) | (dvrp.dma_out[3]);
				const u32 ctlcmd = (dvrp.dma_out[4] << 24) | (dvrp.dma_out[5] << 16) | (dvrp.dma_out[6] << 8) | (dvrp.dma_out[7]);
				const u32 ctllen = (dvrp.dma_out[8] << 24) | (dvrp.dma_out[9] << 16) | (dvrp.dma_out[10] << 8) | (dvrp.dma_out[11]);
				const u32 arglen = (dvrp.dma_out[12] << 24) | (dvrp.dma_out[13] << 16) | (dvrp.dma_out[14] << 8) | (dvrp.dma_out[15]);
				std::string path = dvrp_get_host_path(reinterpret_cast<char*>(dvrp.dma_out + 6));
				const bool exists = FileSystem::FileExists(path.c_str());
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s) : off=%x, cmd=%x, ctllen=%d, arglen=%d, %s %s", cmd, get_dvrp_name(cmd).c_str(),
					ctloff, ctlcmd, ctllen, arglen, &dvrp.dma_out[16], &dvrp.dma_out[16 + arglen]);
				if (ctlcmd == 0x5001) // PDIOC_ZONESZ
					dvrp_set_resp_count_val(3, 0x0008'0000);
				else if (ctlcmd == 0x5002) // PDIOC_ZONEFREE
					dvrp_set_resp_count_val(3, 0x0000'7d00);
				else
					dvrp_set_resp_count(3);
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;

		case STEP2_COMP + dvrf_mkdir:
			{
				u32 mode = (dvrp.dma_out[0] << 24) | (dvrp.dma_out[1] << 16) | (dvrp.dma_out[2] << 8) | (dvrp.dma_out[3]);
				std::string path = dvrp_get_host_path(reinterpret_cast<char*>(dvrp.dma_out + 4));
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)   : path=%s, mode=%x", cmd, get_dvrp_name(cmd).c_str(), path.c_str(), mode);
				FileSystem::EnsureDirectoryExists(path.c_str(), true);
				dvrp_set_resp_count(3);
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;

		case STEP2_COMP + dvrf_open:
			{
				const u32 flags = (dvrp.dma_out[0] << 24) | (dvrp.dma_out[1] << 16) | (dvrp.dma_out[2] << 8) | (dvrp.dma_out[3]);
				const u32 mode  = (dvrp.dma_out[4] << 8) | (dvrp.dma_out[5]);
				std::string path = dvrp_get_host_path(reinterpret_cast<char*>(dvrp.dma_out + 6));
				IOManFile* file = NULL;
				u32 ret = R3000A::hostfile_open(&file, path, flags, mode);
				if (ret || !file)
				{
					if (file)
						file->close();
					dvrp_set_resp_count_val(3, 0xffff'ffff);
				}
				else
				{
					ret = R3000A::ioman::allocfd(file);
					if (ret < 0)
						file->close();
					dvrp_set_resp_count_val(3, ret);
				}
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)    : path=%s, flags=%x, mode=%x, fd=0x%x", cmd, get_dvrp_name(cmd).c_str(), path.c_str(), flags, mode, ret);
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;

		case STEP2_COMP + dvrf_remove:
			{
				std::string path = dvrp_get_host_path(reinterpret_cast<char*>(dvrp.dma_out));
				bool exists = FileSystem::FileExists(path.c_str());
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)  : path=%s, exists=%d", cmd, get_dvrp_name(cmd).c_str(), path.c_str(), exists);
				dvrp_set_resp_count(3);
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;
			
		case STEP2_COMP + dvrf_sync:
			{
				u32 flags = (dvrp.dma_out[0] << 24) | (dvrp.dma_out[1] << 16) | (dvrp.dma_out[2] << 8) | (dvrp.dma_out[3]);
				std::string path = reinterpret_cast<char*>(dvrp.dma_out + 4);
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)    : dev=%s, flags=%d", cmd, get_dvrp_name(cmd).c_str(), path.c_str(), flags);
				dvrp_set_resp_count(3);
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;

		case STEP2_COMP + dvrf_umount:
			{
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)  : path=%s", cmd, get_dvrp_name(cmd).c_str(), dvrp.dma_out);
				dvrp_set_resp_count(3);
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;

		case STEP2_COMP + dvrf_dopen:
			{
				std::string path = dvrp_get_host_path(reinterpret_cast<char*>(dvrp.dma_out));
				dvrp.root_dir = path.ends_with("dvr_hdd0_");
				IOManDir* dir = NULL;
				u32 ret = R3000A::hostdir_open(&dir, path);
				if (ret || !dir)
				{
					if (dir)
						dir->close();
					dvrp_set_resp_count_val(3, 0xffff'ffff);
				}
				else
				{
					ret = R3000A::ioman::allocfd(dir);
					if (ret < 0)
						dir->close();
					dvrp_set_resp_count_val(3, ret);
				}
				Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s) : path=%s, root=%d, fd=0x%x", cmd, get_dvrp_name(cmd).c_str(), path.c_str(), dvrp.root_dir, ret);
				dvrp.dopen_idx = 0;
				dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			}
			break;
			
		case STEP3_COMP + dvrf_chstat:
			dvrp.ret_val = 0;
		case STEP3_COMP + dvrf_getstat:
			dvrp_set_resp_count_val(3, dvrp.ret_val);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;

		case STEP3_COMP + dvrf_dread:
			dvrp_set_resp_count_val(3, dvrp.ret_val);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;
			
		case STEP3_COMP + dvrf_read:
			Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)    : fd=%x, size=%d, ret=%d", cmd, get_dvrp_name(cmd).c_str(), dvrp.file_fd, dvrp.read_size, dvrp.ret_val);
			dvrp_set_resp_count_val(3, dvrp.ret_val);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;

		case STEP2_COMP + dvrf_write:
			if (IOManFile* file = R3000A::ioman::getfd<IOManFile>(dvrp.file_fd))
			{
				dvrp.ret_val = file->write(dvrp.dma_out, dvrp.read_size);
			}
			Console.WriteLn(Color_StrongCyan, "DEV9: DVRP_CMP=%x(%s)   : fd=%x, size=%d", cmd, get_dvrp_name(cmd).c_str(), dvrp.file_fd, dvrp.read_size);
			dvrp_set_resp_count_val(3, dvrp.ret_val);
			dvrp_submit_resp(cmd_base, DVRP_CMD_COMPL);
			break;
	}
}

void dvrp_writeDMA8Mem(u32* pMem, const int size)
{
	//Console.WriteLn(Color_Magenta, "DEV9: dvrp_writeDMA: pMem=0x%x, size=%d", pMem, size);
	if (size > sizeof(dvrp.dma_out))
		Console.Error("Write DMA OUT err : size=%d, buf=%d", size, sizeof(dvrp.dma_out));
	dvrp.dma_out_size = std::min(static_cast<size_t>(size), sizeof(dvrp.dma_out));
	std::memcpy(dvrp.dma_out, pMem, dvrp.dma_out_size);
	dvrp_handle_func(dvrp.dma_cmd, 1, pMem, size);
	dvrp_submit_resp(dvrp.dma_cmd, DVRP_DMA_COMPL);
}

void dvrp_readDMA8Mem(u32* pMem, const int size)
{
	//Console.WriteLn(Color_Magenta, "DEV9: dvrp_readDMA: pMem=0x%x, size=%d", pMem, size);
	std::memset(pMem, 0x00, size);
	dvrp_handle_func(dvrp.dma_cmd, 1, pMem, size);
	dvrp_submit_resp(dvrp.dma_cmd, DVRP_DMA_COMPL);
}

u16 dvrp_read(u32 addr, int width)
{
	//Console.WriteLn(Color_StrongGreen, "DEV9: dvrp_read addr=%x width=%d", addr, width);

	// 8-bit read
	switch (addr)
	{
		case DVRP_DMA_DIR:
		{
			const u8 value = dev9Ru8(addr);
			//Console.WriteLn(Color_StrongGreen, "DEV9: DVRP : read DVRP_DMA_DIR=%x", value);
			return value;
		}
	}
	
	// 16-bit read
	const int value = dev9Ru16(addr);
	switch (addr)
	{
		case DVRP_DMA_SYNC:
			return 0x00;
		case DVRP_ERR:
			//return dvrp.busy_cmd ? 0x80 : 0x00;
			return 0x00;
		case DVRP_STAT:
			// do not spam with sircs cmds while file cmd are in progress
			return dvrp.busy_cmd ? 0x3c : 0x3e;
			return 0x3e;
		case DVRP_34:
		case DVRP_38:
		case DVRP_3c:
			return 0x01;
		case DVRP_RET_DATA:
			return dvrp.cmd_out_data[dvrp.cmd_out_idx++];
		default:
			Console.WriteLn(Color_StrongGreen, "DEV9: DVRP : Unknown %d bit read addr=%x, val=%x", width, addr, value);
			[[fallthrough]];
		case DVRP_INTR_STAT:
		case DVRP_INTR_MASK:
		case DVRP_INTR_CAUSE:
		case DVRP_RET_COUNT:
			dvrp.cmd_out_idx = 0;
			return value;
	}
}

void dvrp_write(u32 addr, u16 value, int width)
{
	// 8-bit write
	switch (addr)
	{
		case DVRP_DMA_DIR:
			dev9Ru8(addr) = value;
			//Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_DMA_DIR=%x", value);
			return;
	}
	
	// 16-bit write
	dev9Ru16(addr) = value;
	switch (addr)
	{
		case DVRP_RESET:
			Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_RESET=%x", value);
			break;
		case DVRP_DMA_DATA:
			Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_DMA_DATA=%x", value);
			break;
		case DVRP_DMA_DATA2:
			Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_DMA_DATA2=%x", value);
			break;
		case DVRP_INTR_ACK:
			//Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_INTR_ACK=%x", value);
			dev9Ru16(DVRP_INTR_STAT) &= ~value;
			dev9.irqcause &= ~DVRP_INTR_INTRQ;
			break;
		case DVRP_ARG:
			//Console.WriteLn(Color_StrongGreen, "DEV9: DVRP_ARG=%x", value);
			if (dvrp.cmd_in_idx < sizeof(dvrp.cmd_in_data))
				dvrp.cmd_in_data[dvrp.cmd_in_idx++] = value;
			else
				Console.Error("DEV9: DVRP_ARG : idx > sizeof(data)");
			break;
		case DVRP_CMD:
			if (dvrp.busy_cmd)
				Console.Error("DEV9: dvrp cmd 0x%04x received while busy with 0x%04x", value, dvrp.busy_cmd);

			dvrp_handle_func(value, 0, nullptr, 0);
			dvrp.cmd_in_idx = 0;
			/*
			// This line intentionally breaks the DMA transfers to avoid getting
			// completely stuck waiting for an unimplemented response.
			// Comment out the line to continue transfer implementation.
			dev9Ru16(DVRP_INTR_STAT) &= ~DVRP_DMA_ACK;
			//*/
			break;
		default:
			Console.WriteLn(Color_StrongGreen, "DEV9: DVRP : Unknown %d bit write addr=%x, val=%x", width, addr, value);
			[[fallthrough]];
		case DVRP_DMA_SYNC:
		case DVRP_DMA_SIZE:
		case DVRP_DMA_WIDTH:
		case DVRP_INTR_MASK:
		case DVRP_ERR:
			break;
	}
}
