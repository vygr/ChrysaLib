#ifndef USB_LINK_H
#define USB_LINK_H

#define NOMINMAX true

#include "link.h"
#include "libusb.h"

struct USBDeviceInstanceInfo
{
	bool operator==(const USBDeviceInstanceInfo &p) const
	{
		return std::tie(p.m_vid, p.m_pid) == std::tie(m_vid, m_pid);
	}
	uint32_t m_vid;
	uint32_t m_pid;
	uint8_t m_bulk_out_addr;
	uint8_t m_bulk_in_addr;
};

struct USBDeviceInstance
{
	bool operator==(const USBDeviceInstance &p) const
	{
		return std::tie(p.m_info, p.m_path) == std::tie(m_info, m_path);
	}
	USBDeviceInstanceInfo m_info;
	std::string m_path;
	bool m_flag;
};

class USB_Link : public Link
{
public:
	USB_Link(const USBDeviceInstance &device)
		: Link()
		, m_device_instance(device)
	{
		m_device_instance.m_flag = true;
		open();
	}
	USBDeviceInstance &get_usb_dev_inst(void) { return m_device_instance; }
	bool open();
	void close();
protected:
	virtual bool send(const std::shared_ptr<Msg> &msg) override;
	virtual std::shared_ptr<Msg> receive() override;
private:
	USBDeviceInstance m_device_instance;
	libusb_device_handle *m_libusb_device_handle;
	Link_Buf m_send_buf;
	Link_Buf m_receive_buf;
};

class USB_Link_Manager : public Link_Manager
{
public:
	USB_Link_Manager()
		: Link_Manager()
	{}
	void start_thread() override
	{
		m_running = true;
		m_thread = std::thread(&USB_Link_Manager::run, this);
	}
	void join_thread() override { if (m_thread.joinable()) m_thread.join(); }
private:
	void run() override;
	std::thread m_thread;
	std::vector<std::unique_ptr<USB_Link>> m_links;
};

#endif
