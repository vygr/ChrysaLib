#include "usb_link.h"
#include "../mail/router.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

extern std::unique_ptr<Router> m_router;

///////////
//utilities
///////////

uint32_t jenkins_hash(const uint8_t *key, size_t len);
void obfuscate(uint8_t *key, size_t len);

std::string get_usb_dev_inst_path(libusb_device *device)
{
	std::string path;
	uint8_t portNumbers[7];
	int32_t r = libusb_get_port_numbers(device, portNumbers, sizeof(portNumbers));
	if (r > 0)
	{
		path = std::to_string(portNumbers[0]);
		for (int32_t j = 1; j < r; j++)
		{
			path.append(".");
			path.append(std::to_string(portNumbers[j]));
		}
	}
	return path;
}

std::vector<USBDeviceInstance> scan_usb_devices()
{
	//scan and return a list of USB devices that match the Prolific cable
	//hardware descriptor pid/vid for the cables we support
	std::vector<USBDeviceInstance> instances;
	libusb_device **devices;
	libusb_device *device;
	libusb_device_descriptor device_descriptor;
	libusb_get_device_list(nullptr, &devices);
	for (auto i = 0; (device = devices[i++]) != nullptr;)
	{
		if (libusb_get_device_descriptor(device, &device_descriptor) == 0)
		{
			static std::vector<USBDeviceInstanceInfo> infos =
				{{0x67b, 0x25a1, 0x2, 0x3},
				 {0x67b, 0x27a1, 0x8, 0x9}};
			for (auto &info : infos)
			{
				if ((device_descriptor.idVendor == info.m_vid)
					&& (device_descriptor.idProduct == info.m_pid))
				{
					instances.emplace_back(USBDeviceInstance{info, get_usb_dev_inst_path(device), false});
				}
			}
		}
	}
	libusb_free_device_list(devices, 1);
	return instances;
}

/////////////
//USB Manager
/////////////

void USB_Link_Manager::run()
{
	libusb_init(nullptr);

	while (m_running)
	{
		//as link come and go we need to add and remove them from the router
		auto devices = scan_usb_devices();

		//set found flags to false
		for (auto &usb_link : m_links) usb_link->get_usb_dev_inst().m_flag = false;

		//test for new devices
		for (auto &device : devices)
		{
			auto itr = std::find_if(begin(m_links), end(m_links), [&] (auto &link)
			{
				return device == link->get_usb_dev_inst();
			});
			if (itr != end(m_links))
			{
				//found the device, so don't delete
				(*itr)->get_usb_dev_inst().m_flag = true;
			}
			else
			{
				//not found the device, so start up a new link
				m_links.emplace_back(std::make_unique<USB_Link>(device));
				m_links.back()->start_threads();
			}
		}

		//purge any dead/closed links
		m_links.erase(std::remove_if(begin(m_links), end(m_links), [&] (auto &link)
		{
			if (link->get_usb_dev_inst().m_flag == true) return false;
			std::cout << "usb_link: purged" << std::endl;
			link->stop_threads();
			link->join_threads();
			return true;
		}), end(m_links));

		//wait till next polling time
		std::this_thread::sleep_for(std::chrono::milliseconds(USB_LINK_MANAGER_POLLING_RATE));
	}

	//close any links
	for (auto &link : m_links) link->stop_threads();
	for (auto &link : m_links) link->join_threads();
	m_links.clear();

	libusb_exit(nullptr);
}

//////////
//USB link
//////////

bool USB_Link::open()
{
	//open a USB device and claim it
	libusb_device **devices;
	libusb_device *device = nullptr;
	libusb_device_descriptor device_descriptor;
	libusb_get_device_list(nullptr, &devices);
	bool found = false;
	for (auto i = 0; ((device = devices[i++]) != nullptr);)
	{
		if (get_usb_dev_inst_path(device) == m_device_instance.m_path
			&& !libusb_get_device_descriptor(device, &device_descriptor)
			&& (device_descriptor.idVendor == m_device_instance.m_info.m_vid)
			&& (device_descriptor.idProduct == m_device_instance.m_info.m_pid))
		{
			found = true;
			break;
		}
	}
	if (found && (libusb_open(device, &m_libusb_device_handle) == LIBUSB_SUCCESS))
	{
		//lets the link open the handle when its thread is created
		if (libusb_claim_interface(m_libusb_device_handle, 0) != LIBUSB_SUCCESS) close();
	}
	libusb_free_device_list(devices, 1);
	return m_libusb_device_handle != nullptr;
}

void USB_Link::close()
{
	//close a USB device and release it
	if (m_libusb_device_handle != nullptr)
	{
		libusb_release_interface(m_libusb_device_handle, 0);
		libusb_close(m_libusb_device_handle);
		m_libusb_device_handle = nullptr;
	}
}

//send msg header and body down the link
bool USB_Link::send(const std::shared_ptr<Msg> &msg)
{
	//pack msg into send buffer, calculate the hash and obfuscate
	memcpy(&m_send_buf.m_dev_id, &m_router->get_dev_id(), sizeof(Dev_ID));
	memcpy((uint8_t*)&m_send_buf.m_msg_header, &msg->m_header, sizeof(Msg_Header));
	memcpy(m_send_buf.m_msg_body, msg->begin() + msg->m_header.m_data_offset, msg->m_header.m_frag_length);
	m_send_buf.m_hash = jenkins_hash((uint8_t*)&m_send_buf.m_dev_id, offsetof(Link_Buf, m_msg_body) - offsetof(Link_Buf, m_dev_id) + msg->m_header.m_frag_length);
	obfuscate((uint8_t*)&m_send_buf, offsetof(Link_Buf, m_msg_body) + msg->m_header.m_frag_length);

	//send the buffer down the link
	auto error = 0;
	auto sent = 0;
	int32_t len = offsetof(Link_Buf, m_msg_body) + msg->m_header.m_frag_length;
	do
	{
		//send down link, retry till no error or exiting
		error = libusb_bulk_transfer(m_libusb_device_handle, LIBUSB_ENDPOINT_OUT | m_device_instance.m_info.m_bulk_out_addr,
			(uint8_t*)&m_send_buf, len, &sent, USB_BULK_TRANSFER_TIMEOUT);
	} while (m_running && error != LIBUSB_SUCCESS && error != LIBUSB_ERROR_NO_DEVICE);
	return error == LIBUSB_SUCCESS ? true : false;
}

std::shared_ptr<Msg> USB_Link::receive()
{
	//receive the buffer from the link
	auto error = 0;
	auto len = 0;
	do
	{
		error = libusb_bulk_transfer(m_libusb_device_handle, LIBUSB_ENDPOINT_IN | m_device_instance.m_info.m_bulk_in_addr,
			(uint8_t*)&m_receive_buf, sizeof(m_receive_buf), &len, USB_BULK_TRANSFER_TIMEOUT);
	} while (m_running && error != LIBUSB_SUCCESS && error != LIBUSB_ERROR_NO_DEVICE);
	if (error != LIBUSB_SUCCESS) return nullptr;

	//un-obfuscate and calculate the hash
	obfuscate((uint8_t*)&m_receive_buf, len);
	auto hash = jenkins_hash((uint8_t*)&m_receive_buf.m_dev_id, len - offsetof(Link_Buf, m_dev_id));
	if (hash != m_receive_buf.m_hash)
	{
		//error with crc hash !!!
		std::cerr << "usb_link: crc error !" << std::endl;
		return nullptr;
	}

	//unpack msg from receive buffer
	auto msg = std::make_shared<Msg>(m_receive_buf.m_msg_header, m_receive_buf.m_msg_body);

	//refresh who we are connected to in a unplug/plug scenario.
	//if the peer device id changes we need to swap the link on the router !
	//the software equivelent of pulling the lead out and plugging another one in.
	if (m_receive_buf.m_dev_id != m_remote_dev_id)
	{
		m_remote_dev_id = m_receive_buf.m_dev_id;
		m_router->sub_link(this);
		m_router->add_link(this, m_remote_dev_id);
	}
	return msg;
}
