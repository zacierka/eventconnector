#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "version.h"
#include "httplib.h"

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class eventconnector: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*//*, public BakkesMod::Plugin::PluginWindow*/
{
public:
	virtual void onLoad();
	virtual void onUnload();


private:
	// Registering Events
	void registerCvars();
	
	// HOOKS (EventHooks.cpp)
	void HookAllEvents();
	void HookInitTeams();
	// void HookMatchCreated();
	// void HookMatchDestroyed();
	// void HookMatchEnded();
	void HookReplayScoreDataChanged(ActorWrapper caller);

	ServerWrapper GetCurrentGameState(std::shared_ptr<GameWrapper> gameWrapper);
	bool ShouldRun(std::shared_ptr<GameWrapper> gameWrapper);

	void sendGoalEvent(int color);
	void sendMatchStartEvent(int color);
	void sendEvent(httplib::Params inParams);
	// CVARS
	std::shared_ptr<bool> enabled             = std::make_shared<bool>(false);
	// IP for NodeRed Endpoint
	std::string NODERED_URL = "192.168.1.29";
	int NODERED_PORT = 1880;


	// Game Events
	const std::string EVENT_HOOK_GOAL         = "Function TAGame.ReplayDirector_TA.OnScoreDataChanged";
	const std::string EVENT_HOOK_MATCHSTART   = "Function TAGame.Team_TA.PostBeginPlay";
	const std::string EVENT_HOOK_MATCHDESTROY = "Function TAGame.GameEvent_Soccar_TA.Destroyed";
	const std::string EVENT_HOOK_MATCHEND     = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";
};