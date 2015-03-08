/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#pragma once

#include "Photon-cpp/inc/Internal/EnetConnect.h"
#include "Photon-cpp/inc/Internal/NetSim.h"

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class EnetConnectNetSim : public EnetConnect
			{
			public:
				EnetConnectNetSim(PhotonConnectListener& listener);
				~EnetConnectNetSim(void);
				void service(void);
			protected:
				// pops from mpRecvNetSim 
				int recvBuffer(nByte** ppBuff, int* pBytesRead);
				// pushes to mpSendNetSim
				int sendBuffer(char* pBuff, int& iSize);
				NetSim* mpSendNetSim;
				NetSim* mpRecvNetSim;
			private:
				typedef EnetConnect super;

				// pops from mpSendNetSim and sends to socket
				void serviceSend(void);
				// receives from socket and pushes to mpRecvNetSim
				void serviceRecv(void);
			};
		}
	}
}