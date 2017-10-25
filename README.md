# Proxy

Command line tool to set IE proxy.

## Usage

```
proxy.exe  # Show current proxy settings.
proxy.exe /proxy [address:port] [bypass] # The connection uses an explicitly set proxy server.
proxy.exe /pac [url] # The connection downloads and processes an automatic configuration script at a specified URL.
proxy.exe /auto # The connection automatically detects settings.
proxy.exe /direct # The connection does not use a proxy server.
proxy.exe /? # Show usage.
```

## Example

```
proxy.exe
proxy.exe /proxy proxy.example.com:8080
proxy.exe /proxy proxy.example.com:8080 "*.example.com;<local>"
proxy.exe /proxy http=proxy.example.com:8080;https=proxy.example.com:8080;ftp=proxy.example.com:8080
proxy.exe /pac http://example.com/proxy.pac
proxy.exe /auto
proxy.exe /direct
```

## Licence

[MIT](https://github.com/emudome/proxy/blob/master/LICENCE)

## Author

[emudome](https://github.com/emudome)
