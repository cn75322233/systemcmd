#include <iostream>
#include <windows.h>
#include <shellapi.h> // 包含 ShellExecute 函数
#include <string>

// 检查当前程序是否已以管理员权限运行
bool IsElevated() {
    BOOL isElevated = FALSE;
    HANDLE tokenHandle;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
        TOKEN_ELEVATION elevation;
        DWORD dwSize;
        if (GetTokenInformation(tokenHandle, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
            isElevated = elevation.TokenIsElevated;
        }
        CloseHandle(tokenHandle);
    }
    return isElevated == TRUE;
}

// 获取可执行文件所在目录的路径
std::wstring GetExePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    wchar_t* lastBackslash = wcsrchr(path, L'\\');
    if (lastBackslash) {
        *lastBackslash = L'\0'; // 截断文件名，只保留目录路径
    }
    return std::wstring(path);
}

// 将宽字符字符串转换为 ANSI 字符串
std::string WideCharToANSI(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

int main() {
    // 检查是否已以管理员权限运行
    if (!IsElevated()) {
        std::wcout << L"程序未以管理员权限运行，正在尝试重新启动..." << std::endl;

        // 获取当前程序的路径
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);

        // 使用 ShellExecute 以管理员权限重新启动程序
        ShellExecuteW(NULL, L"runas", path, NULL, NULL, SW_NORMAL);

        return 0; // 退出当前未提升权限的程序
    }

    std::wcout << L"程序已以管理员权限运行，正在尝试以系统权限运行命令提示符..." << std::endl;

    // 获取EXE文件所在目录路径
    std::wstring exePath = GetExePath();
    // 构建PsExec工具的完整路径
    std::wstring psExecPath = exePath + L"\\Sysinternals\\PsExec.exe";

    // 调用 PsExec 工具
    std::wstring command = L"\"" + psExecPath + L"\" -i -s cmd.exe";
    std::string ansiCommand = WideCharToANSI(command);
    int result = system(ansiCommand.c_str());

    if (result == 0) {
        std::wcout << L"命令提示符已成功以系统权限运行！" << std::endl;
    }
    else {
        std::wcerr << L"运行命令提示符时出错。请确保 PsExec 工具在 Sysinternals 文件夹下。" << std::endl;
    }

    return 0;
}
