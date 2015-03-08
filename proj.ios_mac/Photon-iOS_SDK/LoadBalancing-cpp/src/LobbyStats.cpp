/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#include "LoadBalancing-cpp/inc/LobbyStats.h"

/** @file LobbyStats.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;

		LobbyStats::LobbyStats(const JString& name, nByte type, int peerCount, int roomCount)
			: mName(name)
			, mType(type)
			, mPeerCount(peerCount)
			, mRoomCount(roomCount) 
		{
		}

		JString LobbyStats::getName(void) const
		{
			return mName;
		}

		nByte LobbyStats::getType(void) const
		{
			return mType;
		}

		int LobbyStats::getPeerCount(void) const
		{
			return mPeerCount;
		}

		int LobbyStats::getRoomCount(void) const
		{
			return mRoomCount;
		}

		JString& LobbyStats::toString(JString& retStr, bool withTypes) const
		{
			return retStr += L"{name=" + mName + L", type=" + mType + L", peerCount=" + mPeerCount + L", roomCount=" + mRoomCount + L"}";
		}
	}
}