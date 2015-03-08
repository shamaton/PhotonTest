#include <iostream>

#include "NetworkLogic.h"

int getcharIfKbhit(void);

#ifdef _EG_WINDOWS_PLATFORM
#include <conio.h>

int getcharIfKbhit(void)
{
	int res = _kbhit();
	if(res)
		res = _getch();
	return res;
}

#else
#include <termios.h>
#include <fcntl.h>

int getcharIfKbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	return ch;
}
#endif

#if defined _EG_WINDOWS_PLATFORM
#	define SLEEP(ms) Sleep(ms)
#else
#	define SLEEP(ms) usleep(ms*1000)
#endif

class StdOutputListener: public OutputListener
{
public:
	virtual void write(const ExitGames::Common::JString& str);
	virtual void writeLine(const ExitGames::Common::JString& str);
};

void StdOutputListener::write(const ExitGames::Common::JString& str)
{
	std::wcout << str;
}

void StdOutputListener::writeLine(const ExitGames::Common::JString& str)
{
	write(str);
	write(L"\n");
}

class StdUI: public NetworkLogicListener
{
public:
	void main();

private:
	virtual void stateUpdate(State newState)
	{}
	void usage();
private:
	StdOutputListener mOutputListener;
};

void StdUI::usage()
{	
	mOutputListener.writeLine(L"\nPhoton LoadBalancing Demo");
	mOutputListener.writeLine(L"usage:");
	mOutputListener.writeLine(L"always:");
	mOutputListener.writeLine(L" h - print this help");
	mOutputListener.writeLine(L" 0 - exit");
	mOutputListener.writeLine(L"--------------------");
	mOutputListener.writeLine(L"outside a game room:");
	mOutputListener.writeLine(L" 1 - create game");
	mOutputListener.writeLine(L" 2 - join random game or last joined game");
	mOutputListener.writeLine(L"--------------------");
	mOutputListener.writeLine(L"inside a game room:");
	mOutputListener.writeLine(L" 1 - leave game");
	mOutputListener.writeLine(L" 2 - leave game (will come back - disconnects and quits)");
	mOutputListener.writeLine(L"--------------------");
}

void StdUI::main()
{
	
	NetworkLogic networkLogic(&mOutputListener, ExitGames::LoadBalancing::AuthenticationValues(ExitGames::LoadBalancing::CustomAuthenticationType::CUSTOM, "username=yes&token=yes"));
	usage();
	networkLogic.connect();
	bool exit = false;
	while(networkLogic.getLastInput() != INPUT_EXIT && !exit)
	{
		int ch = getcharIfKbhit();
		switch(ch)
		{
			case 'h':
				usage();
				break;
			case '0':
				networkLogic.setLastInput(INPUT_EXIT);
				exit = true; // TODO: we don't wait disconnect result currently
				break;
			default:
				networkLogic.setLastInput(INPUT_NON);
				break;
		}
		switch(ch)
		{
		case '1':
			networkLogic.setLastInput(INPUT_1);
			break;
		case '2':
			networkLogic.setLastInput(INPUT_2);
			break;
		}
		networkLogic.run();
		SLEEP(100);
	}
}

int main(int argc, const char* argv[])
{
	StdUI ui;
	ui.main();
	return 0;
}