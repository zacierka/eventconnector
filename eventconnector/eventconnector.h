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

	std::shared_ptr<bool> enabled;

	// Node-Red Setup
	std::string NODERED_URL = "127.0.0.1";
	std::string NODERED_ENDPOINT = "rocketleague";

	std::string HOOK_LEAVE_GAME = "Function TAGame.GameEvent_Soccar_TA.Destroyed";
	std::string HOOK_PODIUM_SHOW = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";
	std::string HOOK_START_GAME = "Function GameEvent_Soccar_TA.WaitingForPlayers.BeginState";
	std::string HOOK_BEGIN_MATCH = "Function TAGame.Team_TA.PostBeginPlay";
	//Boilerplate

private:
	virtual void onLoad();
	virtual void onUnload();
	// Registering Events
	void registerHookEvents();
	void registerCvars();
	
	// Game Events
	void onExplode();
};

