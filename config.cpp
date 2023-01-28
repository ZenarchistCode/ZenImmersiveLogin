class CfgPatches
{
	class ZenImmersiveLogin
	{
		requiredVersion = 0.1;
		units[] =
		{
			""
		};
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts"
		};
	};
};

class CfgMods
{
	class ZenImmersiveLogin
	{
		dir = "ZenImmersiveLogin";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "ZenImmersiveLogin";
		credits = "";
		author = "Zenarchist";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = { "Game","World","Mission" };
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = { "ZenImmersiveLogin/scripts/3_game" };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "ZenImmersiveLogin/scripts/4_world" };
			};
			class missionScriptModule
			{
				value = "";
				files[] = { "ZenImmersiveLogin/scripts/5_mission" };
			};
		};
	};
};