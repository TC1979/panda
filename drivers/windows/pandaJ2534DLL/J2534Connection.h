#pragma once
#include "panda/panda.h"
#include "J2534_v0404.h"
#include "synchronize.h"
#include "J2534MessageFilter.h"
#include "PandaJ2534Device.h"

class J2534MessageFilter;
class PandaJ2534Device;

#define check_bmask(num, mask)(((num) & mask) == mask)

class J2534Connection {
public:
	J2534Connection(
		std::shared_ptr<PandaJ2534Device> panda_dev,
		unsigned long ProtocolID,
		unsigned long Flags,
		unsigned long BaudRate
	);
	~J2534Connection() {};
	virtual long PassThruReadMsgs(PASSTHRU_MSG *pMsg, unsigned long *pNumMsgs, unsigned long Timeout);
	virtual long PassThruWriteMsgs(PASSTHRU_MSG *pMsg, unsigned long *pNumMsgs, unsigned long Timeout);
	virtual long PassThruStartPeriodicMsg(PASSTHRU_MSG *pMsg, unsigned long *pMsgID, unsigned long TimeInterval);
	virtual long PassThruStopPeriodicMsg(unsigned long MsgID);

	virtual long PassThruStartMsgFilter(unsigned long FilterType, PASSTHRU_MSG *pMaskMsg, PASSTHRU_MSG *pPatternMsg,
		PASSTHRU_MSG *pFlowControlMsg, unsigned long *pFilterID);

	virtual long PassThruStopMsgFilter(unsigned long FilterID);
	virtual long PassThruIoctl(unsigned long IoctlID, void *pInput, void *pOutput);

	long init5b(SBYTE_ARRAY* pInput, SBYTE_ARRAY* pOutput);
	long initFast(PASSTHRU_MSG* pInput, PASSTHRU_MSG* pOutput);
	long clearTXBuff();
	long clearRXBuff();
	long clearPeriodicMsgs();
	long clearMsgFilters();

	long setBaud(unsigned long baud);
	unsigned long getBaud();

	unsigned long getProtocol();

	virtual bool isProtoCan() {
		return FALSE;
	}

	unsigned long getPort();

	virtual void processMessageReceipt(const PASSTHRU_MSG_INTERNAL& msg);
	virtual void processMessage(const PASSTHRU_MSG_INTERNAL& msg);

	virtual unsigned long getMinMsgLen();
	virtual unsigned long getMaxMsgLen();

	bool loopback = FALSE;

protected:
	unsigned long ProtocolID;
	unsigned long Flags;
	unsigned long BaudRate;
	unsigned long port;

	std::weak_ptr<PandaJ2534Device> panda_dev;

	std::queue<PASSTHRU_MSG_INTERNAL> messages;

	std::array<std::shared_ptr<J2534MessageFilter>, 10> filters;

	Mutex message_access_lock;
};