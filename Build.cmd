@echo off
cd "%~dp0\src"

rd /q /s "bin"

md "bin"

gcc.exe -Oz -Wl,--gc-sections,--exclude-all-symbols,--wrap=memcpy,--wrap=memset -shared -nostdlib -static -s "Bedrock.DirectX.c" -lminhook -lkernel32 -luser32 -ld3d11 -ldxgi -o "bin\Bedrock.DirectX.dll"

gcc.exe -Oz -Wl,--gc-sections,--exclude-all-symbols,--wrap=memcpy,--wrap=memset -shared -nostdlib -static -s "Bedrock.UWP.c" -lminhook -lkernel32 -lruntimeobject -o "bin\Bedrock.UWP.dll"

gcc.exe -Oz -Wl,--gc-sections,--exclude-all-symbols -mwindows -nostdlib -static -s "Bedrock.Desktop.c" -lole32 -lkernel32 -o "bin\Bedrock.Desktop.exe"

upx --best --lzma --brute --ultra-brute "bin\*"
powershell.exe -Command "$ProgressPreference = 'SilentlyContinue'; Compress-Archive -Path 'bin\*' -DestinationPath 'bin\Bedrock.Fixes.zip' -Force"