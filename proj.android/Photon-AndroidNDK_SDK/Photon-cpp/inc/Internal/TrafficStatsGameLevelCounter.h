/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#pragma once

#include "Photon-cpp/inc/TrafficStatsGameLevel.h"

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class TrafficStatsGameLevelCounter : public TrafficStatsGameLevel
			{
			public:
				void countOperation(int operationBytes);
				void countResult(int resultBytes);
				void countEvent(int eventBytes);
				void timeForResponseCallback(nByte code, int time);
				void timeForEventCallback(nByte code, int time);
		        void timeForMessageCallback(int time);
		        void timeForRawMessageCallback(int time);
				void dispatchIncomingCommandsCalled(void);
				void sendOutgoingCommandsCalled(void);
			};
		}
	}
}