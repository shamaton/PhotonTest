/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#pragma once

#include "Photon-cpp/inc/TrafficStats.h"

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class TrafficStatsCounter : public TrafficStats
			{
			public:
				TrafficStatsCounter(int packageHeaderSize);

				void countControlCommand(int size);
				void countReliableOpCommand(int size);
				void countUnreliableOpCommand(int size);
				void countFragmentOpCommand(int size);

				void totalPacketCountInc(int inc=1);
				void totalCommandsInPacketsInc(int inc=1);

				void setTimestampOfLastAck(int t);
				void setTimestampOfLastReliableCommand(int t);
			};
		}
	}
}