#include "file_service.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>

//////////////
// files utils
//////////////

void file_copy(const std::string &src, const std::string &dst);

///////////////
// file service
///////////////

void File_Service::run()
{
	//get my mailbox address, id was allocated in the constructor
	auto mbox = global_router->validate(m_net_id);
	auto entry = global_router->declare(m_net_id, "file_service", "File Service v0.1");

	//event loop
	while (m_running)
	{
		auto msg = mbox->read();
		auto body = (Event*)msg->begin();
		auto body_end = msg->end();
		switch (body->m_evt)
		{
		case evt_get_file_list:
		{
			//get the file list from the subclass
			//the subclass method should just push strings into the given vector
			//for each file in the exported folder
			auto file_list = std::vector<std::string>{};
			in_file_list(file_list);
			auto event = (Event_get_file_list*)body;
			set_file_list(event->m_reply, file_list);
			break;
		}
		case evt_set_file_list:
		{
			//set file list, done this way so it can be a push event as well as requested
			//the subclass on the receiver will probably put this info into a UI widget etc
			auto event = (Event_set_file_list*)body;
			auto file_list = split_string(std::string(event->m_data, body_end), "\n");
			//now call out to whoever wants this
			out_file_list(event->m_src, file_list);
			break;
		}
		case evt_send_file:
		{
			//big job so hive off into a thread from the thread pool,
			//while this thread goes back to reading incoming agent requests.
			//it's not only the size of the job but the fact that this responder
			//can block due to flow control !
			m_thread_pool1->enqueue([=, msg_ref = std::move(msg)]
			{
				//we will time how long things take
				auto start = std::chrono::high_resolution_clock::now();

				auto event = (Event_send_file*)body;
				//prepend the path prefix ?
				auto filename = std::string(event->m_name, body_end);
				if (filename.find('/') == std::string::npos) filename = in_file_path() + filename;
				auto fs = std::ifstream(filename, std::ifstream::ate | std::ifstream::binary);
				if (fs.is_open())
				{
					//get length of file:
					auto total = (uint64_t)fs.tellg();
					fs.seekg (0, fs.beg);
					if (total == 0)
					{
						//dont waste energy on 0 length files !
						fs.close();
						goto nofile;
					}
					//temp Net_ID mailbox for acks
					auto ack_id = global_router->alloc();
					auto ack_mbox = global_router->validate(ack_id);
					//read file and send as chunks over to the destination
					//with an ack window based flow control
					auto offset = uint64_t(0);
					auto num_packets = 0u;
					auto length = (uint64_t)(MAX_PACKET_SIZE - sizeof(send_file_chunk));
					while (offset < total)
					{
						//header
						auto chunk_length = std::min(length, total - offset);
						auto chunk_msg = std::make_shared<Msg>(sizeof(send_file_chunk) + chunk_length);
						chunk_msg->set_dest(event->m_reply);
						//body
						auto reply_body = (send_file_chunk*)chunk_msg->begin();
						reply_body->m_ack = ack_id;
						reply_body->m_total = total;
						reply_body->m_length = chunk_length;
						reply_body->m_offset = offset;
						fs.read(reply_body->m_data, chunk_length);
						global_router->send(chunk_msg);
						offset += chunk_length;
						//do we need to consume an ack before moving on ?
						if (++num_packets >= FILE_CHUNK_WINDOW_SIZE)
						{
							//consume an ack msg, block till we get one or timeout !
							num_packets = 0;
							if (!ack_mbox->read(std::chrono::milliseconds(FILE_TRANSFER_TIMEOUT)))
							{
								auto log = std::ostringstream();
								log << "Send File Error: " << filename;
								out_log(log.str());
								fs.close();
								global_router->free(ack_id);
								return;
							}
						}
					}
					//close file and free the temp ack mailbox
					fs.close();
					global_router->free(ack_id);
				}
				else
				{
				nofile:
					//no such file or empty file, so reply with 0 total msg
					auto chunk_msg = std::make_shared<Msg>(sizeof(send_file_chunk));
					chunk_msg->set_dest(event->m_reply);
					//body
					auto reply_body = (send_file_chunk*)chunk_msg->begin();
					reply_body->m_total = 0;
					global_router->send(chunk_msg);
				}

				//log how long that took
				auto finish = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> elapsed = finish - start;
				auto log = std::ostringstream();
				log << "Sent File: " << filename << std::endl << "Time: " << elapsed.count()/1000.0 << " seconds";
				out_log(log.str());
			});
			break;
		}
		case evt_transfer_file:
		{
			//big job so hive off into a thread from the thread pool.
			m_thread_pool1->enqueue([=, msg_ref = std::move(msg)]
			{
				auto event = (Event_transfer_file*)body;
				//temp mailbox to await reply chunks
				auto rep_id = global_router->alloc();
				auto mbox = global_router->validate(rep_id);
				//send off the file request
				auto files = split_string(std::string(event->m_data, body_end), "\n");
				auto msg = std::make_shared<Msg>(sizeof(Event_send_file));
				msg->set_dest(event->m_src);
				auto event_body = (Event_send_file*)msg->begin();
				event_body->m_evt = evt_send_file;
				event_body->m_reply = rep_id;
				msg->append(files[1]);
				global_router->send(msg);

				//wait for all the reply chunks
				auto tmpname = std::string{};
				auto fs = std::ofstream();
				auto total = uint64_t(0);
				auto amount = uint64_t(0);
				auto offset = uint64_t(0);
				auto num_packets = FILE_CHUNK_WINDOW_SIZE;
				auto progress = 0;
				do
				{
					//read chunk
					auto chunk_msg = mbox->read(std::chrono::milliseconds(FILE_TRANSFER_TIMEOUT));
					if (!chunk_msg)
					{
						auto log = std::ostringstream();
						log << "Transfer File Error: " << files[0] << " <- " << files[1];
						out_log(log.str());
						fs.close();
						global_router->free(rep_id);
						return;
					}
					auto chunk_body = (send_file_chunk*)chunk_msg->begin();
					if (chunk_body->m_total == 0) goto nofile1;
					//first time we know the total size we open a temp file for writing the chunks
					if (!total)
					{
						//create some temp filename for the reply chunks
						total = chunk_body->m_total;
						tmpname = in_temp_file();
						fs.open(tmpname, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
					}
					//write this chunks data into the file
					if (offset != chunk_body->m_offset) fs.seekp(chunk_body->m_offset);
					fs.write(chunk_body->m_data, chunk_body->m_length);
					offset = chunk_body->m_offset + chunk_body->m_length;
					amount += chunk_body->m_length;
					//do we need to send an ack ?
					if (++num_packets >= FILE_CHUNK_WINDOW_SIZE)
					{
						num_packets = 0;
						static auto tag = std::make_shared<std::string>("ack");
						auto ack = std::make_shared<Msg>(tag);
						ack->set_dest(chunk_body->m_ack);
						global_router->send(ack);
					}
					//send a progress report to origin every 10%
					auto new_progress = (int32_t)(amount * 100 / total);
					if (new_progress - progress >= 10)
					{
						msg = std::make_shared<Msg>(sizeof(transfer_file_progress));
						msg->set_dest(event->m_origin);
						auto ack_struct = (transfer_file_progress*)msg->begin();
						ack_struct->m_progress = progress = new_progress;
						global_router->send(msg);
					}
				} while (amount < total);
				fs.close();
				//copy temp over the destination and send OK msg
				if (files[0].find('/') == std::string::npos) files[0] = in_file_path() + files[0];
				std::remove(files[0].data());
				file_copy(tmpname, files[0]);
				std::remove(tmpname.data());
				//and let the origin know it's done
				msg = std::make_shared<Msg>(sizeof(transfer_file_progress));
				msg->set_dest(event->m_origin);
				{
					auto ack_struct = (transfer_file_progress*)msg->begin();
					ack_struct->m_progress = -1;
					global_router->send(msg);
				}
			nofile1:
				//free temp mailbox
				global_router->free(rep_id);
			});
			break;
		}
		default:
			break;
		}
	}

	//forget myself
	global_router->forget(entry);
}

//pushable events

File_Service *File_Service::set_file_list(const Net_ID &net_id, const std::vector<std::string> &file_list)
{
	auto msg = std::make_shared<Msg>(sizeof(Event_set_file_list));
	msg->set_dest(net_id);
	auto event_body = (Event_set_file_list*)msg->begin();
	event_body->m_evt = evt_set_file_list;
	event_body->m_src = m_net_id;
	for (auto &file : file_list) { msg->append(file)->append("\n"); }
	global_router->send(msg);
	return this;
}

//request helpers

File_Service *File_Service::get_file_list(const Net_ID &net_id)
{
	auto msg = std::make_shared<Msg>(sizeof(Event_get_file_list));
	msg->set_dest(net_id);
	auto event_body = (Event_get_file_list*)msg->begin();
	event_body->m_evt = evt_get_file_list;
	event_body->m_reply = m_net_id;
	global_router->send(msg);
	return this;
}

File_Service *File_Service::transfer_file(const Net_ID &dst_id, const Net_ID &src_id, const std::string &dst_name, const std::string &src_name, int32_t ctx)
{
	//big job so hive off into a thread from the thread pool.
	//this method can be called on ANY file_service object and it will arrange the transfer of a file
	//from any source and destination file_service while receiving progress reports.
	//you would most likely ask the file_service who is going to be showing the progress UI !
	m_thread_pool2->enqueue([=]
	{
		//temp mailbox to await progress reports
		auto origin_id = global_router->alloc();
		auto mbox = global_router->validate(origin_id);
		//send off the transfer request
		auto msg = std::make_shared<Msg>(sizeof(Event_transfer_file));
		msg->set_dest(dst_id);
		auto event_body = (Event_transfer_file*)msg->begin();
		event_body->m_evt = evt_transfer_file;
		event_body->m_src = src_id;
		event_body->m_origin = origin_id;
		msg->append(dst_name)->append("\n")->append(src_name);
		global_router->send(msg);
		//wait for progress and confirmation
		for (;;)
		{
			//read progress report
			auto prog_msg = mbox->read(std::chrono::milliseconds(FILE_TRANSFER_TIMEOUT));
			if (!prog_msg)
			{
				//if we don't see any reports for a long time
				//then assume that it's not going to happen
				auto log = std::ostringstream();
				log << "File Transfer Error: " << dst_name << " <- " << src_name;
				out_log(log.str());
				out_error(dst_id, src_id, dst_name, src_name, ctx);
				break;
			}
			auto prog_body = (transfer_file_progress*)prog_msg->begin();
			if (prog_body->m_progress == -1)
			{
				//all has finished
				auto log = std::ostringstream();
				log << "File Transfer OK: " << dst_name << " <- " << src_name;
				out_log(log.str());
				out_ok(dst_id, src_id, dst_name, src_name, ctx);
				break;
			}
			//call the subclass to update a progress bar etc
			out_progress(dst_id, src_id, dst_name, src_name, ctx, prog_body->m_progress);
		}
		global_router->free(origin_id);
	});
	return this;
}
