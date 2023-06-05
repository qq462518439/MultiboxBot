#include "MemoryManager.h"
#include <iostream>

WNDPROC ThreadSynchronizer::oldCallback;
WNDPROC ThreadSynchronizer::newCallback;
HWND ThreadSynchronizer::windowHandle;

std::vector<std::function<void()>> ThreadSynchronizer::actionQueue;

void ThreadSynchronizer::Init() {
    windowHandle = FindWindowW(NULL, L"World of Warcraft");
    while (windowHandle == 0) {
        Sleep(250);
        windowHandle = FindWindowW(NULL, L"World of Warcraft");
    }
    std::cout << "windowHandle:" << windowHandle << "\n";
    newCallback = (WNDPROC)&WndProc;
    oldCallback = (WNDPROC)SetWindowLongW(windowHandle, GWL_WNDPROC, (LONG)newCallback);
}

void ThreadSynchronizer::RunOnMainThread(std::function<void()> action) {
    actionQueue.push_back(action);
    SendUserMessage();
}

int ThreadSynchronizer::WndProc(HWND hwnd, UINT Msg, WPARAM wparam, LPARAM lparam) {
    if (actionQueue.size() > 0) {
        std::invoke(actionQueue.back());
        actionQueue.pop_back();
    }
    return CallWindowProcW(oldCallback, hwnd, Msg, wparam, lparam);
}

void ThreadSynchronizer::SendUserMessage() {
    SendMessageW(windowHandle, WM_USER, 0, 0);
}

/*SignalEventManager::SignalEventManager() {
    InitializeSignalEventHook();
    InitializeSignalEventHookNoArgs();
}

__declspec(naked) void SignalEventManager::asm_InitializeSignalEventHookNoArgs() {
    __asm {
        push esi
        call 0x007040D0
        pushfd
        pushad
        mov edi, [edi]
        push edi
        call func_ptr
        popad
        popfd
        jmp signal_fun_ptr_no_params
    }
}

void SignalEventManager::InitializeSignalEventHookNoArgs() {
    typedef void (*SignalEventNoArgsFunc)(std::string eventName);
    SignalEventNoArgsFunc func = &SignalEventNoArgs;
    func_ptr = (void*&)func;
}*/