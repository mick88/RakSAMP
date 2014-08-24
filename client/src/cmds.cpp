/*
	Updated to 0.3z by P3ti
*/

#include "main.h"

int RunCommand(char *szCMD, int iFromAutorun)
{
	// return 0: should run server sided command.
	// return 1: found local command.
	// return 2: local command not found.
	// return 3: exit process.

	if(szCMD[0] == 0x00)
		return 2;

	if(settings.iConsole)
		memset(&szCMD[(strlen(szCMD) - 2)], 0, 2);

	if(settings.runMode == RUNMODE_RCON)
	{
		if(!strncmp(szCMD, "login", 5) || !strncmp(szCMD, "LOGIN", 5))
		{
			char *pszPass = &szCMD[6];
			strcpy(settings.szRCONPass, pszPass);
			sendRconCommand(pszPass, 1);
			settings.iRCONLoggedIn = 1;
			Log("RCON password set.");
		}
		else
		{
			if(settings.iRCONLoggedIn)
				sendRconCommand(szCMD, 0);
			else
				Log("RCON password was not set. Type login [password]");
		}

		return 1;
	}

	if(szCMD[0] != '!')
	{
		// SERVER CHAT OR COMMAND
		if(szCMD[0] == '/')
			sendServerCommand(szCMD);
		else
			sendChat(szCMD);

		return 0;
	}

	szCMD++;

	// EXIT
	if(!strncmp(szCMD, "exit", 4) || !strncmp(szCMD, "EXIT", 4) ||
		!strncmp(szCMD, "quit", 4) || !strncmp(szCMD, "QUIT", 4))
	{
		sampDisconnect(0);
		ExitProcess(0);

		return 3;
	}

	// RECONNECT
	if(!strncmp(szCMD, "reconnect", 9) || !strncmp(szCMD, "RECONNECT", 9))
	{
		sampDisconnect(0);
		resetPools(1, 2000);

		return 1;
	}

	// RELOAD SETTINGS
	if(!strncmp(szCMD, "reload", 6) || !strncmp(szCMD, "RELOAD", 6))
	{
		ReloadSettings();

		return 1;
	}

	// SET RUNMODE
	if(!strncmp(szCMD, "runmode", 7) || !strncmp(szCMD, "RUNMODE", 7))
	{
		int iRunModeID = atoi(&szCMD[8]);

		if(iRunModeID > 0 && iRunModeID < 6)
		{
			settings.runMode = (eRunModes)iRunModeID;
			Log("Runmode set to %d.", settings.runMode);
		}

		return 1;
	}

	// PLAYER LIST
	if(!strncmp(szCMD, "players", 7) || !strncmp(szCMD, "PLAYERS", 7))
	{
		int iPlayerCount = 0;
		Log(" ");
		Log("============ PLAYER LIST ============");
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			if(!playerInfo[i].iIsConnected)
				continue;

			Log("(ID: %d) %s - score: %d, ping: %d", i, playerInfo[i].szPlayerName, playerInfo[i].iScore, playerInfo[i].dwPing);
			iPlayerCount++;
		}
		Log(" ");
		Log("Count: %d.", iPlayerCount);
		Log("=================================");
		Log(" ");

		return 1;
	}

	// GOTO
	if(!strncmp(szCMD, "goto", 4) || !strncmp(szCMD, "GOTO", 4))
	{
		// TELEPORT TO THE CURRENT CHECKPOINT
		if(!strncmp(szCMD, "gotocp", 6) || !strncmp(szCMD, "GOTOCP", 6))
		{
			if(settings.CurrentCheckpoint.bActive)
			{
				if(settings.runMode != RUNMODE_NORMAL)
				{
					Log("[GOTOCP] You need to be in normal runmode to teleport into the checkpoint.");
					return 1;
				}

				settings.fNormalModePos[0] = settings.CurrentCheckpoint.fPosition[0];
				settings.fNormalModePos[1] = settings.CurrentCheckpoint.fPosition[1];
				settings.fNormalModePos[2] = settings.CurrentCheckpoint.fPosition[2];

				Log("[GOTOCP] You have been teleported to the active checkpoint.");
			}
			else
				Log("[GOTOCP] There is no active checkpoint.");

			return 1;
		}

		int iPlayerID = atoi(&szCMD[5]);

		if(strlen(szCMD) == 4)
		{
			Log("[USAGE] !goto <PlayerID>");
			return 1;
		}

		if(iPlayerID < 0 || iPlayerID > MAX_PLAYERS)
			return 1;

		if(playerInfo[iPlayerID].iIsConnected)
		{
			settings.fNormalModePos[0] = playerInfo[iPlayerID].onfootData.vecPos[0];
			settings.fNormalModePos[1] = playerInfo[iPlayerID].onfootData.vecPos[1];
			settings.fNormalModePos[2] = playerInfo[iPlayerID].onfootData.vecPos[2];

			Log("[GOTO] Teleported to %s.", playerInfo[iPlayerID].szPlayerName);
		}
		else
			Log("[GOTO] Player %d is not connected.", iPlayerID);

		return 1;
	}

	// IMITATE
	if(!strncmp(szCMD, "imitate", 7) || !strncmp(szCMD, "IMITATE", 7))
	{
		char *szPlayerName = &szCMD[8];
		if(!strcmp(szPlayerName,"-1"))
		{
			imitateID = -1;
			Log("[IMITATE] Imitate was disabled.");
			return 1;
		}

		PLAYERID playerID = getPlayerIDFromPlayerName(szPlayerName);

		if(playerID < 0 || playerID > MAX_PLAYERS)
			return 1;

		if(playerInfo[playerID].iIsConnected)
		{
			imitateID = (PLAYERID)playerID;
			Log("[IMITATE] Imitate ID set to %d (%s)", imitateID, szPlayerName);
		}
		else
			Log("[IMITATE] Player %s is not connected.", szPlayerName);

		return 1;
	}

	// VEHICLE LIST
	if(!strncmp(szCMD, "vlist", 5) || !strncmp(szCMD, "VLIST", 5))
	{
		for(VEHICLEID i = 0; i < MAX_VEHICLES; i++)
		{
			if(!vehiclePool[i].iDoesExist)
				continue;

			const struct vehicle_entry *vehicle = gta_vehicle_get_by_id( vehiclePool[i].iModelID );
			if(vehicle)
				Log("[VLIST] %d (%s)", i, vehicle->name);
		}

		return 1;
	}

	// SEND VEHICLE DEATH NOTIFICATION
	if(!strncmp(szCMD, "vdeath", 6) || !strncmp(szCMD, "VDEATH", 6))
	{
		int iSelectedVeh = atoi(&szCMD[7]);
		NotifyVehicleDeath((VEHICLEID)iSelectedVeh);
		Log("[VDEATH] Sent to vehicle ID %d", iSelectedVeh);

		return 1;
	}

	// SEND LOST CONNECTION PACKET TO THE SERVER
	if(!strncmp(szCMD, "fu", 2) || !strncmp(szCMD, "fu", 2))
	{
		RakNet::BitStream bs;
		bs.Write((BYTE)ID_CONNECTION_LOST);
		pRakClient->Send(&bs, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
		return 1;
	}

	// SELECT AN ITEM FROM THE GTA MENU
	if(!strncmp(szCMD, "menusel", 7) || !strncmp(szCMD, "MENUSEL", 7))
	{
		BYTE bRow = (BYTE)atoi(&szCMD[8]);

		if(bRow != 0xFF)
		{
			RakNet::BitStream bsSend;
			bsSend.Write(bRow);
			pRakClient->RPC(&RPC_MenuSelect, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);			
		}

		return 1;
	}

	// FAKE KILL :-)
	if(!strncmp(szCMD, "kill", 4) || !strncmp(szCMD, "KILL", 4))
	{
		if(!settings.bFakeKill) {
			Log("Started flooding.. :-)");
			settings.bFakeKill = true;
		}else{
			settings.bFakeKill = false;
			Log("Stopped flooding.");
		}
		return 1;
	}

	// LAG :-)
	if(!strncmp(szCMD, "lag", 4) || !strncmp(szCMD, "LAG", 4))
	{
		if(!settings.bLag) {
			Log("Started lagging.. :-)");
			settings.bLag=true;
		}else{
			settings.bLag=false;
			Log("Stopped lagging.");
		}
		return 1;
	}

	// SPAM :-)
	if(!strncmp(szCMD, "spam", 4) || !strncmp(szCMD, "SPAM", 4))
	{
		if (settings.bSpam) {
			Log("Stopped spamming.");
			settings.bSpam = false;
		}else{
			Log("Started spamming..");
			settings.bSpam = true;
		}
		return 1;
	}

	// REQUEST CLASS
	if(!strncmp(szCMD, "class", 5) || !strncmp(szCMD, "CLASS", 5))
	{
		sampRequestClass(atoi(&szCMD[6]));
		return 1;
	}

	// SPAWNS THE FAKE PLAYER
	if(!strncmp(szCMD, "spawn", 5) || !strncmp(szCMD, "SPAWN", 5))
	{
		sampSpawn();
		iSpawned = 1;
		return 1;
	}

	// SEND WE PICKED UP A PICKUP :-)
	if(!strncmp(szCMD, "pickup", 6) || !strncmp(szCMD, "PICKUP", 6))
	{
		int iPickupID = atoi(&szCMD[7]);

		sendPickUp(iPickupID);
		Log("Picked up ID %d pickup.", iPickupID);
		return 1;
	}

	// PULSE HEALTH & ARMOR
	if(!strncmp(szCMD, "pulsehealth", 11) || !strncmp(szCMD, "PULSEHEALTH", 11))
	{
		if (settings.pulseHealth)
		{
			Log("Stopped health pulser.");
			settings.pulseHealth = false;

			settings.fPlayerHealth = settings.fHealthBeforePulse;
			settings.fPlayerArmour = settings.fArmourBeforePulse;
		}
		else
		{
			settings.fHealthBeforePulse = settings.fPlayerHealth;
			settings.fArmourBeforePulse = settings.fPlayerArmour;

			Log("Started health pulser...");
			settings.pulseHealth = true;
		}
		return 1;
	}

	// SET THE FAKE PLAYER'S CURRENT WEAPON
	if(!strncmp(szCMD, "weapon", 6) || !strncmp(szCMD, "WEAPON", 6))
	{
		settings.bCurrentWeapon = (BYTE)atoi(&szCMD[7]);
		Log("Client's current weapon set to %d.", settings.bCurrentWeapon);
		return 1;
	}

	// SET THE FOLLOWED PLAYER'S NAME
	if(!strncmp(szCMD, "selplayer", 9) || !strncmp(szCMD, "SELPLAYER", 9))
	{
		char *szPlayerName = &szCMD[10];

		sprintf_s(settings.szFollowingPlayerName, 20, szPlayerName);

		settings.runMode = RUNMODE_FOLLOWPLAYER;

		Log("[SELPLAYER] Following player changed to %s.", settings.szFollowingPlayerName);
		return 1;
	}

	// SET THE FAKE PLAYER'S VEHICLE
	if(!strncmp(szCMD, "selveh", 6) || !strncmp(szCMD, "SELVEH", 6))
	{
		int iSelectedVeh = atoi(&szCMD[7]);

		if(settings.runMode == RUNMODE_FOLLOWPLAYERSVEHICLE)
			settings.iFollowingWithVehicleID = (VEHICLEID)iSelectedVeh;

		Log("[SELVEH] Changed to vehicle ID to %d.", iSelectedVeh);

		return 1;
	}
	
	// CHANGE FOLLOWING OFFSET
	if(!strncmp(szCMD, "follow", 6) || !strncmp(szCMD, "FOLLOW", 6))
	{
		char szType[32];
		float fValue;

		if(sscanf(&szCMD[4], "%s%f", szType, &fValue) < 2)
		{
			Log("USAGE: !follow <x/y/z> <value>");
			return 1;
		}

		if(szType[0] == 'x')
			settings.fFollowXOffset = fValue;

		else if(szType[0] == 'y')
			settings.fFollowYOffset = fValue;

		else if(szType[0] == 'z')
			settings.fFollowZOffset = fValue;

		return 1;
	}

	// SEND BULLETS TO PLAYERS' POSITION :-)
	if(!strncmp(szCMD, "bulletflood", 11) || !strncmp(szCMD, "BULLETFLOOD", 11))
	{
		if (settings.bulletFlood)
		{
			Log("Stopped bullet flooding.");
			settings.bulletFlood = false;
		}
		else
		{
			Log("Started bullet flooding...");
			settings.bulletFlood = true;
		}
		return 1;
	}

	// CHANGE NAME AND REJOIN GAME :-)
	if(!strncmp(szCMD, "changename", 10) || !strncmp(szCMD, "CHANGENAME", 10))
	{
		char *szNewPlayerName = &szCMD[11];

		if(strlen(szCMD) > 11 && strcmp(g_szNickName, szNewPlayerName) != 0)
		{
			sprintf_s(g_szNickName, 32, szNewPlayerName);

			int iVersion = NETGAME_VERSION;
			unsigned int uiClientChallengeResponse = settings.uiChallange ^ iVersion;
			BYTE byteMod = 1;

			char auth_bs[4*16] = {0};
			gen_gpci(auth_bs, 0x3e9);

			BYTE byteAuthBSLen;
			byteAuthBSLen = (BYTE)strlen(auth_bs);
			BYTE byteNameLen = (BYTE)strlen(g_szNickName);
			BYTE iClientVerLen = (BYTE)strlen(settings.szClientVersion);

			RakNet::BitStream bsSend;
			bsSend.Write(iVersion);
			bsSend.Write(byteMod);
			bsSend.Write(byteNameLen);
			bsSend.Write(g_szNickName, byteNameLen);
			
			bsSend.Write(uiClientChallengeResponse);
			bsSend.Write(byteAuthBSLen);
			bsSend.Write(auth_bs, byteAuthBSLen);
			bsSend.Write(iClientVerLen);
			bsSend.Write(settings.szClientVersion, iClientVerLen);

			pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

			iAreWeConnected = 1;

			Log("Changed name to %s and rejoined to the game.", g_szNickName);
		}
		return 1;
	}

	// AUTOMATIC CHECKPOINT TELEPORTER
	if(!strncmp(szCMD, "autogotocp", 10) || !strncmp(szCMD, "AUTOGOTOCP", 10))
	{
		if (settings.AutoGotoCP)
		{
			Log("Stopped automatic checkpoint teleporter.");
			settings.AutoGotoCP = false;
		}
		else
		{
			Log("Started automatic checkpoint teleporter...");
			settings.AutoGotoCP = true;
		}
		return 1;
	}

	// CHANGE POSITION
	if(!strncmp(szCMD, "pos", 3) || !strncmp(szCMD, "POS", 3))
	{
		char szType[32];
		float fValue;

		if(sscanf(&szCMD[4], "%s%f", szType, &fValue) < 2)
		{
			Log("USAGE: !pos <x/y/z> <value>");
			return 1;
		}

		if(szType[0] == 'x')
			settings.fNormalModePos[0] = fValue;

		else if(szType[0] == 'y')
			settings.fNormalModePos[1] = fValue;

		else if(szType[0] == 'z')
			settings.fNormalModePos[2] = fValue;

		return 1;
	}

	// SEND DIALOG RESPONSE :-)
	if(!strncmp(szCMD, "dialogresponse", 14) || !strncmp(szCMD, "DIALOGRESPONSE", 14))
	{
		int szDialogID, szButtonID, szListBoxItem;
		char szInputResp[128];

		if(sscanf(&szCMD[15], "%d%d%d%s", &szDialogID, &szButtonID, &szListBoxItem, szInputResp) < 4)
		{
			Log("USAGE: !dialogresponse <Dialog ID> <Button ID> <Listbox item> <Input response>");
			return 1;
		}

		sendDialogResponse(szDialogID, szButtonID, szListBoxItem, szInputResp);

		Log("Dialog response sent.");
		return 1;
	}

	// SHOW LOG STATUS
	if(!strncmp(szCMD, "logstatus", 9) || !strncmp(szCMD, "LOGSTATUS", 9))
	{
		Log("[LOG] objects: %i, pickups: %i, textlabels: %i", settings.uiObjectsLogging, settings.uiPickupsLogging, settings.uiTextLabelsLogging);
		return 1;
	}

	// SET LOG STATUS
	if(!strncmp(szCMD, "log", 3) || !strncmp(szCMD, "LOG", 3))
	{
		char szLogType[32];
		int iToggle;

		if(sscanf(&szCMD[4], "%s%d", szLogType, &iToggle) < 2)
		{
			Log("USAGE: !log <type> <toggle (0/1)>");
			return 1;
		}

		int iLogType = 0;

		if(!strncmp(szLogType, "objects", 7) || !strncmp(szLogType, "OBJECTS", 7))
			iLogType = 1;

		if(!strncmp(szLogType, "pickups", 7) || !strncmp(szLogType, "PICKUPS", 7))
			iLogType = 2;


		if(!strncmp(szLogType, "textlabels", 10) || !strncmp(szLogType, "TEXTLABELS", 10))
			iLogType = 3;
		
		switch(iLogType)
		{
			case 1:
			{
				settings.uiObjectsLogging = iToggle;
				break;
			}

			case 2:
			{
				settings.uiPickupsLogging = iToggle;
				break;
			}

			case 3:
			{
				settings.uiTextLabelsLogging = iToggle;
				break;
			}

			default:
			{
				Log("Invalid type.");
				break;
			}
		}

		if(iLogType != 0)
			Log("[LOG] objects: %i, pickups: %i, textlabels: %i", settings.uiObjectsLogging, settings.uiPickupsLogging, settings.uiTextLabelsLogging);

		return 1;
	}

	// SHOW TELEPORT MENU
	if(!strncmp(szCMD, "teleport", 8) || !strncmp(szCMD, "TELEPORT", 8))
	{
		int iTeleportID;

		if(sscanf(&szCMD[9], "%d", &iTeleportID) < 1)
			showTeleportMenu();
		else
			useTeleport(iTeleportID);

		return 1;
	}

	// SEND SCM EVENT
	if(!strncmp(szCMD, "scmevent", 8) || !strncmp(szCMD, "SCMEVENT", 8))
	{
		int iEvent, iParam1, iParam2, iParam3;
		
		if(sscanf(&szCMD[9], "%d%d%d%d", &iEvent, &iParam1, &iParam2, &iParam3) < 4)
		{
			Log("USAGE: !scmevent <type> <param1> <param2> <param3>");
			return 1;
		}

		SendScmEvent(iEvent, iParam1, iParam2, iParam3);
		return 1;
	}

	Log("Command %s was not found.", szCMD);

	return 2;
}
