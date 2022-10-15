#include "MemoryManager.h"

WNDPROC ThreadSynchronizer::oldCallback;
WNDPROC ThreadSynchronizer::newCallback;
HWND ThreadSynchronizer::windowHandle;

std::vector<std::function<void()>> ThreadSynchronizer::actionQueue;

void ThreadSynchronizer::Init() {
    windowHandle = FindWindowW(NULL, L"World of Warcraft");
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