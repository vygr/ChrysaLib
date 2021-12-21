#include "../../lib/services/kernel_service.h"
#include "../../lib/services/file_service.h"
#include "../../lib/links/ip_link.h"
#include <iostream>
#include <sstream>

////////////
// files app
////////////

void ss_reset(std::stringstream &ss, std::string s)
{
	ss.str(s);
	ss.clear();
}

int main(int argc, char *argv[])
{
	//process comand args
	auto arg_t = 0U;
	std::vector<std::string> arg_dial;
	std::stringstream ss;
	for (auto i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			//switch
			std::string opt = argv[i];
			while (!opt.empty() && opt[0] == '-') opt.erase(0, 1);
			if (opt == "t")
			{
				if (++i >= argc) goto help;
				ss_reset(ss, argv[i]);
				if (opt == "t") ss >> arg_t;
			}
			else
			{
			help:
				std::cout << "files [switches] ip_addr ...\n";
				std::cout << "eg. files -t 10000 127.0.0.1\n";
				std::cout << "-h:    this help info\n";
				std::cout << "-t ms: exit timeout, default 0, ie never\n";
				exit(0);
			}
		}
		else
		{
			//arg
			arg_dial.push_back(argv[i]);
		}
	}

	//vars
	std::unique_ptr<Router> m_router;
	std::unique_ptr<Kernel_Service> m_kernel;
	std::unique_ptr<File_Service> m_files;
	std::unique_ptr<IP_Link_Manager> m_ip_link_manager;

	//startup, kernel is first service so it gets Mailbox_ID 0
	m_router = std::make_unique<Router>();
	m_kernel = std::make_unique<Kernel_Service>(*m_router);
	m_files = std::make_unique<File_Service>(*m_router);
	m_kernel->start_thread();
	m_files->start_thread();
	if (!arg_dial.empty())
	{
		std::cout << "Starting IP link manager" << std::endl;
		m_ip_link_manager = std::make_unique<IP_Link_Manager>(*m_router, "");
		m_ip_link_manager->start_thread();
		for (auto &addr : arg_dial)
		{
			std::cout << "Dialing " << addr << std::endl;
			m_ip_link_manager->dial(addr);
		}
	}

	//loop till timeout
	auto start = std::chrono::high_resolution_clock::now();
	for (;;)
	{
		//could do somthing here, like monitoring etc

		//are we done ?
		auto finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = finish - start;
		if (arg_t != 0 && elapsed.count() > arg_t) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//shutdown
	if (m_ip_link_manager) m_ip_link_manager->stop_thread();
	m_files->stop_thread();
	m_kernel->stop_thread();
	if (m_ip_link_manager) m_ip_link_manager->join_thread();
	m_files->join_thread();
	m_kernel->join_thread();

	return 0;
}
