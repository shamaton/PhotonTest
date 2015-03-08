/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#include "Common-cpp/inc/Common.h"

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class NetSim : public Common::Base
			{
			public:
				using Common::ToString::toString;

				NetSim(const Common::JString& name, int delay=0, int jitter=0, int lossPerc=0, int dupPerc=0);
			
				void push(nByte* pBuf, int iCount);
				bool pop(nByte*& pBuf, int& iCount);

				virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
			protected:
				Common::JString mName;
				int mDelay;
				int mJitter;
				int mLossPerc;
				int mDupPerc;
			private:
				class Packet : public Common::Base
				{
				public:
					using Common::ToString::toString;

					Packet(nByte* buf, int size, int time=0) : mTime(time), mpBuf(buf), mSize(size) {}

					nByte* mpBuf;
					int mSize;
					int mTime;

					virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
				};

				void pushWithDelay(Packet* pPacket);

				int mPush;
				int mLost;
				int mDuplicated;
				ExitGames::Common::JVector<Packet*> mBuffer;
			};
		}
	}
}