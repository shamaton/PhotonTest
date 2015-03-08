/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#pragma once

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class ProtocolHeader
			{
			public:
				short mPeerID;
				nByte mFlags;
				nByte mCommandCount;
				int   mSentTime;
				int   mChallenge;
				unsigned int mCRC;
			};
		}
	}
}