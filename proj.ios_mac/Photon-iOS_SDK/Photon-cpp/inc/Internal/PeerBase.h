/* Exit Games Photon - C++ Client Lib
 * Copyright (C) 2004-2015 by Exit Games GmbH. All rights reserved.
 * http://www.exitgames.com
 * mailto:developer@exitgames.com
 */

#pragma once

#include "Photon-cpp/inc/Internal/EnetCommand.h"
#include "Photon-cpp/inc/Internal/PhotonConnect.h"
#include "Photon-cpp/inc/OperationRequest.h"
#include "Photon-cpp/inc/PhotonListener.h"
#include "Photon-cpp/inc/Enums/PeerState.h"
#include "Photon-cpp/inc/Internal/Enums/ConnectionState.h"
#include "Photon-cpp/inc/Internal/Enums/InternalProperties.h"
#include "Photon-cpp/inc/Internal/TrafficStatsCounter.h"
#include "Photon-cpp/inc/Internal/TrafficStatsGameLevelCounter.h"

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class PeerBase : public PhotonConnectListener
			{
			public:
				PeerBase(PhotonListener& listener, const Common::Logger& logger, int packageHeaderSize=0); // packageHeaderSize required for enet traffic stats initialization
				virtual ~PeerBase(void);

				virtual bool connect(const EG_CHAR* ipAddr, const nByte appID[32]) = 0;
				virtual bool startConnection(const char* ipAddr) = 0;
				virtual void disconnect(void) = 0;
				virtual void stopConnection(void) = 0;
				virtual void service(bool dispatch);
				virtual void serviceBasic(void);
				virtual bool sendOutgoingCommands(void) = 0;
				virtual bool sendAcksOnly(void);
				virtual bool dispatchIncomingCommands(void) = 0;
				virtual bool opCustom(const OperationRequest* operationRequest, bool sendReliable, nByte channelId, bool encrypt, nByte msgType);
				virtual void fetchServerTimestamp(void) = 0;
				virtual bool establishEncryption(void);

				virtual int getServerTimeOffset(void) const;
				virtual int getServerTime(void) const;
				virtual int getBytesOut(void) const;
				virtual int getBytesIn(void) const;
				virtual PeerState::PeerState getPeerState(void) const;
				virtual int getSentCountAllowance(void) const;
				virtual void setSentCountAllowance(int sentCountAllowance);
				virtual int getTimePingInterval(void) const;
				virtual void setTimePingInterval(int timePingInterval);
				virtual int getRoundTripTime(void) const;
				virtual int getRoundTripTimeVariance(void) const;
				virtual int getTimestampOfLastReceive(void) const;
				virtual int getIncomingReliableCommandsCount(void) const = 0;
				virtual int getByteCountLastOperation(void) const;
				virtual int getByteCountCurrentDispatch(void) const;
				virtual short getPeerID(void) const;
				virtual int getDisconnectTimeout(void) const;
				virtual void setDisconnectTimeout(int disconnectTimeout);
				virtual int getQueuedIncomingCommands(void) const = 0;
				virtual int getQueuedOutgoingCommands(void) const = 0;
				virtual const char* getServerAdress(void) const;
				virtual bool getIsEncryptionAvailable(void) const;
				virtual const PhotonListener* getListener(void) const;
				virtual int getResentReliableCommands(void) const;
				virtual int getLimitOfUnreliableCommands(void) const;
				virtual void setLimitOfUnreliableCommands(int);

				static unsigned short getPeerCount(void);

				virtual void onSendCommands(int errorcode);

				virtual void setCRCEnabled(bool);
				virtual bool getCrcEnabled(void) const;
				virtual int getPacketLossByCrc(void) const;
	
				void setTrafficStatsEnabled(bool);
				bool getTrafficStatsEnabled(void) const;
				// total time while traffic enabled 
				long getTrafficStatsEnabledTime(void) const;
				void resetTrafficStats(void);
                void resetTrafficStatsMaximumCounters(void);
				const Photon::TrafficStats& getTrafficStatsIncoming(void) const;
				const Photon::TrafficStats& getTrafficStatsOutgoing(void) const;
				const Photon::TrafficStatsGameLevel& getTrafficStatsGameLevel(void) const;
			protected:
				virtual void cleanup(void);
				virtual void reset(void);
				virtual void onDisconnected(void);
				virtual void send(nByte cType, nByte* payload, unsigned int payloadSize, nByte channelId) = 0;
				virtual bool serializeOperation(const OperationRequest* pOperationRequest, nByte** buffer,  int* buffSize, bool encrypt, nByte msgType);
				virtual bool deserializeOperation(nByte* inBuff, int inBuffLength);
				virtual void deserializeOperationResponse(nByte* inBuff, bool isEncrypted, int waitForBytes, nByte msgType);
				virtual void deserializeEvent(nByte* inBuff, bool isEncrypted, int waitForBytes);
				virtual void updateRoundTripTimeAndVariance(int time);
				virtual void clearAllQueues(void) = 0;
				virtual bool opExchangeKeysForEncryption(void);
				virtual void deriveSharedKey(const OperationResponse* operationResponse);
				virtual void initCallback(void);

				static unsigned short mPeerCount;

				short mPeerID;
				int mChallenge;
				Common::ANSIString mPeerIpAddr;
				ConnectionState::ConnectionState mConnectionState;
				nByte mChannelCountUserChannels; // this is the number of user-channels. There is an additional exclusive channel for system messages.
				nByte mInitBytes[InternalProperties::INIT_BYTES_LENGTH];
				PhotonConnect* mConnection;
				int mTimeBase;
				int mDisconnectTimeout;
				int mTimeLastReceive;
				int mTimePingInterval;
				int mServerTimeOffset;
				bool mServerTimeOffsetIsAvailable;
				int mRoundTripTime;
				int mRoundTripTimeVariance;
				int mLastRoundTripTime;
				int mLowestRoundTripTime;
				int mLastRoundTripTimeVariance;
				int mHighestRoundTripTimeVariance;
				int mTimestampOfLastReceive;
				int mByteCountLastOperation;
				int mByteCountCurrentDispatch;
				bool mIsSendingCommand; //Indicates if sending command is in progress
				nByte* mSecretKey; // Client private key
				nByte* mSharedKeyHash; // Shared key hash, used as key for encrypting messages
				int mSecretKeySize; // Client private key size
				int mPublicKeySize; // Client public key size
				bool mIsEncryptionAvailable;
				unsigned short mWarningTresholdQueueOutgoingReliable;
				unsigned short mWarningTresholdQueueOutgoingUnreliable;
				unsigned short mWarningTresholdQueueOutgoingAcks;
				unsigned short mWarningTresholdQueueIncomingReliable;
				unsigned short mWarningTresholdQueueIncomingUnreliable;
				unsigned short mWarningTresholdQueueSent;
				bool mApplicationIsInitialized;
				bool mDebugUseShortcut;
			#if defined _EG_MARMALADE_PLATFORM && defined I3D_ARCH_X86 && defined EG_DEBUGGER
				bool mFiredMetricsWarning;
			#endif
				PhotonListener& mListener;
				const Common::Logger& mLogger;

				bool mCRCEnabled;
				int mPacketLossByCRC;

				Photon::Internal::TrafficStatsCounter* mpTrafficStatsIncoming;
				Photon::Internal::TrafficStatsCounter* mpTrafficStatsOutgoing;
				Photon::Internal::TrafficStatsGameLevelCounter* mpTrafficStatsGameLevel;
				long mTrafficStatsEnabledTime;
				int mTrafficStatsEnabled; // 0 if disabled, enabling time if enabled

			private:
				void cleanupNonHierarchical(void);

				friend EnetCommand::EnetCommand(EnetPeer* const pEnetPeer, nByte* pBuffer, unsigned int bufferSize, int* pRead, int sentTime);
				friend void PhotonConnect::service(void);
				friend int PhotonConnect::sendPackage(const nByte* src, int count);
			};
		}
	}
}