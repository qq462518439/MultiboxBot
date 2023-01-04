#ifndef MemoryManager_H
#define MemoryManager_H
#include <vector>
#include <functional>
#include <cstdint>
#include <windows.h>

class ThreadSynchronizer {
public:
    static void Init();
    static void RunOnMainThread(std::function<void()> action);
    static int WndProc(HWND hwnd, UINT Msg, WPARAM wparam, LPARAM lparam);
    static void SendUserMessage();

    static HWND windowHandle;

private:
    static WNDPROC newCallback;
    static WNDPROC oldCallback;
    static std::vector<std::function<void()>> actionQueue;
};

/*class SignalEventManager {
public:
    static void SignalEvent(std::string eventName, std::string format, uintptr_t firstArgPtr);
    static void SignalEventNoArgs(std::string eventName);

private:
    SignalEventManager();
    static void InitializeSignalEventHookNoArgs();
    static void asm_InitializeSignalEventHookNoArgs();
    static void InitializeSignalEventHook();
    static void asm_InitializeSignalEventHook();

    static void* func_ptr;

    const static uintptr_t SIGNAL_EVENT_FUN_PTR = 0x00703F76;
    const static uintptr_t SIGNAL_EVENT_NO_PARAMS_FUN_PTR = 0x00703E72;

    const static uintptr_t signal_fun_ptr_no_params = (SIGNAL_EVENT_NO_PARAMS_FUN_PTR + 6);
};*/

#endif