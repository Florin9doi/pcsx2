/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include <commdlg.h>
#include "usb-passthrough.h"
#include "../Win32/Config_usb.h"
#include "../Win32/resource_usb.h"

namespace usb_passthrough
{
	static OPENFILENAMEW ofn;
	BOOL CALLBACK USBPassthroughDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		int port;

		switch (uMsg)
		{
			case WM_CREATE:
				SetWindowLongPtr(hW, GWLP_USERDATA, (LONG)lParam);
				break;
			case WM_INITDIALOG:
			{
				port = (int)lParam;
				SetWindowLongPtr(hW, GWLP_USERDATA, (LONG)lParam);

				std::wstring selectedDevice;
				LoadSetting(PassthroughDevice::TypeName(), port, APINAME, N_DEVICE, selectedDevice);
				SendDlgItemMessage(hW, IDC_COMBO1_USB, CB_RESETCONTENT, 0, 0);

				std::vector<std::string> devList = PassthroughDevice::libusbGetDevList();
				for (auto i = 0; i != devList.size(); i++)
				{
					std::wstring ws(devList[i].begin(), devList[i].end());

					SendDlgItemMessageW(hW, IDC_COMBO1_USB, CB_ADDSTRING, 0, (LPARAM)ws.c_str());
					if (selectedDevice == ws)
					{
						SendDlgItemMessage(hW, IDC_COMBO1_USB, CB_SETCURSEL, i, i);
					}
				}
				return TRUE;
			}
			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					switch (LOWORD(wParam))
					{
						case IDOK:
						{
							INT_PTR res = RESULT_OK;
							static wchar_t selectedDevice[500] = {0};
							GetWindowTextW(GetDlgItem(hW, IDC_COMBO1_USB), selectedDevice, countof(selectedDevice));
							port = (int)GetWindowLongPtr(hW, GWLP_USERDATA);
							if (!SaveSetting<std::wstring>(PassthroughDevice::TypeName(), port, APINAME, N_DEVICE, selectedDevice))
							{
								res = RESULT_FAILED;
							}
							EndDialog(hW, res);
							return TRUE;
						}
						case IDCANCEL:
							EndDialog(hW, RESULT_CANCELED);
							return TRUE;
					}
				}
		}
		return FALSE;
	}

	int PassthroughDevice::Configure(int port, const std::string& api, void* data)
	{
		Win32Handles handles = *(Win32Handles*)data;
		return DialogBoxParam(handles.hInst,
							  MAKEINTRESOURCE(IDD_DLG_USB_PASSTHROUGH),
							  handles.hWnd,
							  (DLGPROC)USBPassthroughDlgProc, port);
	}

} // namespace usb_passthrough
