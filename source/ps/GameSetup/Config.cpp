/* Copyright (C) 2012 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "Config.h"

#include "ps/ConfigDB.h"
#include "ps/CConsole.h"
#include "ps/GameSetup/CmdLineArgs.h"
#include "lib/timer.h"
#include "soundmanager/SoundManager.h"

// (these variables are documented in the header.)

CStrW g_CursorName = L"test";

bool g_NoGLS3TC = false;
bool g_NoGLAutoMipmap = false;
bool g_NoGLVBO = false;

bool g_PauseOnFocusLoss = false;

bool g_Shadows = false;
bool g_ShadowPCF = false;

bool g_WaterNormal = false;
bool g_WaterRealDepth = false;
bool g_WaterFoam = false;
bool g_WaterCoastalWaves = false;
bool g_WaterRefraction = false;
bool g_WaterReflection = false;
bool g_WaterShadows = false;

bool g_Particles = false;
bool g_Silhouettes = false;
bool g_ShowSky = false;

float g_Gamma = 1.0f;

CStr g_RenderPath = "default";

int g_xres, g_yres;
bool g_VSync = false;

bool g_Quickstart = false;
bool g_DisableAudio = false;

// flag to switch on drawing terrain overlays
bool g_ShowPathfindingOverlay = false;

// flag to switch on triangulation pathfinding
bool g_TriPathfind = false;


// If non-empty, specified map will be automatically loaded
CStr g_AutostartMap = "";


//----------------------------------------------------------------------------
// config
//----------------------------------------------------------------------------

// Fill in the globals from the config files.
static void LoadGlobals()
{
	CFG_GET_VAL("vsync", Bool, g_VSync);

	CFG_GET_VAL("nos3tc", Bool, g_NoGLS3TC);
	CFG_GET_VAL("noautomipmap", Bool, g_NoGLAutoMipmap);
	CFG_GET_VAL("novbo", Bool, g_NoGLVBO);
	CFG_GET_VAL("pauseonfocusloss", Bool, g_PauseOnFocusLoss);
	CFG_GET_VAL("shadows", Bool, g_Shadows);
	CFG_GET_VAL("shadowpcf", Bool, g_ShadowPCF);

	CFG_GET_VAL("waternormals",Bool, g_WaterNormal);
	CFG_GET_VAL("waterrealdepth",Bool, g_WaterRealDepth);
	CFG_GET_VAL("waterfoam",Bool, g_WaterFoam);
	CFG_GET_VAL("watercoastalwaves",Bool, g_WaterCoastalWaves);
	if (g_WaterCoastalWaves && !g_WaterNormal)
		g_WaterCoastalWaves = false;
	CFG_GET_VAL("waterrefraction",Bool, g_WaterRefraction);
	CFG_GET_VAL("waterreflection",Bool, g_WaterReflection);
	CFG_GET_VAL("watershadows",Bool, g_WaterShadows);

	CFG_GET_VAL("renderpath", String, g_RenderPath);
	CFG_GET_VAL("particles", Bool, g_Particles);
	CFG_GET_VAL("silhouettes", Bool, g_Silhouettes);
	CFG_GET_VAL("showsky", Bool, g_ShowSky);

#if CONFIG2_AUDIO
	float gain = 0.5f;
	float musicGain = 0.5f;
	float ambientGain = 0.5f;
	float actionGain = 0.5f;
	int bufferCount = 50;
	unsigned long bufferSize = 65536;

	CFG_GET_VAL("sound.mastergain", Float, gain);
	CFG_GET_VAL("sound.musicgain", Float, musicGain);
	CFG_GET_VAL("sound.ambientgain", Float, ambientGain);
	CFG_GET_VAL("sound.actiongain", Float, actionGain);

	CFG_GET_VAL("sound.bufferCount", Int, bufferCount);
	CFG_GET_VAL("sound.bufferSize", UnsignedLong, bufferSize);

	if (g_SoundManager) {
		g_SoundManager->SetMasterGain(gain);
		g_SoundManager->SetMusicGain(musicGain);
		g_SoundManager->SetAmbientGain(ambientGain);
		g_SoundManager->SetActionGain(actionGain);

		g_SoundManager->SetMemoryUsage(bufferSize, bufferCount);
	}
#endif // CONFIG2_AUDIO
}


static void ProcessCommandLineArgs(const CmdLineArgs& args)
{
	// TODO: all these options (and the ones processed elsewhere) should
	// be documented somewhere for users.

	if (args.Has("buildarchive"))
	{
		// note: VFS init is sure to have been completed by now
		// (since CONFIG_Init reads from file); therefore,
		// it is safe to call this from here directly.
//		vfs_opt_rebuild_main_archive("mods/official/official1.zip", "../logs/trace.txt");
	}

	// Handle "-conf=key:value" (potentially multiple times)
	std::vector<CStr> conf = args.GetMultiple("conf");
	for (size_t i = 0; i < conf.size(); ++i)
	{
		CStr name_value = conf[i];
		if (name_value.Find(':') != -1)
		{
			CStr name = name_value.BeforeFirst(":");
			CStr value = name_value.AfterFirst(":");
			g_ConfigDB.CreateValue(CFG_COMMAND, name)->m_String = value;
		}
	}

	if (args.Has("g"))
	{
		g_Gamma = args.Get("g").ToFloat();
		if (g_Gamma == 0.0f)
			g_Gamma = 1.0f;
	}

//	if (args.Has("listfiles"))
//		trace_enable(true);

	if (args.Has("profile"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "profile")->m_String = args.Get("profile");

	if (args.Has("quickstart"))
	{
		g_Quickstart = true;
		g_DisableAudio = true; // do this for backward-compatibility with user expectations
	}

	if (args.Has("nosound"))
		g_DisableAudio = true;

	if (args.Has("shadows"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "shadows")->m_String = "true";

	if (args.Has("xres"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "xres")->m_String = args.Get("xres");

	if (args.Has("yres"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "yres")->m_String = args.Get("yres");

	if (args.Has("vsync"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "vsync")->m_String = "true";

	if (args.Has("ooslog"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "ooslog")->m_String = "true";

	if (args.Has("serializationtest"))
		g_ConfigDB.CreateValue(CFG_COMMAND, "serializationtest")->m_String = "true";
}


void CONFIG_Init(const CmdLineArgs& args)
{
	TIMER(L"CONFIG_Init");

	new CConfigDB;

	// Load the global, default config file
	g_ConfigDB.SetConfigFile(CFG_DEFAULT, L"config/default.cfg");
	g_ConfigDB.Reload(CFG_DEFAULT);	// 216ms
	// Try loading the local system config file (which doesn't exist by
	// default) - this is designed as a way of letting developers edit the
	// system config without accidentally committing their changes back to SVN.
	g_ConfigDB.SetConfigFile(CFG_SYSTEM, L"config/local.cfg");
	g_ConfigDB.Reload(CFG_SYSTEM);

	g_ConfigDB.SetConfigFile(CFG_USER, L"config/user.cfg");
	g_ConfigDB.Reload(CFG_USER);

	g_ConfigDB.SetConfigFile(CFG_MOD, L"config/mod.cfg");
	// No point in reloading mod.cfg here - we haven't mounted mods yet

	ProcessCommandLineArgs(args);

	// Initialise console history file
	int max_history_lines = 200;
	CFG_GET_VAL("console.history.size", Int, max_history_lines);
	g_Console->UseHistoryFile(L"config/console.txt", max_history_lines);

	// Collect information from system.cfg, the profile file,
	// and any command-line overrides to fill in the globals.
	LoadGlobals();	// 64ms
}
