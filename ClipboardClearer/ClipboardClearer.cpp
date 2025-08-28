#include <windows.h>
#include <shellapi.h>

constexpr UINT    WM_TRAYICON = WM_APP + 1;
constexpr UINT_PTR TIMER_ID = 1;
constexpr DWORD    CLEAR_INTERVAL_MS = 5 * 60 * 1000;
constexpr UINT     ID_TRAY_QUIT = 1001;

// Forward declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Helper to add/remove tray icon
void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    lstrcpy(nid.szTip, TEXT("Clipboard Auto-Clear"));
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = hwnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ClearClipboardContents() {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        CloseClipboard();
    }
}

int
APIENTRY
wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow
){
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // 1) Inform the user
    MessageBox(nullptr,
        TEXT("The clipboard will be cleared 5 minutes after the last change."),
        TEXT("Clipboard Auto-Clear"),
        MB_OK | MB_ICONINFORMATION);

    // 2) Register window class
    const wchar_t CLASS_NAME[] = L"ClipboardAutoClearClass";
    WNDCLASSEX wc = { sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassEx(&wc);

    // 3) Create hidden window
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, nullptr,
        0, 0, 0, 0, 0,
        HWND_MESSAGE, nullptr,
        hInstance, nullptr
    );

    // 4) Register for clipboard updates and add tray icon
    AddClipboardFormatListener(hwnd);
    AddTrayIcon(hwnd);

    // 5) Run message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLIPBOARDUPDATE:
        // Reset 5-minute timer on every clipboard change
        KillTimer(hwnd, TIMER_ID);
        SetTimer(hwnd, TIMER_ID, CLEAR_INTERVAL_MS, nullptr);
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            KillTimer(hwnd, TIMER_ID);
            ClearClipboardContents();
        }
        return 0;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            // Show context menu with "Quit"
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            InsertMenu(hMenu, 0, MF_BYPOSITION, ID_TRAY_QUIT, TEXT("Quit"));
            SetForegroundWindow(hwnd); // Required for correct dismiss behavior
            TrackPopupMenu(hMenu,
                TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                pt.x, pt.y,
                0, hwnd, nullptr);
            DestroyMenu(hMenu);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_QUIT) {
            // Cleanup and exit
            RemoveTrayIcon(hwnd);
            RemoveClipboardFormatListener(hwnd);
            PostQuitMessage(0);
        }
        return 0;

    case WM_DESTROY:
        RemoveTrayIcon(hwnd);
        RemoveClipboardFormatListener(hwnd);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
