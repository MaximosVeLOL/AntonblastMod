#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_int = nullptr;

void Clamp(int* value, int minV, int maxV) {
	if (*value < minV) {
		*value = minV;
	}
	if (*value > maxV) {
		*value = maxV;
	}
}

#define DEBUG_ENABLED false

/* Notes
	-3 is the "all" keyword in gml
	-1 is the "noone" keyword in gml
	the players state isn't an enum value, its a struct with functions inside
*/

void DEBUG_GetAllObjects() { //A debug feature that I used to trobuleshoot the "No Noclip!" issue.
	for (int i = 0; i < g_int->CallBuiltin("instance_number", { -3 }).AsReal(); i++) {
		CInstance* Character;
		g_int->GetInstanceObject( int32_t(g_int->CallBuiltin("instance_find", { -3, i }).AsReal()), Character); //Get the (i)th number of all instances in a room, and set it to the Character
		RValue Out;
		g_int->GetBuiltin("object_index", Character, NULL_INDEX, Out);
		g_int->PrintInfo(g_int->CallBuiltin("object_get_name", { Out }).AsString());
	}
}


void DEBUG_GetAllVariables(CInstance** Object) { //A Debug feature that I used to get the variables name of the players state. Big breakthrough
	for (int i = 0; i < g_int->CallBuiltin("variable_instance_get_names", { *Object }).length(); i++) {
		g_int->PrintInfo(g_int->CallBuiltin("variable_instance_get_names", { *Object }).at(i).AsString());
	}
}


void ThrowError(RValue Message) { //Made this a function just incase
	g_int->CallBuiltin("show_message", { Message });
}

void FrameCallback(FWFrame& Context) {
	UNREFERENCED_PARAMETER(Context);
	try {
		//Player Noclip
		static CInstance* ob_player = nullptr;
		static RValue curX = 0;
		static RValue curY = 0;
		//Converting RValue to int32_t so the compiler will stop yapping at me for data loss. Bro thinks that he's Protogent 💀💀💀
		if (AurieSuccess(g_int->GetInstanceObject(int32_t(g_int->CallBuiltin("instance_find", { g_int->CallBuiltin("asset_get_index", {"ob_player"}).AsReal(), 0 }).AsReal()), ob_player))) { //I was referencing the player's object index rather than the instance in the current room. This is now fixed.
			g_int->GetBuiltin("x", ob_player, NULL_INDEX, curX);
			g_int->GetBuiltin("y", ob_player, NULL_INDEX, curY);
			int moveX = GetAsyncKeyState(VK_NUMPAD4) - GetAsyncKeyState(VK_NUMPAD6);
			Clamp(&moveX, -10, 10);
			int moveY = GetAsyncKeyState(VK_NUMPAD8) - GetAsyncKeyState(VK_NUMPAD5);
			Clamp(&moveY, -10, 10);

			curX = curX.AsReal() + moveX;
			curY = curY.AsReal() + moveY;

			g_int->SetBuiltin("x", ob_player, NULL_INDEX, curX);
			g_int->SetBuiltin("y", ob_player, NULL_INDEX, curY);
		}

		// Going to a target room
		if (GetAsyncKeyState(VK_CONTROL)) {
			g_int->CallBuiltin(
				"audio_stop_all",
				{}
			);
			RValue targetRoom = g_int->CallBuiltin("get_string", { "What is the name of the target room?", "" });
			if (g_int->CallBuiltin("asset_get_index", { targetRoom }).AsReal() == -1) {
				ThrowError("ERROR_404: Cannot find room! Make sure you typing in the room correctly.");
				return;
			}

			g_int->CallBuiltin(
				"room_goto",
				{ g_int->CallBuiltin(
					"asset_get_index",
					{ targetRoom }
			) }
			);
			RValue Zero = 0; //Will throw an error about I-values when I don't use an RValue, Fugget about it!
			g_int->SetBuiltin("x", ob_player, NULL_INDEX, Zero);
			g_int->SetBuiltin("y", ob_player, NULL_INDEX, Zero);
		}
		// Spawning an object
		static RValue TargetObject = 0;
		static int isClassic = 0;
		if (GetAsyncKeyState(VK_MENU)) {

			TargetObject = g_int->CallBuiltin("get_string", { "What is the name of the object you want to spawn?", "" });
			if (g_int->CallBuiltin("asset_get_index", { TargetObject.AsString() }).AsReal() == -1) { //If we typed in something or the object doesn't exist
				ThrowError("ERROR_404: Cannot find the requested object! Did you type in the name correctly?");
				return;
			}
			TargetObject = g_int->CallBuiltin("asset_get_index", { TargetObject.AsString() });
			if (isClassic == 0) {
				if (g_int->CallBuiltin("get_string", { "Do you want to spawn the instance with mouse? (type 1 for yes, 2 for no)", "" }).AsString() == "2") {
					isClassic = 1;
				}
				else isClassic = -1;
			}
			else if (isClassic == 1) {
				if (g_int->CallBuiltin("instance_exists", { ob_player }).AsBool()) {
					g_int->CallBuiltin("instance_create_depth", { curX, curY, 0, TargetObject });
				}
			}
		}
		if (GetAsyncKeyState(VK_NUMPAD2) && TargetObject.AsReal() != -1 && isClassic == -1) {
			RValue MousePos[2];
			g_int->GetBuiltin("mouse_x", nullptr, NULL_INDEX, MousePos[0]);
			g_int->GetBuiltin("mouse_y", nullptr, NULL_INDEX, MousePos[1]);

			g_int->CallBuiltin("instance_create_depth", { MousePos[0], MousePos[1], 0, TargetObject });
		}
		if (GetAsyncKeyState(VK_NUMPAD1)) {
			ThrowError("PG 1/2, Controls:\n Use NUMPAD 8, 5, 4, & 6 to noclip move, use ALT and NUMPAD 2 to spawn instances, and use CTRL to go to rooms");
			ThrowError("PG 2/2, Credits:\n YYToolkit (Mod Library) & AurieManager (Mod Loader) is created by AurieFramework. AntonMod is created by, and only by Maximos Ve. ANTONBLAST is created by Summitsphere.");
		}
	}
	catch (std::exception error) {
		std::string exception_string = error.what(); //Why is VS2022 always yapping?
		ThrowError("ERROR_BAD: This should never, ever happen.\n ERROR_CODE: " + exception_string);
	}
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObGetInterface(
		"YYTK_Main",
		reinterpret_cast<AurieInterfaceBase*&>(g_int)
	);

	if (!AurieSuccess(last_status)) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	g_int->CreateCallback(
		Module,
		EVENT_FRAME,
		FrameCallback,
		1
	);
	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}