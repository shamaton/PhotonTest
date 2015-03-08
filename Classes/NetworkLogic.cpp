#include "NetworkLogic.h"
#include "cocos2d.h"

static const ExitGames::Common::JString appId = L"no-app-id"; // set your app id here
static const ExitGames::Common::JString appVersion = L"1.0";

static const bool autoLobbbyStats = true;
static const bool useDefaultRegion = false;

static const ExitGames::Common::JString PLAYER_NAME = 
#if defined _EG_MARMALADE_PLATFORM
#	if defined I3D_ARCH_X86
#		if defined _EG_MS_COMPILER
			L"Marmalade X86 Windows";
#		else
			L"Marmalade X86 OS X";
#		endif
#	elif defined I3D_ARCH_ARM
		L"Marmalade ARM";
#	elif defined I3D_ARCH_MIPS
		L"Marmalade MIPS";
#	else
		L"unknown Marmalade platform";
#	endif
#elif defined _EG_WINDOWS_PLATFORM
	L"Windows";
#elif defined _EG_IPHONE_PLATFORM
	L"iOS";
#elif defined _EG_IMAC_PLATFORM
	L"OS X";
#elif defined _EG_ANDROID_PLATFORM
	L"Android";
#elif defined _EG_BLACKBERRY_PLATFORM
	L"Blackberry";
#elif defined _EG_PS3_PLATFORM
	L"PS3";
#elif defined _EG_LINUX_PLATFORM
	L"Linux";
#else
	L"unknown platform";
#endif

ExitGames::Common::JString& NetworkLogicListener::toString(ExitGames::Common::JString& retStr, bool /*withTypes*/) const
{
	return retStr;
}

State StateAccessor::getState(void) const
{
	return mState;
}

void StateAccessor::setState(State newState)
{
	mState = newState;
	for(unsigned int i=0; i<mStateUpdateListeners.getSize(); i++)
		mStateUpdateListeners[i]->stateUpdate(newState);
}

void StateAccessor::registerForStateUpdates(NetworkLogicListener* listener)
{
	mStateUpdateListeners.addElement(listener);
}

Input NetworkLogic::getLastInput(void) const
{
	return mLastInput;
}

void NetworkLogic::setLastInput(Input newInput)
{
	mLastInput = newInput;
}

State NetworkLogic::getState(void) const
{
	return mStateAccessor.getState();
}

// functions
NetworkLogic::NetworkLogic(const ExitGames::LoadBalancing::AuthenticationValues& authenticationValues)
#ifdef _EG_MS_COMPILER
#	pragma warning(push)
#	pragma warning(disable:4355)
#endif
	: mLoadBalancingClient(*this, appId, appVersion, PLAYER_NAME+GETTIMEMS(), false, authenticationValues, autoLobbbyStats, useDefaultRegion)
	, mLastInput(INPUT_NON)
	//, mpOutputListener(listener)
	, mLastActorNr(0)
#ifdef _EG_MS_COMPILER
#	pragma warning(pop)
#endif
{
	mStateAccessor.setState(STATE_INITIALIZED);
	mLoadBalancingClient.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // that instance of LoadBalancingClient and its implementation details
	mLogger.setListener(*this);
	mLogger.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // this class
	ExitGames::Common::Base::setListener(this);
	ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // all classes that inherit from Base 
}

void NetworkLogic::registerForStateUpdates(NetworkLogicListener* listener)
{
	mStateAccessor.registerForStateUpdates(listener);
}

void NetworkLogic::connect(void)
{
	CCLOG("connecting to Photon");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"connecting to Photon"));
	mLoadBalancingClient.connect();
    mStateAccessor.setState(STATE_CONNECTING);
}

void NetworkLogic::disconnect(void)
{
	mLoadBalancingClient.disconnect();
}

void NetworkLogic::opCreateRoom(void)
{
	// if last digits are always nearly the same, this is because of the timer calling this function being triggered every x ms with x being a factor of 10
	ExitGames::Common::JString tmp;
	mLoadBalancingClient.opCreateRoom(tmp=GETTIMEMS(), true, true, 4, ExitGames::Common::Hashtable(), ExitGames::Common::JVector<ExitGames::Common::JString>(), ExitGames::Common::JString(), 1, INT_MAX/2, 10000);
    mStateAccessor.setState(STATE_JOINING);
	CCLOG("creating room ...");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"creating room ") + tmp + L"...");
}

void NetworkLogic::opJoinRandomRoom(void)
{
	mLoadBalancingClient.opJoinRandomRoom();
}

void NetworkLogic::run(void)
{
    State state = mStateAccessor.getState();
    if(mLastInput == INPUT_EXIT && state != STATE_DISCONNECTING && state != STATE_DISCONNECTED)
	{
		disconnect();
		mStateAccessor.setState(STATE_DISCONNECTING);
		CCLOG("terminating application");
		//mpOutputListener->writeLine(L"terminating application");
	}
	else
	{
		switch(state)
		{
			case STATE_INITIALIZED:
				connect();
				mStateAccessor.setState(STATE_CONNECTING);
				CCLOG("connecting");
				//mpOutputListener->writeLine("connecting");
				break;
			case STATE_CONNECTING:
				break; // wait for callback
			case STATE_CONNECTED:

                switch(mLastInput)
                {
                    case INPUT_1: // create Game
						CCLOG("\n=========================");
                        //mpOutputListener->writeLine(L"\n=========================");
                        opCreateRoom();
                        break;
                    case INPUT_2: // join Game
						CCLOG("\n=========================");
                        //mpOutputListener->writeLine(L"\n=========================");
                        // remove false to enable rejoin
                        if(false && mLastJoinedRoom.length())
                        {
							CCLOG("%s %ls %s %d %s", "rejoining ", mLastJoinedRoom.cstr(), " with actorNr = ", mLastActorNr , "...");
                            //mpOutputListener->writeLine(ExitGames::Common::JString(L"rejoining ") + mLastJoinedRoom + " with actorNr = " + mLastActorNr + "...");
                            mLoadBalancingClient.opJoinRoom(mLastJoinedRoom, true, mLastActorNr);
                        }
                        else
                        {
							CCLOG("joining random room...");
                            //mpOutputListener->writeLine(ExitGames::Common::JString(L"joining random room..."));
                            opJoinRandomRoom();
                        }
                        mStateAccessor.setState(STATE_JOINING);
                        break;
                    default: // no or illegal input -> stay waiting for legal input
                        break;
                }
                break;
            case STATE_JOINING:
				break; // wait for callback
			case STATE_JOINED:
                sendEvent();
                switch(mLastInput)
                {
                    case INPUT_1: // leave Game
                        mLoadBalancingClient.opLeaveRoom();
						CCLOG("");
						CCLOG("leaving room");
                        //mpOutputListener->writeLine(L"");
                        //mpOutputListener->writeLine(L"leaving room");
                        mStateAccessor.setState(STATE_LEAVING);
                        break;
                    case INPUT_2: // leave Game
                        mLoadBalancingClient.opLeaveRoom(true);
						CCLOG("");
						CCLOG("leaving room (will come back)");
                        //mpOutputListener->writeLine(L"");
                        //mpOutputListener->writeLine(L"leaving room (will come back)");
                        mStateAccessor.setState(STATE_LEAVING);
                        break;
                    default: // no or illegal input -> stay waiting for legal input
                        break;
                }
                break;
            case STATE_LEAVING:
				break; // wait for callback
			case STATE_LEFT:
				mStateAccessor.setState(STATE_CONNECTED);
				break;
			case STATE_DISCONNECTING:
				break; // wait for callback
			default:
				break;        
		}
	}
	mLastInput = INPUT_NON;
	mLoadBalancingClient.service();
}

void NetworkLogic::sendEvent(void)
{
	static int64 count = 0;
	mLoadBalancingClient.opRaiseEvent(false, ++count, 0);
	//mpOutputListener->write(ExitGames::Common::JString(L"s") + count + L" ");
}

// protocol implementations

void NetworkLogic::debugReturn(ExitGames::Common::DebugLevel::DebugLevel /*debugLevel*/, const ExitGames::Common::JString& string)
{
	//mpOutputListener->debugReturn(string);
}

void NetworkLogic::connectionErrorReturn(int errorCode)
{
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	CCLOG("%s %d", "received connection error ", errorCode);
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"received connection error ") + errorCode);
    mStateAccessor.setState(STATE_DISCONNECTED);
}

void NetworkLogic::clientErrorReturn(int errorCode)
{
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	CCLOG("%s %d %s","received error ", errorCode , " from client");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"received error ") + errorCode + L" from client");
}

void NetworkLogic::warningReturn(int warningCode)
{
	EGLOG(ExitGames::Common::DebugLevel::WARNINGS, L"code: %d", warningCode);
	CCLOG("%s %d %s","received warning ", warningCode , " from client");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"received warning ") + warningCode + L" from client");
}

void NetworkLogic::serverErrorReturn(int errorCode)
{
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	CCLOG("%s %d %s","received error ", errorCode , " from server");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"received error ") + errorCode + " from server");
}

void NetworkLogic::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& /*playernrs*/, const ExitGames::LoadBalancing::Player& player)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls joined the game", player.getName().cstr());
	CCLOG("");
	//mpOutputListener->writeLine(L"");
	auto ttttt = player.getName();
	CCLOG("%s %d %s %ls %s","player ", playerNr, " ", ttttt.cstr() , " has joined the game");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"player ") + playerNr + L" " + player.getName() + L" has joined the game");
}

void NetworkLogic::leaveRoomEventAction(int playerNr, bool isInactive)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	CCLOG("");
	CCLOG("%s %d %s", "player ", playerNr, " has left the game");
	//mpOutputListener->writeLine(L"");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"player ") + playerNr + L" has left the game");
}

void NetworkLogic::disconnectEventAction(int playerNr)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	CCLOG("");
	CCLOG("%s %d %s", "player ", playerNr, " has disconnected");
	//mpOutputListener->writeLine(L"");
	//mpOutputListener->writeLine(ExitGames::Common::JString(L"player ") + playerNr + L" has disconnected");
}

//void NetworkLogic::customEventAction(int /*playerNr*/, nByte /*eventCode*/, const ExitGames::Common::Object& eventContent)
//{
//	// you do not receive your own events, unless you specify yourself as one of the receivers explicitly, so you must start 2 clients, to receive the events, which you have sent, as sendEvent() uses the default receivers of opRaiseEvent() (all players in same room like the sender, except the sender itself)
//	EGLOG(ExitGames::Common::DebugLevel::ALL, L"");
//	//mpOutputListener->write(ExitGames::Common::JString(L"r") + ExitGames::Common::ValueObject<long long>(eventContent).getDataCopy() + L" ");
//}


void NetworkLogic::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent)
{
	ExitGames::Common::Hashtable* event;
 
	switch (eventCode) {
		case 1:
			event = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable*>(eventContent).getDataCopy();
			float x = ExitGames::Common::ValueObject<float>(event->getValue(1)).getDataCopy();
			float y = ExitGames::Common::ValueObject<float>(event->getValue(2)).getDataCopy();
			eventQueue.push({static_cast<float>(playerNr), x, y});
			break;
	}
}

void NetworkLogic::connectReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
        mStateAccessor.setState(STATE_DISCONNECTING);
		return;
	}
	CCLOG("connected");
	//mpOutputListener->writeLine(L"connected");
	mStateAccessor.setState(STATE_CONNECTED);
}

void NetworkLogic::disconnectReturn(void)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	CCLOG("disconnected");
	//mpOutputListener->writeLine(L"disconnected");
    mStateAccessor.setState(STATE_DISCONNECTED);
}

void NetworkLogic::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		CCLOG("%s %ls", "opCreateRoom() failed: ", errorString.cstr());
		//mpOutputListener->writeLine(L"opCreateRoom() failed: " + errorString);
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}
	mLastJoinedRoom = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	mLastActorNr = localPlayerNr;

	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	auto tttt = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	CCLOG("%s %ls %s" , "... room ", tttt.cstr(), " has been created");
	CCLOG("regularly sending dummy events now");
	//mpOutputListener->writeLine(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been created");
	//mpOutputListener->writeLine(L"regularly sending dummy events now");
	mStateAccessor.setState(STATE_JOINED);
	
	// ルーム内で割り当てられたプレイヤー番号を取得する
	CCLOG("player nr = %d", localPlayerNr);
	playerNr = localPlayerNr;
}

void NetworkLogic::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{		
		mLastJoinedRoom = "";
		mLastActorNr = 0;
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		CCLOG("%s %ls", "opJoinRoom() failed: ", errorString.cstr());
		//mpOutputListener->writeLine(L"opJoinRoom() failed: " + errorString);
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	auto tttt = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	CCLOG("%s %ls %s" , "... room ", tttt.cstr(), " has been successfully joined");
	CCLOG("regularly sending dummy events now");
	//mpOutputListener->writeLine(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
	//mpOutputListener->writeLine(L"regularly sending dummy events now");
	mStateAccessor.setState(STATE_JOINED);
	
	// ルーム内で割り当てられたプレイヤー番号を取得する
	CCLOG("player nr = %d", localPlayerNr);
	playerNr = localPlayerNr;
}

void NetworkLogic::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		CCLOG("%s %ls", "opJoinRandomRoom() failed: ", errorString.cstr());
		//mpOutputListener->writeLine(L"opJoinRandomRoom() failed: " + errorString);
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}

	mLastJoinedRoom = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	mLastActorNr = localPlayerNr;
	
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	auto tttt = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	CCLOG("%s %ls %s" , "... room ", tttt.cstr(), " has been successfully joined");
	CCLOG("regularly sending dummy events now");
	//mpOutputListener->writeLine(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
	//mpOutputListener->writeLine(L"regularly sending dummy events now");
	mStateAccessor.setState(STATE_JOINED);
	
	// ルーム内で割り当てられたプレイヤー番号を取得する
	CCLOG("player nr = %d", localPlayerNr);
	playerNr = localPlayerNr;
}

void NetworkLogic::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		CCLOG("%s %ls", "opLeaveRoom() failed: ", errorString.cstr());
		//mpOutputListener->writeLine(L"opLeaveRoom() failed: " + errorString);
        mStateAccessor.setState(STATE_DISCONNECTING);
		return;
	}
	mStateAccessor.setState(STATE_LEFT);
	CCLOG("room has been successfully left");
	//mpOutputListener->writeLine(L"room has been successfully left");
}

void NetworkLogic::joinLobbyReturn(void)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	CCLOG("joined lobby");
	//mpOutputListener->writeLine(L"joined lobby");
}

void NetworkLogic::leaveLobbyReturn(void)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	CCLOG("left lobby");
	//mpOutputListener->writeLine(L"left lobby");
}

void NetworkLogic::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStats>& lobbyStats)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"onLobbyStatsUpdate: %ls", lobbyStats.toString().cstr());
	auto tttt = lobbyStats.toString();
	CCLOG("%s %ls", "LobbyStats: ", tttt.cstr());
	//mpOutputListener->writeLine(L"LobbyStats: " + lobbyStats.toString());
}

void NetworkLogic::onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStats>& lobbyStats)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"onLobbyStatsUpdate: %ls", lobbyStats.toString().cstr());
	CCLOG("%s %ls", "LobbyStats: ", lobbyStats.toString().cstr());
	//mpOutputListener->writeLine(L"LobbyStats: " + lobbyStats.toString());
}

void NetworkLogic::onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegions, const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegionServers)
{
    EGLOG(ExitGames::Common::DebugLevel::INFO, L"onAvailableRegions: %ls", availableRegions.toString().cstr(), availableRegionServers.toString().cstr());
	CCLOG("%s %ls %s %ls", "onAvailableRegions: ", availableRegions.toString().cstr(), "/", availableRegionServers.toString().cstr());
	//mpOutputListener->writeLine(L"onAvailableRegions: " + availableRegions.toString() + L" / " + availableRegionServers.toString());
	
	// select first region from list
	CCLOG("%s %ls ", "selecting region: ", availableRegions[0].cstr());
    //mpOutputListener->writeLine(L"selecting region: " + availableRegions[0]);
    mLoadBalancingClient.selectRegion(availableRegions[0]);
}


////////////////////
bool NetworkLogic::isRoomExists(void)
{
	if (mLoadBalancingClient.getRoomList().getIsEmpty()) {
		return false;
	}
 
	return true;
}

void NetworkLogic::sendEvent(nByte code, ExitGames::Common::Hashtable* eventContent)
{
	mLoadBalancingClient.opRaiseEvent(true, eventContent, 1, code);
}