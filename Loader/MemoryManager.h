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

#endif