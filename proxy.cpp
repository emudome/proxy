//https://msdn.microsoft.com/en-us/library/windows/desktop/aa385145(v=vs.85).aspx

#pragma comment(lib, "wininet.lib")
#include <Windows.h>
#include <Wininet.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

constexpr wchar_t const* OPTION_DIRECT = L"/direct";
constexpr wchar_t const* OPTION_PROXY = L"/proxy";
constexpr wchar_t const* OPTION_AUTO_PROXY_URL = L"/pac";
constexpr wchar_t const* OPTION_AUTO_DETECT = L"/auto";

static const std::map<std::wstring, uint32_t> OPTION_MAP = {
	{ OPTION_DIRECT ,			PROXY_TYPE_DIRECT },
	{ OPTION_PROXY ,			PROXY_TYPE_PROXY },
	{ OPTION_AUTO_PROXY_URL ,	PROXY_TYPE_AUTO_PROXY_URL },
	{ OPTION_AUTO_DETECT ,		PROXY_TYPE_AUTO_DETECT },
};

struct ProxySetting {
	ProxySetting() : Type (0){}
	uint32_t Type;
	std::wstring ProxyServer;
	std::wstring ProxyBypass;
	std::wstring AutoProxyUrl;
};

bool SetProxyOptions(const ProxySetting& setting) {
	std::vector<INTERNET_PER_CONN_OPTIONW> options;
	INTERNET_PER_CONN_OPTIONW connFlagOption;
	connFlagOption.dwOption = INTERNET_PER_CONN_FLAGS_UI;
	connFlagOption.Value.dwValue = setting.Type;
	options.push_back(connFlagOption);
	std::vector<wchar_t> proxyServer;

	if (setting.Type & PROXY_TYPE_PROXY) {
		INTERNET_PER_CONN_OPTIONW serverOption;
		serverOption.dwOption = INTERNET_PER_CONN_PROXY_SERVER;
		serverOption.Value.pszValue = const_cast<wchar_t*>(setting.ProxyServer.c_str());
		options.push_back(serverOption);
		INTERNET_PER_CONN_OPTIONW bypassOption;
		bypassOption.dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
		bypassOption.Value.pszValue = const_cast<wchar_t*>(setting.ProxyBypass.c_str());
		options.push_back(bypassOption);
	}
	if (setting.Type & PROXY_TYPE_AUTO_PROXY_URL) {
		INTERNET_PER_CONN_OPTIONW autoProxyOption;
		autoProxyOption.dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
		autoProxyOption.Value.pszValue = const_cast<wchar_t*>(setting.AutoProxyUrl.c_str());
		options.push_back(autoProxyOption);
	}

	INTERNET_PER_CONN_OPTION_LISTW option_list = {};
	option_list.dwSize = sizeof(option_list);
	option_list.pszConnection = nullptr;
	option_list.dwOptionCount = static_cast<DWORD>(options.size());
	option_list.dwOptionError = 0;
	option_list.pOptions = const_cast<INTERNET_PER_CONN_OPTIONW*>(options.data());

	return InternetSetOptionW(nullptr, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, sizeof(option_list)) &&
		InternetSetOptionW(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);
}

bool ShowProxySetting() {
	std::vector<INTERNET_PER_CONN_OPTIONW> options(4);
	options[0].dwOption = INTERNET_PER_CONN_FLAGS_UI;
	options[0].Value.dwValue = 0;
	options[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	options[1].Value.pszValue = nullptr;
	options[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
	options[2].Value.pszValue = nullptr;
	options[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
	options[3].Value.pszValue = nullptr;

	INTERNET_PER_CONN_OPTION_LISTW option_list = {};
	option_list.dwSize = sizeof(option_list);
	option_list.pszConnection = nullptr;
	option_list.dwOptionCount = (DWORD)options.size();
	option_list.dwOptionError = 0;
	option_list.pOptions = options.data();

	DWORD dwSize = sizeof(option_list);

	if (!InternetQueryOptionW(nullptr, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, &dwSize)) {
		printf("ERROR: Query proxy setting failed.\n");
		return false;
	}

	const uint32_t proxyType = options[0].Value.dwValue;

	printf("- [%s] Direct\n", (proxyType & PROXY_TYPE_DIRECT)? "X":" ");
	printf("- [%s] Proxy\n", (proxyType & PROXY_TYPE_PROXY) ? "X" : " ");
	if (options[1].Value.pszValue != nullptr) {
		printf("    - Server: %ls\n", options[1].Value.pszValue);
	}
	if (options[2].Value.pszValue != nullptr) {
		printf("    - Bypass: %ls\n", options[2].Value.pszValue);
	}
	printf("- [%s] Auto proxy URL\n", (proxyType & PROXY_TYPE_AUTO_PROXY_URL) ? "X" : " ");
	if (options[3].Value.pszValue != nullptr) {
		printf("    - URL: %ls\n", options[3].Value.pszValue);
	}
	printf("- [%s] Auto detect\n", (proxyType & PROXY_TYPE_AUTO_DETECT) ? "X" : " ");

	return true;
}

void ShowUsage() {
	printf("Usage\n"
		"  proxy.exe\n"
		"  - Show current proxy settings.\n"
		"  proxy.exe /proxy [address:port] [bypass]\n"
		"  - The connection uses an explicitly set proxy server.\n"
		"  proxy.exe /pac [url]\n"
		"  - The connection downloads and processes an automatic configuration script at a specified URL.\n"
		"  proxy.exe /auto\n"
		"  - The connection automatically detects settings.\n"
		"  proxy.exe /direct\n"
		"  - The connection does not use a proxy server.\n"
	    "  proxy.exe /?\n"
		"  - Show this message.\n");
	printf("\nExamples\n"
		"  proxy.exe\n"
		"  proxy.exe /proxy proxy.example.com:8080\n"
		"  proxy.exe /proxy proxy.example.com:8080 \"*.example.com;<local>\"\n"
		"  proxy.exe /proxy http=proxy.example.com:8080;https=proxy.example.com:8080;ftp=proxy.example.com:8080\n"
		"  proxy.exe /pac http://example.com/proxy.pac\n"
		"  proxy.exe /auto\n"
		"  proxy.exe /direct\n"
	);
}

bool ParseCommandLine(int argc, const wchar_t * argv[], ProxySetting& setting) {
	uint32_t currentOption = 0;
	for (int i = 1; i < argc; i++) {
		std::wstring param = std::wstring(argv[i]);
		if (param[0] == '/') {
			auto itr = OPTION_MAP.find(param);
			if (itr == OPTION_MAP.end()) {
				return false;
			}
			currentOption = itr->second;
			setting.Type |= itr->second;
		}
		else if (currentOption == PROXY_TYPE_PROXY) {
			if (setting.ProxyServer.empty()) {
				setting.ProxyServer = param;
			}
			else if (setting.ProxyBypass.empty()) {
				setting.ProxyBypass = param;
			}
			else {
				return false;
			}
		}
		else if (currentOption == PROXY_TYPE_AUTO_PROXY_URL) {
			if (setting.AutoProxyUrl.empty()) {
				setting.AutoProxyUrl = param;
			}
			else {
				return false;
			}
		}
	}

	return true;
}

int wmain(int argc, const wchar_t * argv[]) {

	if (argc == 1) {
		ShowProxySetting();
		return 0;
	}

	if (argc == 2) {
		std::wstring param = std::wstring(argv[1]);
		if (param == L"/?") {
			ShowUsage();
			return 0;
		}
	}

	ProxySetting setting;
	if (!ParseCommandLine(argc, argv, setting)){
		ShowUsage();
		return 1;
	}

	if(!SetProxyOptions(setting)) {
		ShowUsage();
		return 1;
	}

	return 0;
}

