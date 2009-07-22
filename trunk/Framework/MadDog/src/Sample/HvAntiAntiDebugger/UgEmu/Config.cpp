/*
This file is part of UgDbg.
 
UgDbg is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
UgDbg is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with UgDbg.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "helper.h"
#include "libconfig.h"

bool LoadConfig (char * FileName, char * Entry, config_result * result)
{
	struct config_t cfg;
	config_setting_t *setting = NULL;

	config_init(&cfg);
	if (!config_read_file(&cfg, FileName)) {
		return false;
	}

	if (result->IsInteger == true) {
		setting = config_lookup(&cfg, Entry);
		if (setting) {
			result->Value = config_setting_get_int(setting);
			config_destroy(&cfg);
			return true;
		} else return false;
	}

	if (result->IsInteger == false) {
		setting = config_lookup(&cfg, Entry);
		if (setting) {
			const char * GetString = NULL;
			GetString = config_setting_get_string(setting);
			if (GetString != NULL) {
				strcpy_s ((char *)&result->String,_countof(result->String),GetString);
				config_destroy(&cfg);
				return true;
			}
			config_destroy(&cfg);
			return false;
		} else return false;
	}

	config_destroy(&cfg);
	return false;
}

bool SaveConfig (char * FileName, char * Entry, config_result * result)
{
	struct config_t cfg;
	config_setting_t *setting = NULL;

	config_init(&cfg);
	if (!config_read_file(&cfg, FileName)) {
		return false;
	}

	if (result->IsInteger == true) {
		setting = config_setting_add (cfg.root, Entry, CONFIG_TYPE_INT);
		if (!setting) {
			setting = config_setting_get_member (cfg.root,Entry);
			if (!setting) {
				return false;
			}
		}
		config_setting_set_int(setting, result->Value);
	}

	if (strlen (result->String) != 0) {
		setting = config_setting_add (cfg.root, Entry, CONFIG_TYPE_STRING);
		if (!setting) {
			setting = config_setting_get_member (cfg.root,Entry);
			if (!setting) {
				return false;
			}
		}
		config_setting_set_string(setting, result->String);
	}

	config_write_file(&cfg, FileName);
	return true;
}

bool ReSaveConfig (char * FileName) {
	config_result result;
	memset (&result,0,sizeof (config_result));
	result.IsInteger = true;
	result.Value = GetCurrentHeight();
	if (SaveConfig (FileName,"WindowSizeY",&result) == false) return false;
	result.IsInteger = true;
	result.Value = GetCurrentWidth();
	if (SaveConfig (FileName,"WindowSizeX",&result) == false) return false;
	result.IsInteger = true;
	result.Value = GetLogYPos();

	if (SaveConfig (FileName,"LOGTEXT_Y_POS",&result) == false) return false;
	result.IsInteger = true;
	result.Value = GetDisassemblyYPos();
	if (SaveConfig (FileName,"DISASSEMBLY_Y_POS",&result) == false) return false;
/*
	result.IsInteger = false;
	strcpy_s ((char *)&result.String,_countof(result.String),"test1 \"test2\" test3");
	result.Value = false;
	if (SaveConfig (FileName,"autoexec",&result) == false) return false;
	*/
	return true;
}

bool ReLoadConfig (char * FileName)
{
	config_result result;
	memset (&result,0,sizeof (config_result));

	memset (&AutoExec,0,sizeof(AutoStruc));

	result.IsInteger = true;
	if (LoadConfig (FileName,"WindowSizeX",&result) == true) {
		SetCurrentWidth (result.Value);
	} else return false;

	memset (&result,0,sizeof (config_result));
	result.IsInteger = true;
	if (LoadConfig (FileName,"WindowSizeY",&result) == true) {
		SetCurrentHeight (result.Value);
	} else return false;

	memset (&result,0,sizeof (config_result));
	result.IsInteger = true;
	if (LoadConfig (FileName,"LOGTEXT_Y_POS",&result) == true) {
		SetLogYPos (result.Value);
	} else return false;

	memset (&result,0,sizeof (config_result));
	result.IsInteger = true;
	if (LoadConfig (FileName,"DISASSEMBLY_Y_POS",&result) == true) {
		SetDisassemblyYPos (result.Value);
	} else return false;

	memset (&result,0,sizeof (config_result));
	result.IsInteger = false;
	if (LoadConfig (FileName,"autoexec",&result) == true) {
		strcpy_s ((char *)&AutoExec.AutoExecString,_countof(AutoExec.AutoExecString),result.String);
	} else return false;

	return true;
}