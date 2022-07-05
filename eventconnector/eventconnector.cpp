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
	enabled = std::make_shared<bool>(false);

	registerHookEvents();
	registerCvars();
}

void eventconnector::onUnload()
{
	gameWrapper->UnhookEvent("Function TAGame.Ball_TA.Explode");

	_globalCvarManager->removeCvar("eventconnector_enabled");
	_globalCvarManager->removeCvar("eventconnector_disabled");
	_globalCvarManager->removeCvar("eventconnector_setURL");
	_globalCvarManager->removeCvar("eventconnector_setEndpoint");

	cvarManager->log("Plugin Unloaded!");
}

void eventconnector::registerHookEvents()
{
	gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&eventconnector::onExplode, this));
}

void eventconnector::registerCvars()
{
	_globalCvarManager->registerCvar("eventconnector_enabled", "1", "Enable eventconnector", true, true, 0, true, 1);
	_globalCvarManager->registerCvar("eventconnector_disabled", "0", "Disable eventconnector");

	_globalCvarManager->registerCvar("eventconnector_setURL", "127.0.0.1", "URL for NodeRed Endpoint", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			NODERED_URL = cvar.getStringValue();
		});

	_globalCvarManager->registerCvar("eventconnector_setEndpoint", "wled", "NodeRed Endpoint", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			NODERED_ENDPOINT = cvar.getStringValue();
		});
}

void eventconnector::onExplode()
{
	// Threaded to ignore the blocking call of the Get request
	std::thread t([this]() {
		std::string url = std::string(NODERED_URL + " " + NODERED_ENDPOINT);
		httplib::Client cli(url.c_str());

		cli.Get("/goalevent"); // Ignore resulting status code
	});
	t.detach();
}
