//https://msdn.microsoft.com/en-us/library/windows/desktop/aa385145(v=vs.85).aspx

#pragma comment(lib, "wininet.lib")
#include <Windows.h>
#include <Wininet.h>
#include <stdio.h>
#include <string>

constexpr wchar_t const* OPTION_AUTO_PROXY_URL =L"/pac";
constexpr wchar_t const* OPTION_AUTO_DETECT = L"/auto";
constexpr wchar_t const* OPTION_DIRECT = L"/direct";

bool SetProxyOptions(const INTERNET_PER_CONN_OPTIONW* options, int size) {
	INTERNET_PER_CONN_OPTION_LISTW option_list = {};
	option_list.dwSize = sizeof(option_list);
	option_list.pszConnection = nullptr;
	option_list.dwOptionCount = size;
	option_list.dwOptionError = 0;
	option_list.pOptions = const_cast<INTERNET_PER_CONN_OPTIONW*>(options);

	return InternetSetOptionW(0, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, sizeof(option_list));
}

bool SetAutoProxyUrl(const wchar_t* proxy_pac_url) {
	INTERNET_PER_CONN_OPTIONW options[2] = {};
	options[0].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
	options[0].Value.pszValue = const_cast<wchar_t*>(proxy_pac_url);
	options[1].dwOption = INTERNET_PER_CONN_FLAGS;
	options[1].Value.dwValue = PROXY_TYPE_AUTO_PROXY_URL;

	return SetProxyOptions(options, _countof(options));
}

bool SetProxy(const wchar_t* proxy, const wchar_t* bypass = nullptr) {
	INTERNET_PER_CONN_OPTIONW options[3] = {};
	options[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	options[0].Value.pszValue = const_cast<wchar_t*>(proxy);
	options[1].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
	options[1].Value.pszValue = const_cast<wchar_t*>(bypass);
	options[2].dwOption = INTERNET_PER_CONN_FLAGS;
	options[2].Value.dwValue = PROXY_TYPE_PROXY;

	return SetProxyOptions(options, _countof(options));
}

bool SetProxyAutoDetect() {
	INTERNET_PER_CONN_OPTIONW options[1] = {};
	options[0].dwOption = INTERNET_PER_CONN_FLAGS;
	options[0].Value.dwValue = PROXY_TYPE_AUTO_DETECT;

	return SetProxyOptions(options, _countof(options));
}

bool SetProxyDirect() {
	INTERNET_PER_CONN_OPTIONW options[1] = {};
	options[0].dwOption = INTERNET_PER_CONN_FLAGS;
	options[0].Value.dwValue = PROXY_TYPE_DIRECT;

	return SetProxyOptions(options, _countof(options));
}

void ShowUsage() {
	printf("Usage\n"
		   " [address:port] [bypass]\n"
	       "  The connection uses an explicitly set proxy server.\n"
           " /pac [url]\n"
           "  The connection downloads and processes an automatic configuration script at a specified URL.\n"
	       " /auto\n"
	       "  The connection automatically detects settings.\n"
	       " /direct\n"
	       "  The connection does not use a proxy server.\n");
	printf("\nExample\n"
		   "  proxy.exe proxy.example.com:8080 example.com;<local>\n"
		   "  proxy.exe /pac http://example.com/proxy.pac\n"
		   "  proxy.exe /auto\n"
		   "  proxy.exe /direct\n" 
	);
}

int wmain(int argc, wchar_t * argv[]) {

	if (argc == 1) {
		ShowUsage();
		return 1;
	}

	const std::wstring option(argv[1]);

	bool result = false;
	if (option == OPTION_AUTO_PROXY_URL) {
		result = (argc > 2) ?
			SetAutoProxyUrl(argv[2]) :
			false;
	}
	else if (option == OPTION_AUTO_DETECT) {
		result = SetProxyAutoDetect();
	} 
	else if (option == OPTION_DIRECT) {
		result = SetProxyDirect();
	}else{
		result = (argc > 1) ?
			SetProxy(argv[1], argc > 2 ? argv[2] : nullptr) :
			false;
	}

	if (!result) {
		ShowUsage();
		return 1;
	}

	if (!InternetSetOption(0, INTERNET_OPTION_REFRESH, NULL, NULL)) {
		printf("INTERNET_OPTION_REFRESH failed\n");
		return 1;
	}

	return 0;
}