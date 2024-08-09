#include "Runtime.h"
#include "MinHook.h"
#include <iostream>
#include "HavokScript.h"
#include "frida-core.h"
#pragma comment(lib, "libMinHook.x64.lib")
#pragma comment(lib, "frida-core.lib")

namespace Runtime {
	HMODULE GameCore;
	uintptr_t GameCoreAddress;
	DWORD GameProcessId;
	HANDLE GameProcess;

	static GMainLoop* loop = NULL;

	static gboolean stop(gpointer user_data)
	{
		g_main_loop_quit(loop);

		return FALSE;
	}

	static void on_detached(FridaSession* session,
			FridaSessionDetachReason reason,
			FridaCrash* crash,
			gpointer user_data)
	{
		gchar* reason_str;

		reason_str = g_enum_to_string(FRIDA_TYPE_SESSION_DETACH_REASON, reason);
		g_print("on_detached: reason=%s crash=%p\n", reason_str, crash);
		g_free(reason_str);

		g_idle_add(stop, NULL);
	}

	static void on_message(FridaScript* script,
			const gchar* message,
			GBytes* data,
			gpointer user_data)
	{
		JsonParser* parser;
		JsonObject* root;
		const gchar* type;

		parser = json_parser_new();
		json_parser_load_from_data(parser, message, -1, NULL);
		root = json_node_get_object(json_parser_get_root(parser));

		type = json_object_get_string_member(root, "type");
		if (strcmp(type, "log") == 0)
		{
			const gchar* log_message;

			log_message = json_object_get_string_member(root, "payload");
			g_print("%s\n", log_message);
		}
		else
		{
			g_print("on_message: %s\n", message);
		}

		g_object_unref(parser);
	}

	static void on_signal(int signo)
	{
		g_idle_add(stop, NULL);
	}

	void CreateFrida() {
		FridaDeviceManager* manager;
		GError* error = NULL;
		FridaDeviceList* devices;
		gint num_devices, i;
		FridaDevice* local_device;
		FridaSession* session;
		gchar* script_code;

		frida_init();

		loop = g_main_loop_new(NULL, TRUE);

		signal(SIGINT, on_signal);
		signal(SIGTERM, on_signal);

		manager = frida_device_manager_new();

		devices = frida_device_manager_enumerate_devices_sync(manager, NULL, &error);
		g_assert(error == NULL);

		local_device = NULL;
		num_devices = frida_device_list_size(devices);
		for (i = 0; i != num_devices; i++)
		{
			FridaDevice* device = frida_device_list_get(devices, i);

			g_print("[*] Found device: \"%s\"\n", frida_device_get_name(device));

			if (frida_device_get_dtype(device) == FRIDA_DEVICE_TYPE_LOCAL)
				local_device = g_object_ref(device);

			g_object_unref(device);
		}
		g_assert(local_device != NULL);

		frida_unref(devices);
		devices = NULL;

		session = frida_device_attach_sync(local_device, GameProcessId, NULL, NULL, &error);
		if (error == NULL)
		{
			FridaScript* script;
			FridaScriptOptions* options;

			g_signal_connect(session, "detached", G_CALLBACK(on_detached), NULL);
			if (frida_session_is_detached(session))
				goto session_detached_prematurely;

			g_print("[*] Attached\n");

			options = frida_script_options_new();
			frida_script_options_set_name(options, "example");
			frida_script_options_set_runtime(options, FRIDA_SCRIPT_RUNTIME_QJS);

			script_code = g_strdup_printf(
				"Interceptor.attach(ptr('0x%lx'), {\n"
				"  onEnter(args) {\n"
				"    console.log(`[*] Destroy(\"${args[1].toUInt64()}\")`);\n"
				"  }\n"
				"});\n",
				0x34d4e0 + GameCoreAddress);

			script = frida_session_create_script_sync(session, script_code, options, NULL, &error);
			g_assert(error == NULL);

			g_clear_object(&options);

			g_signal_connect(script, "message", G_CALLBACK(on_message), NULL);

			frida_script_load_sync(script, NULL, &error);
			g_assert(error == NULL);

			g_print("[*] Script loaded\n");

			if (g_main_loop_is_running(loop))
				g_main_loop_run(loop);

			g_print("[*] Stopped\n");

			frida_script_unload_sync(script, NULL, NULL);
			frida_unref(script);
			g_print("[*] Unloaded\n");

			frida_session_detach_sync(session, NULL, NULL);
		session_detached_prematurely:
			frida_unref(session);
			g_print("[*] Detached\n");
		}
		else
		{
			g_printerr("Failed to attach: %s\n", error->message);
			g_error_free(error);
		}

		frida_unref(local_device);

		frida_device_manager_close_sync(manager, NULL, NULL);
		frida_unref(manager);
		g_print("[*] Closed\n");

		g_main_loop_unref(loop);
	}

	// Should only be called once
	void Create() {
		GameCore = LoadLibrary(TEXT("../../../DLC/Expansion2/Binaries/Win64/GameCore_XP2_FinalRelease.dll"));
		if (!GameCore) {
			std::cout << "Original GameCore failed to load!\n";
			return;
		}
		GameCoreAddress = reinterpret_cast<uintptr_t>(GameCore);
		GameProcessId = GetCurrentProcessId();
		GameProcess = GetCurrentProcess();
	}

	void InitMinHook() {
		if (MH_Initialize() != MH_OK) {
			std::cout << "MH failed to init\n";
		}
		std::cout << "MH initialized successfully.\n";
	}

	// Call on dll exit
	void Destroy() {
		if (GameCore) {
			MH_DisableHook(MH_ALL_HOOKS);
			MH_Uninitialize();
			FreeLibrary(GameCore);
			GameCore = NULL;
			GameCoreAddress = 0x0;
		}
	}

	void InitConsole() {
		if (!AllocConsole()) {
			MessageBoxW(NULL, L"Failed to create the console!", L"Error", MB_ICONERROR);
			return;
		}

		// Redirect standard input/output streams to the console
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);

		std::cout << "Console initialized successfully." << std::endl;
	}

	void CloseConsole() {
		if (!FreeConsole()) {
			MessageBoxW(NULL, L"Failed to close the console!", L"Error", MB_ICONERROR);
		}
		else {
			std::cout << "Console closed successfully." << std::endl;
		}

		// Reset standard input/output streams
		freopen_s((FILE**)stdin, "NUL:", "r", stdin);
		freopen_s((FILE**)stdout, "NUL:", "w", stdout);
		freopen_s((FILE**)stderr, "NUL:", "w", stderr);

		// Clear the error state for each of the C++ standard streams after redirect.
		std::cin.clear();
		std::cout.clear();
		std::cerr.clear();
	}

	BOOL WriteCodeToProcess(uintptr_t address, byte* bytes, size_t sizeOfBytes) {
		DWORD oldProtect;
		LPVOID lpBaseAddress = reinterpret_cast<LPVOID>(address);

		if (!VirtualProtectEx(GameProcess, lpBaseAddress, sizeOfBytes, PAGE_EXECUTE_READWRITE, &oldProtect)) {
			std::cerr << "Failed to change memory protection: " << GetLastError() << std::endl;
			return FALSE;
		}

		SIZE_T numberOfBytesWritten;
		BOOL result = WriteProcessMemory(GameProcess, lpBaseAddress, bytes, sizeOfBytes, &numberOfBytesWritten);

		if (!result || numberOfBytesWritten != sizeOfBytes) {
			std::cerr << "Failed to write to memory: " << GetLastError() << std::endl;
			VirtualProtectEx(GameProcess, lpBaseAddress, sizeOfBytes, oldProtect, &oldProtect);
			return FALSE;
		}

		VirtualProtectEx(GameProcess, lpBaseAddress, sizeOfBytes, oldProtect, &oldProtect);

		return TRUE;
	}

	BOOL WriteCodeToGameCore(uintptr_t address, byte* bytes, size_t sizeOfBytes) {
		return WriteCodeToProcess(address + GameCoreAddress, bytes, sizeOfBytes);
	}
};
