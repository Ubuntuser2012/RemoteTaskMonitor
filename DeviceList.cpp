#include "stdafx.h"
#include "../common/types.h"
extern "C" {
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"

int DEVICE_LIST::Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const
{
	DEVICE *d1 = (DEVICE*) visited_obj;
	DEVICE *d2 = (DEVICE*) user_obj;

	if(d1->getIp() > d2->getIp())
		return 1;
	else if(d1->getIp() < d2->getIp())
		return -1;
	else 
		return 0;
}