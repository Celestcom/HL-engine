#include "stdafx.h"
#include "synchronizer2.h"

#include "PacketDispatcher.h"
const uint8_t PACKET_DELIMETER = '$';
const uint8_t PACKET_FOOTER[2] = { 0x0D, 0x0A };

//this..should be rewritten to what it was before :(
//we can still use deadline timer to drive it, but it was simpler as a state machine

bool packetIsWellFormed(const Packet& actualPacket)
{
	


	return  actualPacket[0] == PACKET_DELIMETER &&
		actualPacket[14] == PACKET_FOOTER[0] &&
		actualPacket[15] == PACKET_FOOTER[1];
}


synchronizer2::State synchronizer2::state() const
{
	return m_state;
}

synchronizer2::synchronizer2(boost::asio::io_service& io, std::shared_ptr<boost::lockfree::spsc_queue<uint8_t>> data)
	: m_dispatcher()
	, m_data(data)
	, m_badSyncLimit(2)
	, m_badSyncCounter(0)
	, m_syncInterval(boost::posix_time::millisec(10))
	, m_syncTimer(io)
	, m_totalBytesRead(0)
	, m_state(State::SearchingForSync)
{

}

void synchronizer2::start()
{
	transition_state();
}

void synchronizer2::stop()
{
	std::cout << "STOPPING SYNC\n";
	m_syncTimer.cancel();
}

void synchronizer2::on_packet(PacketEvent::slot_type slot)
{
	m_dispatcher.connect(slot);
}

std::size_t synchronizer2::total_bytes_read() const
{
	return m_totalBytesRead;
}

boost::asio::io_service & synchronizer2::get_io_service()
{
	return m_syncTimer.get_io_service();
}

void synchronizer2::transition_state()
{
	switch (m_state)
	{
	case State::SearchingForSync:
		search_for_sync();
		break;
	case State::ConfirmingSync:
		confirm_sync();
		break;
	case State::Synchronized:
		synchronized_read();
		break;
	case State::ConfirmingSyncLoss:
		confirm_sync_loss();
		break;
	default:
		break;
	}

	auto self(shared_from_this());
	m_syncTimer.expires_from_now(m_syncInterval);
	m_syncTimer.async_wait([this, self](const auto& ec) { if (ec) { return; } transition_state(); });
}

void synchronizer2::confirm_sync()
{

	auto possiblePacket = dequeuePacket();
	if (!possiblePacket) {
		m_state = State::ConfirmingSync;
		return;
	}

	if (packetIsWellFormed(*possiblePacket)) {
		m_dispatcher(*possiblePacket);
		m_state = State::Synchronized;
	}
	else {
		m_state = State::SearchingForSync;
	}
}

void synchronizer2::confirm_sync_loss()
{

	auto possiblePacket = this->dequeuePacket();
	if (!possiblePacket) {
		return;
	}

	if (!packetIsWellFormed(*possiblePacket)) {
		m_badSyncCounter++;
		if (m_badSyncCounter >= m_badSyncLimit) {
			m_state = State::SearchingForSync;
		}
	}
	else {
		m_dispatcher(*possiblePacket);
		m_state = State::Synchronized;
	}
}

void synchronizer2::search_for_sync()
{

	
	auto possiblePacket = dequeuePacket();
	if (!possiblePacket) {
		return;
	}

	if (packetIsWellFormed(*possiblePacket) || seek_offset(*possiblePacket)) {
		m_dispatcher(*possiblePacket);
		m_state = State::ConfirmingSync;
	}
}

bool synchronizer2::seek_offset(const Packet& realPacket)
{

	for (std::size_t offset = 1; offset < PACKET_LENGTH; ++offset) {
		if (realPacket[offset] == PACKET_DELIMETER) {
			std::size_t howMuchLeft = offset;
			for (std::size_t i = 0; i < howMuchLeft; ++i)
			{
				m_data->pop();
			}
			return true;
		}
	}

	return false;
}

void synchronizer2::synchronized_read()
{

	auto possiblePacket = this->dequeuePacket();

	if (!possiblePacket) {
		return;
	}

	if (packetIsWellFormed(*possiblePacket)) {
		m_dispatcher(*possiblePacket);
	}
	else {
		m_badSyncCounter = 1;
		m_state = State::ConfirmingSyncLoss;
	}
}



boost::optional<Packet> synchronizer2::dequeuePacket()
{
	try
	{
		auto avail = m_data->read_available();
		if (avail < PACKET_LENGTH) {
			return boost::none;
		}


		Packet p;
		int numPopped = m_data->pop(p.data(), PACKET_LENGTH);
		assert(numPopped == PACKET_LENGTH);
		m_totalBytesRead += numPopped;


		return p;

	}
	catch (const std::exception&) {
		//core_log(nsvr_severity_error, "Synchronizer", std::string("Tried to read from the data stream, but there wasn't enough data!" + std::string(e.what())));
	}

	return boost::none;

}