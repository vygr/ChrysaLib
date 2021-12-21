#ifndef MSG_H
#define MSG_H

#include "net.h"

//message header, used for all messages, including fragments of parcels.
//parcels are messages that are bigger than the max msg packet size.
//fragments of parcels are like holograms, they say how big the full parcel is
//but they only hold a chunk of that parcel.
struct Msg_Header
{
	Msg_Header() {}
	Msg_Header(unsigned int total_len)
		: m_frag_length(total_len)
		, m_total_length(total_len)
	{}
	Msg_Header(const Msg_Header &header)
		: m_dest(header.m_dest)
		, m_src(header.m_src)
		, m_frag_length(header.m_frag_length)
		, m_frag_offset(header.m_frag_offset)
		, m_total_length(header.m_total_length)
	{}
	Msg_Header(const Net_ID &dst, const Net_ID &src, unsigned int total_len, unsigned int frag_len, unsigned int frag_offset)
		: m_dest(dst)
		, m_src(src)
		, m_frag_length(frag_len)
		, m_frag_offset(frag_offset)
		, m_data_offset(frag_offset)
		, m_total_length(total_len)
	{}
	//the destination id info.
	//fill in before Router::send(msg) with msg->set_dest(Net_ID)
	Net_ID m_dest;
	//the folowing are filled in automatically.
	//the source id info (combo of src parcel id and source dev id...)
	//this is NOT the thing that sent the msg !!! it's system level information !
	Net_ID m_src;
	//info describing this packet
	uint32_t m_frag_length = 0;
	uint32_t m_frag_offset = 0;
	uint32_t m_data_offset = 0;
	uint32_t m_total_length = 0;
};

//message is just a header and body data.
//the body is a shared pointer so that fragments can be created, and broadcasting can be done,
//without having to copy the body data.
//useful append methods are provided to ease populating a msg body.
class Msg
{
public:
	Msg() : m_data(std::make_shared<std::string>())
	{}
	Msg(std::shared_ptr<std::string> data)
		: m_header((unsigned int)data->size())
		, m_data(data)
	{}
	Msg(std::shared_ptr<std::string> data, const Net_ID &dst, const Net_ID &src, unsigned int frag_len, unsigned int frag_offset)
		: m_header(dst, src, (unsigned int)data->size(), frag_len, frag_offset)
		, m_data(data)
	{}
	//the fill constructors used here wastes time, need to work out how to
	//tell C++ to allocate the buffer but not bother to zero it !
	Msg(size_t length)
		: m_header((unsigned int)length)
		, m_data(std::make_shared<std::string>(length, '\0'))
	{}
	Msg(const Msg_Header &header, const uint8_t *buf)
		: m_header(header)
		, m_data(std::make_shared<std::string>((const char*)buf, header.m_frag_length))
	{}
	//can be compared ! Very important when using mbox->filter() method to roll up etc
	bool operator==(const Msg &p) const { return *m_data == *p.m_data; }
	//return the begining and end of the body data
	auto begin() { return m_data->size() ? &(*m_data->begin()) : nullptr; }
	auto end() { return begin() + m_data->size(); }
	//set destination Net_ID, the delivery address
	auto set_dest(const Net_ID &id) { m_header.m_dest = id; return this; }
	//append some string to the body data
	auto append(const std::string data)
	{
		m_data->append(data);
		m_header.m_frag_length = m_header.m_total_length = (unsigned int)m_data->size();
		return this;
	}
	//append some raw bytes to the body data
	auto append(const char *data, unsigned int len)
	{
		m_data->append(data, len);
		m_header.m_frag_length = m_header.m_total_length = (unsigned int)m_data->size();
		return this;
	}
	//msg info
	Msg_Header m_header;
	std::shared_ptr<std::string> m_data;
};

#endif
