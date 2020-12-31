
#include "PrecompiledHeader.h"
#include "../qemu-usb/vl.h"
#include "../qemu-usb/desc.h"
#include "../shared/inifile_usb.h"
#include "usb-passthrough.h"
#include <libusb-1.0/libusb.h>

namespace usb_passthrough
{
	std::vector<std::string> PassthroughDevice::libusbGetDevList()
	{
		std::vector<std::string> devList;

		if (libusb_init(NULL) < 0)
			return devList;

		libusb_device** devs;
		const ssize_t cnt = libusb_get_device_list(NULL, &devs);
		if (cnt < 0)
		{
			return devList;
		}

		int i = 0;
		libusb_device* dev;
		while ((dev = devs[i++]) != NULL)
		{
			char dev_vid[32] = {0};
			char dev_pid[32] = {0};
			char manufacturer[256] = {0};
			char product[256] = {0};

			struct libusb_device_descriptor desc;
			int ret = libusb_get_device_descriptor(dev, &desc);
			if (ret < 0)
			{
				fprintf(stderr, "USB: unable to get device descriptor\n");
				continue;
			}

			libusb_device_handle* handle;
			ret = libusb_open(dev, &handle);
			if (ret < 0)
			{
				fprintf(stderr, "USB: unable to open device: %04x:%04x\n", desc.idVendor, desc.idProduct);
				continue;
			}

			snprintf(dev_vid, sizeof(dev_vid) - 1, "%04x", desc.idVendor);
			snprintf(dev_pid, sizeof(dev_pid) - 1, "%04x", desc.idProduct);
			if (desc.iManufacturer)
				ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char*)manufacturer, sizeof(manufacturer) - 1);
			if (desc.iProduct)
				ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*)product, sizeof(product) - 1);

			libusb_close(handle);

			devList.push_back("[" + std::string(dev_vid) + ":" + std::string(dev_pid) + "] "
					+ std::string(manufacturer) + " - " + std::string(product));
		}

		libusb_free_device_list(devs, 1);
		return devList;
	}

	static libusb_device_handle* usb_handle;

	int PassthroughDevice::loadAndOpen(std::string devName)
	{
		fprintf(stderr, "USB pass: selectedDevice=%s\n", devName.c_str());
		int vid = 0;
		int pid = 0;
		int ret = sscanf(devName.c_str(), "[%04x:%04x]", &vid, &pid);
		if (ret != 2)
		{
			return -1;
		}
		fprintf(stderr, "USB pass: selectedDevice : vid=%04x, pid=%04x\n", vid, pid);

		if (libusb_init(NULL) < 0)
		{
			fprintf(stderr, "USB err: libusb_init\n");
			return -2;
		}

		usb_handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
		if (usb_handle == nullptr)
		{
			fprintf(stderr, "USB err: libusb_open_device_with_vid_pid\n");
			return -3;
		}
		ret = libusb_claim_interface(usb_handle, 0); // TODO:
		if (ret)
		{
			fprintf(stderr, "USB err: libusb_claim_interface\n");
			return -4;
		}
		return 0;
	}


	typedef struct PassthroughState
	{
		USBDevice dev;
		USBDesc desc;
		USBDescDevice desc_dev;
	} PassthroughState;

	static const USBDescStrings desc_strings = {
		"",
		"",
		"\"PSP\" Type D",
	};

	static void usb_psp_handle_reset(USBDevice* dev)
	{
		fprintf(stderr, "Reset\n");
	}

	static void usb_psp_handle_control(USBDevice* dev, USBPacket* p, int request, int value,
									   int index, int length, uint8_t* data)
	{
		int ret = 0;

		fprintf(stderr, "F: usb_ctrl; req=%4x (%s), val=%4x, idx=%4x, len=%4x : [",
				request,
				(request == 5) ? "setAddr" :
			    (request == 0x8006) ? "getDesc" :
				(request == 9) ? "setConf" :
				(request == 0x2109) ? "setRepr" :
				"",
				value, index, length);

		ret = usb_desc_handle_control(dev, p, request, value, index, length, data);
		if (ret >= 0)
		{
			for (int i = 0; i < p->actual_length; i++)
			{
				fprintf(stderr, "%02x ", data[i]);
			}
			fprintf(stderr, "]\n");
			//return;
		}

		switch (request)
		{	
			case DeviceOutRequest | USB_REQ_SET_ADDRESS:
			case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
				return;
			case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
				libusb_set_configuration(usb_handle, value);
				return;
			case VendorInterfaceOutRequest | 0x07:
				for (int i = 0; i < length; i++)
				{
					fprintf(stderr, "%02x ", data[i]);
				}
				fprintf(stderr, "]\n");
				return;
				break;
			default:
				p->status = USB_RET_STALL;
				break;
		}
		for (int i = 0; i < p->actual_length; i++)
		{
			fprintf(stderr, "%02x ", data[i]);
		}
		fprintf(stderr, "]\n");
	}

	static void usb_psp_handle_data(USBDevice* dev, USBPacket* p)
	{
		unsigned char data[64];
		int len = 0;
		int ret = 0;
		uint8_t dev_ep = p->ep->nr;

		switch (p->pid)
		{
			case USB_TOKEN_IN:
				//fprintf(stderr, "F: dataIn: pid=%x id=%llx ep=%x : \n", p->pid, p->id, dev_ep);
				ret = libusb_interrupt_transfer(usb_handle, USB_DIR_IN | dev_ep, data, sizeof(data), &len, 20);
				if (ret == LIBUSB_SUCCESS)
				{
					usb_packet_copy(p, data, len);
				}
				else
				{
					fprintf(stderr, "libusb_interrupt_transfer = %d=%s\n", ret, libusb_strerror(ret));
					p->status = USB_RET_NAK;
				}
				//for (int i = 0; i < len; i++)
				//{
				//	fprintf(stderr, "%02x ", data[i]);
				//}
				break;

			case USB_TOKEN_OUT:
				fprintf(stderr, "F: dataOut: pid=%x id=%llx ep=%x : \n", p->pid, p->id, dev_ep);
				break;

			default:
				fprintf(stderr, "Bad token\n");
				p->status = USB_RET_STALL;
				break;
		}
	}

	static void usb_psp_handle_destroy(USBDevice* dev)
	{
		PassthroughState* s = (PassthroughState*)dev;
		if (usb_handle != nullptr)
		{
			libusb_close(usb_handle);
			usb_handle = nullptr;
		}

		delete s;
	}

	USBDevice* PassthroughDevice::CreateDevice(int port)
	{
		PassthroughState* s = new PassthroughState();
		std::string api = *PassthroughDevice::ListAPIs().begin();

		std::string selectedDevice;
#ifdef _WIN32
		std::wstring tmp;
		LoadSetting(PassthroughDevice::TypeName(), port, APINAME, N_DEVICE, tmp);
		selectedDevice = wstr_to_str(tmp);
#else
		LoadSetting(PassthroughDevice::TypeName(), port, APINAME, N_DEVICE, selectedDevice);
#endif
		int ret = loadAndOpen(selectedDevice);
		if (ret)
			goto fail;

		libusb_device* usb_dev = libusb_get_device(usb_handle);
		struct libusb_device_descriptor usb_dev_desc;
		ret = libusb_get_device_descriptor(usb_dev, &usb_dev_desc);

		struct libusb_config_descriptor* usb_dev_config;
		ret = libusb_get_active_config_descriptor(usb_dev, &usb_dev_config);

		uint8_t usb_dev_config2[512];
		uint16_t usb_dev_config_len = 0;

		memcpy(usb_dev_config2 + usb_dev_config_len, usb_dev_config, 9);
		usb_dev_config_len += 9;

		//for (int c = 0; c < usb_dev_desc.bNumConfigurations; c++)
		//{
		for (int i = 0; i < usb_dev_config->bNumInterfaces; i++)
		{
			for (int a = 0; a < usb_dev_config->interface[i].num_altsetting; a++)
			{
				memcpy(usb_dev_config2 + usb_dev_config_len, &usb_dev_config->interface[i].altsetting[a],
						usb_dev_config->interface[i].altsetting[a].bLength);
				usb_dev_config_len += usb_dev_config->interface[i].altsetting[a].bLength;

				for (int e = 0; e < usb_dev_config->interface[i].altsetting[a].bNumEndpoints; e++)
				{
					memcpy(usb_dev_config2 + usb_dev_config_len, &usb_dev_config->interface[i].altsetting[a].endpoint[e],
							usb_dev_config->interface[i].altsetting[a].endpoint[e].bLength);
					usb_dev_config_len += usb_dev_config->interface[i].altsetting[a].endpoint[e].bLength;
				}
			}
		}

		libusb_free_config_descriptor(usb_dev_config);


		s->dev.speed = USB_SPEED_FULL;

		s->desc.full = &s->desc_dev;
		s->desc.str = desc_strings;
		if (usb_desc_parse_dev((const uint8_t*)&usb_dev_desc, sizeof(usb_dev_desc), s->desc, s->desc_dev) < 0)
		{
			fprintf(stderr, "USB pass: Bad desc\n");
			goto fail;
		}
		if (usb_desc_parse_config((const uint8_t*)usb_dev_config2, usb_dev_config_len, s->desc_dev) < 0)
		{
			fprintf(stderr, "USB pass: Bad config\n");
			goto fail;
		}

		s->dev.klass.handle_attach = usb_desc_attach;
		s->dev.klass.handle_reset = usb_psp_handle_reset;
		s->dev.klass.handle_control = usb_psp_handle_control;
		s->dev.klass.handle_data = usb_psp_handle_data;
		s->dev.klass.unrealize = usb_psp_handle_destroy;
		s->dev.klass.usb_desc = &s->desc;
		s->dev.klass.product_desc = desc_strings[2];

		usb_desc_init(&s->dev);
		usb_ep_init(&s->dev);

		usb_psp_handle_reset((USBDevice*)s);
		return (USBDevice*)s;

	fail:
		usb_psp_handle_destroy((USBDevice*)s);
		return NULL;
	}

	int PassthroughDevice::Freeze(int mode, USBDevice* dev, void* data)
	{
		return 0;
	}

} // namespace usb_passthrough
