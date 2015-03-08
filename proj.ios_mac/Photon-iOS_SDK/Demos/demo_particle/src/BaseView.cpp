#include <stdarg.h>
#include <stdio.h>
#include <stdarg.h>

#include "BaseView.h"

#ifdef _EG_ANDROID_PLATFORM
#	include <android/log.h>
#endif

void BaseView::info(const char* format, ...) 
{
	fprintf(stdout, "INFO: ");
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n");
#ifdef _EG_ANDROID_PLATFORM
	__android_log_vprint(ANDROID_LOG_INFO, "DemoParticle", format, argptr);
#endif
}

void BaseView::warn(const char* format, ...) 
{
	fprintf(stderr, "WARN: ");
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	fprintf(stderr, "\n");
#ifdef _EG_ANDROID_PLATFORM
	__android_log_vprint(ANDROID_LOG_INFO, "DemoParticle", format, argptr);
#endif
}

void BaseView::error(const char* format, ...) 
{
	fprintf(stderr, "ERROR: ");
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	fprintf(stderr, "\n");
#ifdef _EG_ANDROID_PLATFORM
	__android_log_vprint(ANDROID_LOG_INFO, "DemoParticle", format, argptr);
#endif
}

void BaseView::updateState(int state, const char* stateStr, const char* joinedRoomName) 
{
	info("state: %d/%s", state, stateStr);
}
void BaseView::addPlayer(int playerNr, const char* playerName, bool local) 
{
	info("player added: %s / %d ", playerName, playerNr);
}
void BaseView::removePlayer(int playerNr) 
{
	info("player removed: %d", playerNr);
}
void BaseView::changePlayerColor(int playerNr, int color) 
{
	info("player %d color: %d", playerNr, color);
}
void BaseView::changePlayerPos(int playerNr, int x, int y) 
{
	info("player %d pos: %d, %d", playerNr, x, y);
}
void BaseView::setupScene(int gridSize)
{
	info("setup scene with gridSize=%d", gridSize);
}

void BaseView::updateRoomList(const char** roomNames, unsigned int size) 
{
	info("room list:");
	for(unsigned int i=0; i<size; ++i) 
		info("  %s", roomNames[i]);
}