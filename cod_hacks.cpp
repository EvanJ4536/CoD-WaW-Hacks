#include <iostream>
#include <windows.h>
#include <iomanip>
#include <time.h>
#include <thread>
#include <TlHelp32.h>

using namespace std;

HANDLE hProcess = NULL;
DWORD PID;

// Disables aggression for zombies but messes with some wall and door buys.
void disable_ai(bool* status) {
	if (!*status) {
		char Nop[] = "\x90\x90";

		WriteProcessMemory(hProcess, (void*)0x0051634C, Nop, 2, NULL);
		printf("Zombie Aggression Disabled\n");
		*status = true;
	}
	else {
		char op_code[] = "\x7F\xC0";

		WriteProcessMemory(hProcess, (void*)0x0051634C, &op_code, 2, NULL);
		printf("Zombie Aggression Enabled\n");
		*status = false;
	}
}

// Nops out instructions that change weapon Id when you switch weapons and also sets weapon id to the value in the weapon variable
void custom_weapon(bool* status, int weapon) {
	//Panzer 53
	//Wunder 62
	//Penetrator 26
	//24 Bore 38
	//Nitro 56
	//Ray Gun 60
	//grenade 15

	if (!*status) {
		*status = true;
		
		char Nop[] = "\x90\x90\x90\x90\x90\x90";

		WriteProcessMemory(hProcess, (void*)0x004106BC, Nop, 6, NULL);
		WriteProcessMemory(hProcess, (void*)0x0176C7D0, &weapon, sizeof(int), NULL);
	}
	else {
		*status = false;
		char op_code[] = "\x89\x91\xE0\x00\x00\x00";

		WriteProcessMemory(hProcess, (void*)0x004106BC, &op_code, 6, NULL);
	}
}

// Makes all guns automatic by nopping rechamber instructions
void autos_hack(HANDLE hProc, bool* status) {
	if (*status) {
		char Nop[] = "\x90\x90\x90";
		printf("All Guns Auto Enabled\n");
		WriteProcessMemory(hProc, (void*)0x0042061E, Nop, 3, NULL);
	}
	else {
		char op_code[] = "\x89\x56\x40";
		printf("All Guns Auto Disabled\n");
		WriteProcessMemory(hProc, (void*)0x0042061E, op_code, 3, NULL);
	}
}

// Nops out gravity logic so player just kind of floats around. Doing this allows fly hack to work 
void nop_gravity(HANDLE hProc, bool* status) {
	if (!*status) {
		char Nop[] = "\x90\x90\x90\x90\x90";

		WriteProcessMemory(hProc, (void*)0x0041B493, Nop, 5, NULL);
		*status = true;
	}
	else {
		char op_code[] = "\xF3\x0F\x11\x5F\x08";

		WriteProcessMemory(hProc, (void*)0x0041B493, &op_code, 5, NULL);
		*status = false;
	}
}

// Fly hack, space to go up, shift to go down. Reads player height coordinate and adds or subtracts to it based on the key presses.
// Works in conjunction with the nop_gravity function.
void fly_hack(HANDLE hProc, bool* status) {
	float up = 5;
	float player_h;
	float fly_up;
	int mError;

	*status = true;

	while (*status) {
		if (GetKeyState(VK_SPACE) & 0x8000) {
			ReadProcessMemory(hProc, (void*)0x018ED090, &player_h, sizeof(player_h), NULL);

			fly_up = player_h + up;
			if (fly_up > 300.0) {
				fly_up = 300.0;
			}

			WriteProcessMemory(hProc, (void*)0x018ED090, &fly_up, sizeof(fly_up), NULL);
		}
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			ReadProcessMemory(hProc, (void*)0x018ED090, &player_h, sizeof(player_h), NULL);

			fly_up = player_h - up;
			if (fly_up > 300.0) {
				fly_up = 300.0;
			}

			WriteProcessMemory(hProc, (void*)0x018ED090, &fly_up, sizeof(fly_up), NULL);
		}
		Sleep(10);
	}
}

// Gives player 99999 health and nops out player damage logic
void nop_health(HANDLE hProc, bool* status) {
	if (!*status) {
		int maxHealth = 99999;
		WriteProcessMemory(hProc, (void*)0x0176C8BC, &maxHealth, sizeof(int), NULL);

		char Nop[] = "\x90\x90";

		WriteProcessMemory(hProc, (void*)0x004F31F2, Nop, 2, NULL);
		printf("God Mode Enabled\n");
		*status = true;
	}
	else {
		int maxHealth = 100;
		WriteProcessMemory(hProc, (void*)0x0176C8BC, &maxHealth, sizeof(int), NULL);

		char op_code[] = "\x2B\xD3";

		WriteProcessMemory(hProc, (void*)0x004F31F2, &op_code, 2, NULL);
		printf("God Mode Disabled\n");
		*status = false;
	}
}

// Replaces ammo logic instructions with nops for unlimited ammo, if its nopped when the function is called it will restore the instructions 
void nop_ammo(HANDLE hProc, bool* status) {
	if (!*status) {
		char Nop[] = "\x90\x90\x90\x90\x90\x90\x90";
		
		WriteProcessMemory(hProc, (void*)0x0041E608, Nop, 7, NULL);
		printf("Bottomless Clip Enabled\n");
		*status = true;
	}
	else {
		char op_code[] = "\x8B\x84\x8F\xFC\x05\x00\x00";

		WriteProcessMemory(hProc, (void*)0x0041E608, &op_code, 7, NULL);
		printf("Bottomless Clip Disabled\n");
		*status = false;
	}
}

// Finds cod process and attaches
bool attach_proc(char* procName) {
	PROCESSENTRY32 procEntry32;

	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcSnap == INVALID_HANDLE_VALUE) {
		printf("Failed to take snapshot of process\n");
		return false;
	}

	while (Process32Next(hProcSnap, &procEntry32)) {
		if (!strcmp(procName, procEntry32.szExeFile)) {
			cout << "Found process " << procEntry32.szExeFile << " with process ID " << procEntry32.th32ProcessID << endl;

			hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, procEntry32.th32ProcessID);
			PID = procEntry32.th32ProcessID;

			if (hProcess == NULL) {
				printf("Failed to attach to process.\n");
			}

			CloseHandle(hProcSnap);
			return true;
		}
	}

	cout << "Couldnt find " << procName << endl;
	CloseHandle(hProcSnap);

	return false;
}

int main() {
	int choice;

	bool godmode_status = false;
	bool ammo_status = false;
	bool jump_hack_status = false;
	bool gravity_status = false;
	bool autos_status = false;
	bool custom_weapon_status = false;
	bool ai_status = false;

	//int base = 00400000;

	char jmp_hack_text[20];
	char jmp_hack_enable[20] = "Enable Fly Hack";
	char jmp_hack_disable[20] = "Disable Fly Hack";

	strcpy(jmp_hack_text, jmp_hack_enable);

	char process_name[15] = "CoDWaW.exe";
	if (!attach_proc(process_name)) {
		return 1;
	}

	while (true) {
		printf("1. God Mode\n2. Bottomless Clip\n3. %s\n4. All Guns Automatic\n5. $1000000\n6. Toggle AI\n7. Ray Gun\n8. Penetrator\n9. WunderWaffe\n0. Panzer\n\n>> ", jmp_hack_text);

		scanf("%d", &choice);


		if (choice == 1) {
			nop_health(hProcess, &godmode_status);
		}
		else if (choice == 2) {
			nop_ammo(hProcess, &ammo_status);
		}
		else if (choice == 3) {
			if (!jump_hack_status) {
				strcpy(jmp_hack_text, jmp_hack_disable);
				thread jmp_hack_thread(fly_hack, hProcess, &jump_hack_status);
				nop_gravity(hProcess, &gravity_status);
				jmp_hack_thread.detach();
			}
			else {
				jump_hack_status = false;
				nop_gravity(hProcess, &gravity_status);
				strcpy(jmp_hack_text, jmp_hack_enable);
			}
		}
		else if (choice == 4) {
			if (!autos_status) {
				autos_status = true;
				autos_hack(hProcess, &autos_status);
			}
			else {
				printf("All Guns Auto Disabled\n");
				autos_status = false;
				autos_hack(hProcess, &autos_status);
			}
		}
		else if (choice == 5) {
			int money = 1000000;
			WriteProcessMemory(hProcess, (void*)0x018EF124, &money, sizeof(int), NULL);
		}
		else if (choice == 6) {
			disable_ai(&ai_status);
		}
		else if (choice == 7) {
			custom_weapon(&custom_weapon_status, 60);
		}
		else if (choice == 8) {
			custom_weapon(&custom_weapon_status, 26);
		}
		else if (choice == 9) {
			custom_weapon(&custom_weapon_status, 62);
		}
		else if (choice == 0) {
			custom_weapon(&custom_weapon_status, 53);
		}
	}
	return 0;
}