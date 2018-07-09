#include <windows.h> //win32-API
#include <tlhelp32.h> //fot find of processes
#include <stdio.h>  
#include <io.h> //Для проверки на наличие dll

DWORD GetProcessByName(char* process_name)
{
	//Берем снимок всех процессов
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//Хранит информацию о процессе
	PROCESSENTRY32 process;
	DWORD proc_id = 0;

	//Если первый процесс существует
	if(Process32First(snapshot, &process))
	{
		//Проходим по процессам в снимке
		while (Process32Next(snapshot, &process))
		{
			//Сравниваем длину и идентичность имен процессов
			if (_stricmp(process.szExeFile, process_name) == 0)
			{
				//Запоминаем
				proc_id = process.th32ProcessID;
				break;
			}
		}
	}
	CloseHandle(snapshot);
	return proc_id;
}


bool FileExist(char* name)
{
	//Проверяем существование файла dll
	return _access(name, 0) != -1;
}

bool Inject(DWORD pID, char*path)
{
	HANDLE proc_handle; //Дескриптор процеса
	LPVOID RemoteString; //Адрес процесса
	LPCVOID LoadLibAddy; //Адрес библиотеки

	//Если процесс усели закрыть выходим
	if (pID == 0)
		return false;

	//Плучаем дескриптор процесса
	proc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, pID);

	//Если нет доступа к процессу
	if (proc_handle == 0)
		return false;

	//Получаем указатель на функцию загрузки dll
	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

	//Выделяем память под dll и запоминаем адрес
	RemoteString = VirtualAllocEx(proc_handle, NULL, strlen(path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	//Записываем код dll в память
	WriteProcessMemory(proc_handle, RemoteString, path, strlen(path), NULL);

	//Запускаем поток в процессе
	CreateRemoteThread(proc_handle, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, RemoteString, NULL, NULL);

	//Удаляем дескриптор
	CloseHandle(proc_handle);

	return true;
}

int main()
{
    char process_name[32];
    char dll_name[32];
    char path[256];

	//Получаем id процесса, если процесс запущен
	printf("enter process name: ");
	scanf("%s", process_name);
	DWORD pID = GetProcessByName(process_name);

	printf("Waiting %s for start...\n", process_name);
	for (;; Sleep(50))
	{
		if (pID == 0)
			pID = GetProcessByName(process_name);
		if (pID != 0) break;
	}
	printf("%s found (pid = %X)!\n", process_name, pID);

	//enter dll's path
	while (FileExist(path) == false)
	{
		printf("Enter DLL name: ");
		scanf("%s", dll_name);
		GetFullPathName(dll_name, sizeof(path), path, NULL);
		if (FileExist(path))
		{
			printf("DLL found!\n");
			break;
		}
		else
			printf("DLL not found!\n");
	}

	//inject
	printf("Preparing DLL for injection...\n");
	if (Inject(pID, path))
	{
		printf("DLL successfully injected!\n");
		system("pause");
	}
	else
	{
		printf("CRITICAL ERROR! \nDestroying window...\n");
		Sleep(500);
	}
    return 0;
}
