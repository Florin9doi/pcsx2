#ifndef USBPASSTHROUGH_H
#define USBPASSTHROUGH_H

#include "../deviceproxy.h"

namespace usb_passthrough
{
	static const char* APINAME = "libusb";

	class PassthroughDevice
	{
	public:
		virtual ~PassthroughDevice() {}
		static USBDevice* CreateDevice(int port);
		static const TCHAR* Name()
		{
			return TEXT("USB passthrough");
		}
		static const char* TypeName()
		{
			return "usb_passthrough";
		}
		static std::list<std::string> ListAPIs()
		{
			return std::list<std::string>{APINAME};
		}
		static const TCHAR* LongAPIName(const std::string& name)
		{
			return TEXT("libusb-1.0");
		}
		static int Configure(int port, const std::string& api, void* data);
		static int Freeze(int mode, USBDevice* dev, void* data);

		static std::vector<std::string> libusbGetDevList();
		static int loadAndOpen(std::string devName);
	};

} // namespace usb_passthrough
#endif
