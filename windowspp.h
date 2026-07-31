#pragma once

#include <Windows.h>
#include <cassert>
#include <string>
#include <memory>
#include <utility>
#include <functional>
#include <stdexcept>

class WindowException final : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

inline static std::string GetLastErrorMessage() {
	DWORD error = GetLastError();

	if (error == 0) {
		return {};
	}

	LPSTR buffer = nullptr;

	DWORD size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		error,
		0,
		reinterpret_cast<LPSTR>(&buffer),
		0,
		nullptr
	);

	if (!size || !buffer) {
		return "Unknown error";
	}

	std::string message(buffer, size);

	LocalFree(buffer);

	return message;
}

struct WindowSize {
public:
	unsigned int m_width{};
	unsigned int m_height{};

	WindowSize() = default;

	WindowSize(unsigned int width, unsigned int height) : m_width(width), m_height(height) {}
};

struct WindowEvents {
	std::function<void()> Close;
	std::function<void(int, int)> Resize;
	std::function<void(int, int)> MouseMove;
	std::function<void(unsigned)> KeyDown;
};

class Window final {
private:	
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	WindowEvents m_events{};

	template <typename Func, typename... Args>
	void InvokeCallback(Func& func, Args&&... args) noexcept {
		if (!func) {
			return;
		}

		try {
			func(std::forward<Args>(args)...);
		}
		catch (const std::exception& error) {
			MessageBoxA(m_window, error.what(), "WindowsPP Error", MB_OK | MB_ICONERROR);
		}
		catch (...) {
			MessageBoxA(m_window, "Unknown error", "WindowsPP Error", MB_OK | MB_ICONERROR);
		}
	}

	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept {
		switch (msg) {
		case WM_SIZE: {
			int width = static_cast<int>(LOWORD(lparam));
			int height = static_cast<int>(HIWORD(lparam));

			InvokeCallback(m_events.Resize, width, height);

			return 0;
		}

		case WM_MOUSEMOVE: {
			int x = static_cast<int>(LOWORD(lparam));
			int y = static_cast<int>(HIWORD(lparam));

			InvokeCallback(m_events.MouseMove, x, y);

			return 0;
		}

		case WM_KEYDOWN: {
			InvokeCallback(m_events.KeyDown, static_cast<unsigned>(wparam));

			return 0;
		}

		case WM_PAINT: {
			InvokeCallback(m_events.Paint);
		}

		case WM_CLOSE: {
			InvokeCallback(m_events.Close);

			DestroyWindow(m_window);
			return 0;
		}

		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}

		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

private:
	HWND m_window{};
	inline static HINSTANCE m_instance{};
	inline static ATOM m_classatom{};
	inline static bool m_registered = false;

	static void RegisterWindowClass(HINSTANCE instance) {
		if (m_registered) {
			return;
		}

		WNDCLASSEXW windowclass{};	

		windowclass.cbSize = sizeof(windowclass);
		windowclass.lpfnWndProc = &Window::WindowProc;
		windowclass.lpszClassName = L"ClassWindowsPP";
		windowclass.hInstance = instance;
		windowclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowclass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

		m_classatom = RegisterClassExW(&windowclass);

		if (!m_classatom) {
			throw WindowException("RegisterWindowClass: " + GetLastErrorMessage());
		}

		m_registered = true;
	}

public:
	Window(const std::string& title, unsigned int width, unsigned int height) {
		if (title.empty()) {
			throw WindowException("Window Error: Title can't be empty");
		}

		if (width == 0) {
			throw WindowException("Window Error: Width can't be 0");
		}

		if (height == 0) {
			throw WindowException("Window Error: Height can't be 0");
		}

		HMODULE instance = GetModuleHandle(nullptr);

		if (!instance) {
			throw WindowException("Window Error: " + GetLastErrorMessage());
		}

		m_instance = instance;

		RegisterWindowClass(instance);

		std::wstring wide_title(title.begin(), title.end());

		m_window = CreateWindowExW(
			0,
			L"ClassWindowsPP",
			wide_title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width, height,
			nullptr,
			nullptr,
			instance,
			this
		);

		if (!m_window) {
			throw WindowException("Window Error: " + GetLastErrorMessage());
		}

		ShowWindow(m_window, SW_SHOW);
		UpdateWindow(m_window);
	}

	Window(const std::string& title, const WindowSize& size) : Window(title, size.m_width, size.m_height) {}

	~Window() noexcept {
		if (m_window) {
			DestroyWindow(m_window);
			m_window = nullptr;
		}
	}

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	void SetEvents(WindowEvents events) {
		m_events = std::move(events);
	}

public:
	HWND GetHandle() const noexcept {
		if (!m_window) {
			return nullptr;
		}

		return m_window;
	}

	HINSTANCE GetInstance() const noexcept {
		if (!m_instance) {
			return NULL;
		}

		return m_instance;
	}

	WindowSize GetClientSize() const noexcept {
		if (!m_window) {
			return {};
		}

		RECT rect{};
		GetClientRect(m_window, &rect);

		return {
			static_cast<unsigned>(rect.right),
			static_cast<unsigned>(rect.bottom)
		};
	}

	POINT GetPosition() const noexcept {
		if (!m_window) {
			return {};
		}

		RECT rect{};
		GetWindowRect(m_window, &rect);

		return {
			rect.left,
			rect.top
		};
	}
public:
	void Show() noexcept {
		if (!m_window) {
			return;
		}

		ShowWindow(m_window, SW_SHOW);
	}

	void Hide() noexcept {
		if (!m_window) {
			return;
		}

		ShowWindow(m_window, SW_HIDE);
	}

	void Minimize() noexcept {
		if (!m_window) {
			return;
		}

		ShowWindow(m_window, SW_MINIMIZE);
	}

	void Maximize() noexcept {
		if (!m_window) {
			return;
		}

		ShowWindow(m_window, SW_MAXIMIZE);
	}

	void Restore() noexcept {
		if (!m_window) {
			return;
		}

		ShowWindow(m_window, SW_RESTORE);
	}

public:
	bool IsMinimized() const noexcept {
		if (!m_window) {
			return;
		}

		return IsIconic(m_window);
	}

	bool IsMaximized() const noexcept {
		if (!m_window) {
			return;
		}

		return IsZoomed(m_window);
	}

	bool IsVisible() const noexcept {
		if (!m_window) {
			return;
		}

		return IsWindowVisible(m_window);
	}

	bool IsFocused() const noexcept {
		if (!m_window) {
			return;
		}
		
		return GetActiveWindow() == m_window;
	}
};

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	Window* window = nullptr;

	if (msg == WM_NCCREATE) {
		auto cs = reinterpret_cast<CREATESTRUCTW*>(lparam);

		window = static_cast<Window*>(cs->lpCreateParams);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

		window->m_window = hwnd;
	}
	else {
		window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (window) {
		return window->HandleMessage(hwnd, msg, wparam, lparam);
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

class WindowsPP final {
public:
	WindowsPP() = default;

	int Run() const {
		MSG msg{};

		while (GetMessage(&msg, nullptr, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return static_cast<int>(msg.wParam);
	}

	std::unique_ptr<Window> MakeWindow(const std::string& title, unsigned int width, unsigned int height) const {
		try {
			return std::make_unique<Window>(title, width, height);
		}
		catch (const WindowException& error) {
			MessageBoxA(nullptr, error.what(), "WindowsPP Error", MB_OK | MB_ICONERROR);
			return nullptr;
		}
	}

	std::unique_ptr<Window> MakeWindow(const std::string& title, const WindowSize& size) const {
		try {
			return std::make_unique<Window>(title, size);
		}
		catch (const WindowException& error) {
			MessageBoxA(nullptr, error.what(), "WindowsPP Error", MB_OK | MB_ICONERROR);
			return nullptr;
		}
	}
};
