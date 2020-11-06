#include "pch.h"
#include "CSource2Client.h"

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

inline void* GetInterface(const char* dllname, const char* interfacename) {
	CreateInterfaceFn CreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(GetModuleHandle(dllname), "CreateInterface"));

	int returnCode = 0;
	void* interface = CreateInterface(interfacename, &returnCode);

	return interface;
}

DWORD WINAPI Thread(HMODULE hModule)
{
	CSource2Client* client = reinterpret_cast<CSource2Client*>(GetInterface("client.dll", "Source2Client002"));

	FILE* stream;
	errno_t err = fopen_s(&stream, "all_classes.txt", "w+");

	if (err == 0)
	{
		for (ClientClass* classes = client->GetAllClasses(); classes; classes = classes->m_pNext) {
			if (!classes->recvTable || !classes->recvTable->netVarsArray || !classes->m_pClassName)
				continue;

			fprintf(stream, "%s - NumOfVars: %d; Class Name; Variable Type; Offset (Hex); Offset (Decimal)\n", classes->m_pClassName, classes->recvTable->numOfVars);
			for (int i = 0; i < classes->recvTable->numOfVars; i++) {
				Netvar* var = classes->recvTable->netVarsArray[i].netVar;
				if (!var
					|| !var->netvarName
					|| !var->typeName)
					break;

				fprintf(stream, "\t[%d]; %s; %s; 0x%x; %d\n", i + 1, var->netvarName, var->typeName, var->offset, var->offset);
			}
		}

		err = fclose(stream);
	}

    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved) {

    if (reason == DLL_PROCESS_ATTACH)
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread, hModule, 0, 0));
    return TRUE;
}