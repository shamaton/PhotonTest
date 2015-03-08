#pragma once

#include "LoadBalancing-cpp/inc/Listener.h"
#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/LobbyStats.h"
#include "BaseView.h"

struct LocalPlayer
{
	LocalPlayer();
	int x;
	int y;
	int color;
	unsigned long lastUpdateTime;
};

class LoadBalancingListener : public ExitGames::LoadBalancing::Listener
{
	ExitGames::LoadBalancing::Client* lbc;
	BaseView* view;
public: 
	LoadBalancingListener(BaseView* view);
	~LoadBalancingListener()  
	{ 
		delete view; 
	}

	void setLBC(ExitGames::LoadBalancing::Client* lbc) 
	{
		this->lbc = lbc;
	}
	void connect() 
	{ 
		lbc->connect(); 
	}
	void setUseGroups(bool value)
	{
		useGroups = value;
	}
	bool getUseGroups(void)
	{
		return useGroups;
	}
	void updateGroups();
	void setSendGroup(nByte value)
	{
		sendGroup = value;
	}
	void service();
	void createRoom();
	void setAutomove(bool a) 
	{ 
		automove = a; 
	}
	bool getAutomove(void) 
	{ 
		return automove;
	}
	void changeRandomColor();
	void nextGridSize();
	bool setLocalPlayerPos(int x, int y);
	void moveLocalPlayer();
private:
	LocalPlayer localPlayer;
	int localPlayerNr;
	int gridSize;
	ExitGames::Common::JString map;
	bool automove;
	bool useGroups;
	int sendGroup; // forces group in outgoing events even if useGroups = false

	//From Common::BaseListener
	// receive and print out Photon datatypes debug out here
	virtual void debugReturn(const ExitGames::Common::JString& string);

	//From LoadBalancing::LoadBalancingListener
	// receive and print out Photon LoadBalancing debug out here
	virtual void debugReturn(ExitGames::Common::DebugLevel::DebugLevel debugLevel, const ExitGames::Common::JString& string);

	// implement your error-handling here
	virtual void connectionErrorReturn(int errorCode);
	virtual void clientErrorReturn(int errorCode);
	virtual void warningReturn(int warningCode);
	virtual void serverErrorReturn(int errorCode);

	// events, triggered by certain operations of all players in the same room
	virtual void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player);
	virtual void leaveRoomEventAction(int playerNr, bool isInactive);
	virtual void disconnectEventAction(int playerNr);
	virtual void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent);

	// callbacks for operations on PhotonLoadBalancing server
	virtual void connectReturn(int errorCode, const ExitGames::Common::JString& errorString);
	virtual void disconnectReturn(void);
	virtual void createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString);
	virtual void gotQueuedReturn(void);
	virtual void joinLobbyReturn(void);
	virtual void leaveLobbyReturn(void);
	virtual void onRoomListUpdate(void);
	virtual void onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStats>& lobbyStats);
	virtual void onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStats>& lobbyStats);
	virtual void onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes);

	void updateState(void);
	void afterRoomJoined(int localPlayerNr);
	bool updateGridSize(const ExitGames::Common::Hashtable& props);
	void raiseColorEvent(void);
	int getGroupByPos(void);
};