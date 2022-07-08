#include "pch.h"
#include "eventconnector.h"


BAKKESMOD_PLUGIN(eventconnector, "nodered connector", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

/*
 ADD LISTENERS FOR JOIN MATCH, LEAVE MATCH, END GAME, GOALS FOR ORANGE/BLUE, Freeplay, WIN, LOSS

*/
void eventconnector::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->log("Plugin loaded!");
	enabled = std::make_shared<bool>(true);

	registerCvars();
	HookAllEvents();
}

void eventconnector::onUnload()
{
	gameWrapper->UnhookEvent(EVENT_HOOK_GOAL);
	//gameWrapper->UnhookEvent(EVENT_HOOK_MATCHSTART);

	_globalCvarManager->removeCvar("EC_Enable");
	_globalCvarManager->removeCvar("EC_SetEndpoint");

	cvarManager->log("Plugin Unloaded!");
}

void eventconnector::registerCvars()
{
	try {
		cvarManager->registerCvar("EC_Enable", "1", "Event NodeRed Connector", true, true, 0, true, 1).bindTo(enabled);

		_globalCvarManager->registerCvar("EC_SetEndpoint", "192.168.1.29", "URL for NodeRed Endpoint", true, true, 0, true, 1)
			.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			NODERED_URL = cvar.getStringValue();
		});
	}
	catch (std::exception& e)
	{
		LOG("Plugin failed to load! Please contact plugin developer");
		LOG("{}:{}", __FUNCTION__, e.what());
	}
}

void eventconnector::sendGoalEvent(int teamColor)
{
	// 0 = Blue, 1 = Orange
	std::string team = teamColor == 0 ? "blue" : "orange";
	httplib::Params params;
	params.emplace("event", "goal");
	params.emplace("team", team);

	sendEvent(params);
}

void eventconnector::sendMatchStartEvent(int teamColor)
{
	std::string team = teamColor == 0 ? "blue" : "orange";
	httplib::Params params;
	params.emplace("event", "matchstart");
	params.emplace("team", team);

	sendEvent(params);
}


void eventconnector::sendEvent(httplib::Params inParams)
{
	std::thread t([this](httplib::Params params, std::string inUrl, int port) {
		std::string url = std::string(inUrl + ":" + std::to_string(port));
		httplib::Client cli(url.c_str());

		cli.Post("/rleventconnector", params); // Ignore resulting status code
		}, inParams, NODERED_URL, NODERED_PORT);

	t.detach();
}


ServerWrapper eventconnector::GetCurrentGameState(std::shared_ptr<GameWrapper> gameWrapper)
{
	if (gameWrapper->IsInReplay())
		return gameWrapper->GetGameEventAsReplay().memory_address;
	else if (gameWrapper->IsInOnlineGame())
		return gameWrapper->GetOnlineGame();
	else
		return gameWrapper->GetGameEventAsServer();
}

bool eventconnector::ShouldRun(std::shared_ptr<GameWrapper> gameWrapper)
{
	//Check if server exists
	ServerWrapper server = GetCurrentGameState(gameWrapper);
	if (server.IsNull())
	{
		LOG("server.IsNull(): (need false) true");
		return false;
	}

	//DisAllow in replay mode
	if (gameWrapper->IsInReplay())
	{
		return false;
	}

	if (gameWrapper->IsInFreeplay())
	{
		return false;
	}

	//Check if player is spectating
	if (!gameWrapper->GetLocalCar().IsNull())
	{
		LOG("GetLocalCar().IsNull(): (need true) false");
		return false;
	}

	return true;
}


void eventconnector::HookAllEvents()
{
	using namespace std::placeholders;

	//GAME EVENTS
	// gameWrapper->HookEventPost("Function TAGame.Team_TA.PostBeginPlay", std::bind(&eventconnector::HookInitTeams, this));
	// gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&eventconnector::HookMatchDestroyed, this));
	// gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&eventconnector::HookMatchEnded, this));
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.ReplayDirector_TA.OnScoreDataChanged", std::bind(&eventconnector::HookReplayScoreDataChanged, this, _1));
}


void eventconnector::HookInitTeams()
{
	static int NumTimesCalled = 0;

	//"Function TAGame.Team_TA.PostBeginPlay" is called twice rapidly, once for each team
	// Only initialize lobby on the second hook once both teams are ready

	++NumTimesCalled;
	if (NumTimesCalled >= 2)
	{
		//Set a delay so that everything can be filled in before trying to initialize
		gameWrapper->SetTimeout([this](GameWrapper* gw)
			{
				if (ShouldRun(gameWrapper))
				{
					// Do something
				}
			}, .05f);

		NumTimesCalled = 0;
	}

	//Reset call counter after 2 seconds in case it never got through the >= 2 check
	if (NumTimesCalled != 0)
	{
		gameWrapper->SetTimeout([this](GameWrapper* gw) { NumTimesCalled = 0; }, 2.f);
	}

}

void eventconnector::HookReplayScoreDataChanged(ActorWrapper caller)
{
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (server.IsNull()) { LOG("SERVER IS BAD"); return; }

	ReplayDirectorWrapper RDW(caller.memory_address);
	if (RDW.IsNull()) { return; }
	ReplayScoreData scoreData = RDW.GetReplayScoreData();
	if (caller.IsNull()) { LOG("CALLER IS BAD"); return; }
	LOG("CALLER IS GOOD");
	
	LOG("SERVER IS GOOD");
	auto primary = server.GetLocalPrimaryPlayer();
	if (!primary) { LOG("PRI IS BAD"); return; }
	LOG("PLAYER IS GOOD");
	LOG("CALLER IS GOOD {}", caller.GetTeamNum2());
	LOG("LOCAL IS GOOD {}", primary.GetTeamNum2());
	if (scoreData.ScoreTeam == primary.GetTeamNum2())
	{
		LOG("GOAL SCORED BY PLAYER TEAM");
		sendGoalEvent(primary.GetTeamNum2());
	}
}