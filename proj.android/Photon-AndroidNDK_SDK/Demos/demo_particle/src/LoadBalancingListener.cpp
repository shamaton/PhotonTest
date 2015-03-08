#include <stdlib.h>
#include "Common-cpp/inc/UTF8String.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/EventCode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/ParameterCode.h"
#include "LoadBalancingListener.h"
#include "DemoConstants.h"

using namespace ExitGames::Common;
using namespace ExitGames::LoadBalancing;

static int randomColor(int from = 0, int to = 256) 
{
	int r = from + rand() % (to - from);
	int g = from + rand() % (to - from);
	int b = from + rand() % (to - from);
	return (r << 16) + (g << 8) + b;
}

const char* PeerStatesStr[] = {
	"Uninitialized",
	"PeerCreated",
	"ConnectingToNameserver",
	"ConnectedToNameserver",
	"DisconnectingFromNameserver",
	"Connecting",
	"Connected",
	"WaitingForCustomAuthenticationNextStepCall",
	"Authenticated",
	"JoinedLobby",
	"DisconnectingFromMasterserver",
	"ConnectingToGameserver",
	"ConnectedToGameserver",
	"AuthenticatedOnGameServer",
	"Joining",
	"Joined",
	"Leaving",
	"Left",
	"DisconnectingFromGameserver",
	"ConnectingToMasterserver",
	"ConnectedComingFromGameserver",
	"AuthenticatedComingFromGameserver",
	"Disconnecting",
	"Disconnected"
};

// TODO: update PeerStatesStr when PeerStates changes
// Checker below checks count match only
class PeerStatesStrChecker
{
public:
	PeerStatesStrChecker()
	{
		assert(sizeof(PeerStatesStr)/sizeof(const char*) == PeerStates::Disconnected + 1);
	}
} checker;

LocalPlayer::LocalPlayer() : x(0), y(0), color(randomColor(100)), lastUpdateTime(0) {}

LoadBalancingListener::LoadBalancingListener(BaseView* view) : lbc(NULL), view(view), gridSize(DEFAULT_GRID_SIZE), map(DEFAULT_MAP), automove(true), localPlayerNr(0), useGroups(true), sendGroup(0) {}

void LoadBalancingListener::debugReturn(const JString& string)
{
	view->info(string.UTF8Representation().cstr());
}

void LoadBalancingListener::debugReturn(DebugLevel::DebugLevel /*debugLevel*/, const JString& string)
{
	debugReturn(string);
}

void LoadBalancingListener::connectionErrorReturn(int errorCode)
{
	view->warn("connection failed with error %d", errorCode);
	updateState();
	// reconnect on error
	//	connect();
}

void LoadBalancingListener::clientErrorReturn(int errorCode)
{
	view->warn("received error %d from client", errorCode);
	updateState();
}

void LoadBalancingListener::warningReturn(int warningCode)
{
	view->warn("received warning %d from client", warningCode);
}

void LoadBalancingListener::serverErrorReturn(int errorCode)
{
	view->warn("received error %d from server", errorCode);
	updateState();
}

void LoadBalancingListener::joinRoomEventAction(int playerNr, const JVector<int>& playernrs, const Player& player)
{
	view->info( "player %d %s has joined the game", playerNr, player.getName().UTF8Representation().cstr());
	view->addPlayer(playerNr, player.getName().UTF8Representation().cstr(), player.getNumber() == lbc->getLocalPlayer().getNumber());
	view->changePlayerColor(localPlayerNr, localPlayer.color);
	updateGroups();
}

void LoadBalancingListener::leaveRoomEventAction(int playerNr, bool isInactive)
{    
	if(isInactive)
		view->info( "player %d has suspended the game", playerNr);		
	else
	{
		view->info( "player %d has abandoned the game", playerNr);
		view->removePlayer(playerNr);
	}
}

void LoadBalancingListener::disconnectEventAction(int playerNr)
{
	view->info( "player %d has disconnected", playerNr);
}

void LoadBalancingListener::customEventAction(int playerNr, nByte eventCode, const Object& eventContentObj)
{
	ExitGames::Common::Hashtable eventContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContentObj).getDataCopy();
	if(eventCode == 1)
	{
		Object const* obj = eventContent.getValue("1");
		if(obj == NULL) obj = eventContent.getValue((nByte)1);
		if(obj == NULL) obj = eventContent.getValue(1);
		if(obj == NULL) obj = eventContent.getValue(1.0);
		if(obj) 
		{
			int color = (int)(obj->getType() == 'd' ?  ((ValueObject<double>*)obj)->getDataCopy() : ((ValueObject<int>*)obj)->getDataCopy());
			view->changePlayerColor(playerNr, color);
		}
		else 
			view->warn("bad color event");
	}
	else if(eventCode == 2)
	{
		Object const* obj = eventContent.getValue("1");
		if(obj == NULL) obj = eventContent.getValue((nByte)1);
		if(obj == NULL) obj = eventContent.getValue(1);
		if(obj == NULL) obj = eventContent.getValue(1.0);
		if(obj && obj->getDimensions() == 1 && obj->getSizes()[0] == 2) 
		{
			int x = 0; int y = 0;
			if(obj->getType() == 'd') 
			{
				double* data = ((ValueObject<double*>*)obj)->getDataCopy();
				x = (int)data[0];
				y = (int)data[1];
			}
			if(obj->getType() == 'i') 
			{
				int* data = ((ValueObject<int*>*)obj)->getDataCopy();
				x = (int)data[0];
				y = (int)data[1];
			}
			else if(obj->getType() == 'b') 
			{
				nByte* data = ((ValueObject<nByte*>*)obj)->getDataCopy();
				x = (int)data[0];
				y = (int)data[1];
			}
			else if(obj->getType() == 'z') 
			{
				Object* data = ((ValueObject<Object*>*)obj)->getDataCopy();
				if(data[0].getType() == 'i') 
				{
					x = ((ValueObject<int>*)(data + 0))->getDataCopy();
					y = ((ValueObject<int>*)(data + 1))->getDataCopy();
				}
				else 
				{
					x = (int)((ValueObject<double>*)(data + 0))->getDataCopy();
					y = (int)((ValueObject<double>*)(data + 1))->getDataCopy();
				}
			}
			view->changePlayerPos(playerNr, x, y);
		}
		else 
			view->warn("Bad position event");
	}
}

void LoadBalancingListener::connectReturn(int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		view->info("connected");
		lbc->opJoinRandomRoom();
	}
	else
		view->warn("Warn: connect failed %d %s", errorCode, errorString.UTF8Representation().cstr());
}

void LoadBalancingListener::disconnectReturn(void)
{
	updateState();
	view->info("disconnected");
	view->initPlayers();
}

void LoadBalancingListener::createRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		view->info("room has been created");
		afterRoomJoined(localPlayerNr);
	}
	else
		view->warn("Warn: opCreateRoom() failed: %s", errorString.UTF8Representation().cstr());
}

void LoadBalancingListener::joinRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		view->info("game room has been successfully joined");
		afterRoomJoined(localPlayerNr);
	}
	else
		view->info("opJoinRoom() failed: %s", errorString.UTF8Representation().cstr());
}

void LoadBalancingListener::joinRandomRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::NO_MATCH_FOUND)
		createRoom();
	else if(errorCode == ErrorCode::OK)
	{
		view->info("game room has been successfully joined");
		afterRoomJoined(localPlayerNr);
	}
	else
		view->info("opJoinRandomRoom() failed: %s", errorString.UTF8Representation().cstr());
}

void LoadBalancingListener::leaveRoomReturn(int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
		view->info("game room has been successfully left");
	else
		view->info("opLeaveRoom() failed: %s", errorString.UTF8Representation().cstr());
	view->initPlayers();
}

void LoadBalancingListener::gotQueuedReturn(void)
{
	updateState();
}

void LoadBalancingListener::joinLobbyReturn(void)
{
	view->info("joined lobby");
	updateState();
}

void LoadBalancingListener::leaveLobbyReturn(void)
{
	view->info("left lobby");
	updateState();
}

void LoadBalancingListener::onLobbyStatsUpdate(const JVector<LobbyStats>& res)
{
	view->info("===================== lobby stats update");
	for(unsigned int i=0; i<res.getSize(); ++i) 
		view->info(res[i].toString().UTF8Representation().cstr());

	// lobby stats request test
	JVector<LobbyStats> ls;
	ls.addElement(LobbyStats());
	ls.addElement(LobbyStats(L"AAA"));
	ls.addElement(LobbyStats(L"BBB"));
	ls.addElement(LobbyStats(L"CCC", LobbyType::DEFAULT));
	ls.addElement(LobbyStats(L"AAA", LobbyType::SQL_LOBBY));
	lbc->opLobbyStats(ls);
	//	lbc->opLobbyStats();
}

void LoadBalancingListener::onLobbyStatsResponse(const ExitGames::Common::JVector<LobbyStats>& res)
{
	view->info("===================== lobby stats response");
	for(unsigned int i=0; i<res.getSize(); ++i) 
		view->info(res[i].toString().UTF8Representation().cstr());
}

void LoadBalancingListener::onRoomListUpdate()
{	
	const JVector<Room*>& rooms = lbc->getRoomList();
	const char** names = MemoryManagement::allocateArray<const char*>(rooms.getSize());
	for(unsigned int i=0; i<rooms.getSize(); ++i) 
	{
		const Room* r = rooms.getElementAt(i);
		UTF8String u = r->getName();
		const char* name = u.cstr();
		char* tmp = MemoryManagement::allocateArray<char>(strlen(name) + 1);
		strcpy(tmp, name);
		names[i] = tmp;
	}
	view->updateRoomList(names, rooms.getSize());
	for(unsigned int i = 0; i < rooms.getSize();++i) 
		MemoryManagement::deallocateArray(names[i]);
	MemoryManagement::deallocateArray(names);
}

void LoadBalancingListener::onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes) 
{
	if(updateGridSize(changes)) 
		view->setupScene(gridSize);
}

void LoadBalancingListener::updateState() 
{
	int state = lbc->getState();
	view->updateState(state, PeerStatesStr[state], state == PeerStates::Joined ? ((ExitGames::Common::UTF8String)lbc->getCurrentlyJoinedRoom().getName()).cstr() : NULL);
}

bool LoadBalancingListener::updateGridSize(const ExitGames::Common::Hashtable& props) 
{
	if(props.contains("s")) 
	{
		const Object* v = props.getValue("s");
		char x = v->getType();
		switch(v->getType()) 
		{
		case 'i':
			gridSize = ((ValueObject<int>*)v)->getDataCopy();
			return true;
		case 'd':
			gridSize = (int)((ValueObject<double>*)v)->getDataCopy();
			return true;
		}
	}
	return false;
}

void LoadBalancingListener::afterRoomJoined(int localPlayerNr)
{
	view->info( "afterRoomJoined: localPlayerNr=", localPlayerNr );
	this->localPlayerNr = localPlayerNr;
	MutableRoom& myRoom = lbc->getCurrentlyJoinedRoom();
	Hashtable props = myRoom.getCustomProperties();
	updateGridSize(props);
	if(props.contains("m"))
		map = ((ValueObject<JString>*)props.getValue("m"))->getDataCopy();

	view->initPlayers();
	view->setupScene(gridSize);

	const JVector<Player*>& players = myRoom.getPlayers();
	for(unsigned int i=0; i<players.getSize(); ++i)
	{
		const Player* p = players[i];
		view->addPlayer(p->getNumber(), p->getName().UTF8Representation().cstr(), p->getNumber() == lbc->getLocalPlayer().getNumber());
	}
	view->changePlayerColor(localPlayerNr, localPlayer.color);
	raiseColorEvent();
}

void LoadBalancingListener::createRoom() 
{
	char name[16];
	sprintf(name, "native-%d",rand() % 100);
	Hashtable props;
	props.put("s", gridSize);
	props.put("m", map);
	lbc->opCreateRoom(name, true, true, 0, props);
	view->info( "Creating room %s", name );
}

void LoadBalancingListener::service()
{

	unsigned long t = GETTIMEMS();
	if((t - localPlayer.lastUpdateTime) > PLAYER_UPDATE_INTERVAL_MS) 
	{
		localPlayer.lastUpdateTime = t;
		if(lbc->getState() == PeerStates::Joined && automove) 
			moveLocalPlayer();
	}
}

void LoadBalancingListener::moveLocalPlayer()
{
	int d = rand() % 8;
	int xOffs[] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int yOffs[] = {1, 1, 1, 0, 0, -1, -1, -1};
	int x = localPlayer.x + xOffs[d];
	int y = localPlayer.y += yOffs[d];
	if(x < 0) x = 1;
	if(x >= gridSize) x = gridSize - 2;
	if(y < 0) y = 1;
	if(y >= gridSize) y = gridSize - 2;
	setLocalPlayerPos(x, y);
}

bool LoadBalancingListener::setLocalPlayerPos(int x, int y)
{
	if(x >= 0 && x < gridSize && y >= 0 && y < gridSize) 
	{
		int prevGroup = getGroupByPos();
		localPlayer.x = x;
		localPlayer.y = y;
		if(prevGroup != getGroupByPos()) 
			updateGroups();
		Hashtable data;
		nByte coords[] = {localPlayer.x, localPlayer.y};
		data.put((nByte)1, coords, 2);

		if(sendGroup)
			lbc->opRaiseEvent(false, data, 2, 0, 0, 0, 0, 0, sendGroup);
		else
		{
			if(useGroups)
				lbc->opRaiseEvent(false, data, 2, 0, 0, 0, 0, 0, getGroupByPos());
			else
				lbc->opRaiseEvent(false, data, 2);
		}
		view->changePlayerPos(localPlayerNr, localPlayer.x, localPlayer.y);
		return true;
	}
	else 
		return false;
}

void LoadBalancingListener::raiseColorEvent()
{
	Hashtable data;	
	data.put((nByte)1, localPlayer.color);
	lbc->opRaiseEvent(true, data, 1,0,ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE);
}

int LoadBalancingListener::getGroupByPos(void)
{
	int xp = localPlayer.x * GROUPS_PER_AXIS / gridSize;
	int yp = localPlayer.y * GROUPS_PER_AXIS / gridSize;
	return 1 + xp + yp * GROUPS_PER_AXIS;
}

void LoadBalancingListener::changeRandomColor()
{
	localPlayer.color = randomColor(100);
	raiseColorEvent();
	view->changePlayerColor(localPlayerNr, localPlayer.color);
}

void LoadBalancingListener::nextGridSize()
{
	gridSize = gridSize << 1;
	if(gridSize > GRID_SIZE_MAX) 
		gridSize = GRID_SIZE_MIN;
	view->info("nextGridSize: %d", gridSize);
	lbc->getCurrentlyJoinedRoom().addCustomProperty("s", gridSize);
	view->setupScene(gridSize);
}

void LoadBalancingListener::updateGroups()
{
	if(lbc->getIsInRoom())
	{	
		ExitGames::Common::JVector<nByte> remove;
		if(useGroups)
		{
			ExitGames::Common::JVector<nByte> add;
			add.addElement(getGroupByPos());
			lbc->opChangeGroups(&remove, &add);
		}
		else
			lbc->opChangeGroups(&remove, NULL);
	}
}