/* 
Copyright (c) 2011 Vladimir Jimenez
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*** Variable Protection Details ***
Author:
Vladimir Jimenez (allejo)

Description:
The plugin will reset any variable that you choose to be locked. This is to
prevent users from changing dangerous settings and risk crashing the server
such as _gravity. This plugin is ideal for servers where anyone can use the
/set command.

To load the plugin you must load it with a conf file listing the variables
you want to be protected. The syntax of loading the plugin is as follows:

-loadplugin /path/to/varProtection.so,/path/to/lockedVars.txt

Each variable needs to be put with an '_' and on a seperate line. In order
for the plugin to work correctly, the variable must be spelled correctly
because the plugin IS case sensitve. The format for the conf file is as
follows:

_gravity
_wingsGravity
_jumpVelocity

Slash Commands:
/listupdate

License:
BSD

Version:
3.0
*/

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <string>
#include <vector>
#include <fstream>
#include <string.h>

std::vector<std::string> lockedVariables;
std::vector<std::string> backupLockedVariables;

std::string trim(std::string str, std::string what = " ")
{
	if(what.size() == 0)
		return str;
	while(str.size() >= what.size() && str.substr(0, what.size()) == what)
		str = str.substr(what.size());
	while(str.size() >= what.size() && str.substr(str.size() - what.size()) == what)
		str = str.substr(0, str.size() - what.size());
	return str;
}

class varProtectionHandler : public bz_Plugin , public bz_CustomSlashCommandHandler
{
public:
	virtual const char* Name (){return "Variable Protection";}
	virtual void Init (const char* config);
	virtual void Event(bz_EventData *eventData);
	virtual void Cleanup ();
	virtual bool SlashCommand (int playerID, bz_ApiString, bz_ApiString, bz_APIStringList*);
};

BZ_PLUGIN(varProtectionHandler);

const char* filePath;
std::string slashcommand; //String variable to store the slashcommand executed throughout the entire process
int playerID; //Int variable to store the playerID throughout the entire process
std::string variableChanged; //String variable to store the changed variable
bool change = false; //Boolean to notify the plugin of a change


void varProtectionHandler::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_eSlashCommandEvent: //Slashcommand was used
		{		
			bz_SlashCommandEventData_V1* commanddata = (bz_SlashCommandEventData_V1*)eventData; //Get the slashcommand data
			slashcommand = commanddata->message.c_str(); //Get the slashcommand used
			playerID = commanddata->from; //Int variable used for easy reference to the player that issused the command
			
			if(slashcommand.find("set")==1){
				variableChanged = slashcommand.substr(5,(slashcommand.rfind(' ')-5)); //Truncate so we get just the variable
    			for(std::vector<std::string>::iterator it = lockedVariables.begin(); it != lockedVariables.end(); it++){ //Go through the locked variables
    				if(it->compare(variableChanged.c_str())==0/* && slashcommand.find("set")==1*/){ //Check if a locked variable has been changed
    					change = true; //Notify the plugin to that a locked variable has been changed
        				break; //Get out of the for loop
        			}
        		}
			}
        	
  		}
  		break;
  				
		case bz_eTickEvent:
		{
			if(change){ //Checks if a locked variable has been changed
				change = false; //Tells plugin that it is aware of the change
				bz_resetBZDBVar(variableChanged.c_str()); //Immedidately reset the locked variable
				bz_sendTextMessagef (BZ_SERVER, playerID, "Warning: You are not allowed to change the %s setting.",variableChanged.c_str()); //Warns the user
				bz_sendTextMessagef (BZ_SERVER, eAdministrators, "*** Server Warning: Player #%d tried to change the %s setting. ***", playerID,variableChanged.c_str()); //Notifies the admins
				variableChanged = ""; //Erases the variable for the next use
			}
		}
		
		break;
		default:break;
	}
}

bool varProtectionHandler::SlashCommand( int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params )
{
	bz_BasePlayerRecord *playerdata;
    playerdata = bz_getPlayerByIndex(playerID);
    
    if(command=="listupdate" && (bz_hasPerm(playerID, "OP") || bz_hasPerm(playerID, "op"))){
    	backupLockedVariables=lockedVariables;
    	lockedVariables.clear();    	
		std::ifstream varFile;
		varFile.open(filePath);
		if(!varFile.is_open())
		{
  			bz_sendTextMessage(BZ_SERVER,playerID,"Error: Specified file does not exist.");
  			lockedVariables=backupLockedVariables;
    		varFile.close();
    		return 0;
    	}
		std::string line;
		while(getline(varFile, line))
		{
  			std::vector<std::string> params = tokenize(line, "#", 0, true);
			if(params.size() < 1)
    			continue;
    		std::string variable = trim(params.at(0));
			if(variable == "")
    			continue;
    		lockedVariables.push_back(variable.c_str());
		}
		varFile.close();
		bz_sendTextMessagef(BZ_SERVER, eAdministrators, "varProtection locked variable list updated by %s", playerdata->callsign.c_str());
		return 1;
    }
    else{
    	bz_sendTextMessage(BZ_SERVER,playerID,"You do not have the permission to update the locked variables.");
    }
    return 1;
}

void varProtectionHandler::Init(const char* commandline)
{
	Register(bz_eSlashCommandEvent);
	Register(bz_eTickEvent);
	bz_registerCustomSlashCommand ("listupdate",this);
	filePath = commandline;
	if(strlen(commandline) == 0)
	{
		bz_debugMessage(0, "Error: You must load varProtection with a list of protected variables.");
    }
	std::ifstream varFile;
	varFile.open(commandline);
	if(!varFile.is_open())
	{
  		bz_debugMessage(0, "Error: Specified file does not exist.");
    	varFile.close();
    }
	std::string line;
	while(getline(varFile, line))
	{
  		std::vector<std::string> params = tokenize(line, "#", 0, true);
		if(params.size() < 1)
    		continue;
    	std::string variable = trim(params.at(0));
		if(variable == "")
    		continue;
    	lockedVariables.push_back(variable.c_str());
	}
	varFile.close();
}

void varProtectionHandler::Cleanup(void)
{
	Flush();
	bz_removeCustomSlashCommand ("listupdate");
}

//What are you looking at this line for? Why in the world would you read the last line of code? =)


//Really? You continue reading?


//Stop! Stop reading me!


// :D -<
// :D|-<
// :D/-<
// :D\-<

//You happy? The source code danced for you.


//Really? What were you expecting?


//Do you expect more?