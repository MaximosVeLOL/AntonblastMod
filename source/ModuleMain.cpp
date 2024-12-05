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

void FrameCallback(FWFrame& Context) {
	UNREFERENCED_PARAMETER(Context);


	//Player Noclip
	static CInstance* ob_player = nullptr;
	static RValue curX = 0;
	static RValue curY = 0;
	if (AurieSuccess(g_int->GetInstanceObject(g_int->CallBuiltin("asset_get_index", {"ob_player"}), ob_player))) { //Change it from 52(?), to the object index that its in. This fixes the issues with the Release of Antonblast
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
		if (targetRoom.AsString() == "") {
			targetRoom = "rm_mainMenu";
		}

		g_int->CallBuiltin(
			"room_goto",
			{ g_int->CallBuiltin(
				"asset_get_index",
				{ targetRoom }
		)}
		);
	}
	// Spawning an object
	if (GetAsyncKeyState(VK_MENU)) {
		RValue targetObject = g_int->CallBuiltin("get_string", { "What is the name of the object you want to spawn?", "" });
		if (targetObject.AsString() == "") {
			targetObject = "noone";
		}
		if (g_int->CallBuiltin("instance_exists", { ob_player }).AsBool()) {
			g_int->CallBuiltin(
				"instance_create_depth",
				{ curX, curY, 0,
					g_int->CallBuiltin(
					"asset_get_index",
					{ targetObject }
			) }
			);
		}
		else 		g_int->CallBuiltin(
			"instance_create_depth",
			{ 0,0, 0,
				g_int->CallBuiltin(
				"asset_get_index",
				{ targetObject }
		) }
		);
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

	g_int->PrintWarning("Antonblast Mod Created!");
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