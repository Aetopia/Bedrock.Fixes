@echo off
cd "%~dp0">nul 2>&1
rmdir /Q /S "bin">nul 2>&1
mkdir "bin">nul 2>&1

gcc.exe -Os -Wl,--gc-sections -fvisibility=hidden -flto -shared -nostdlib -static -s "Bedrock.DirectX.c" -lMinHook -lKernel32 -lucrtbase -lUser32 -lD3D11 -lDXGI -o "bin\Bedrock.DirectX.dll"

gcc.exe -Os -Wl,--gc-sections -fvisibility=hidden -flto -shared -nostdlib -static -s "Bedrock.UWP.c" -lMinHook -lKernel32 -lucrtbase -lKernel32 -lRuntimeObject -o "bin\Bedrock.UWP.dll"

gcc.exe -Os -Wl,--gc-sections -fvisibility=hidden -flto -mwindows -nostdlib -static -s "Bedrock.Desktop.c" -lOle32 -lKernel32 -o "bin\Bedrock.Desktop.exe"

upx --best --lzma --brute --ultra-brute "bin\*">nul 2>&1
powershell.exe -Command "$ProgressPreference = 'SilentlyContinue'; Compress-Archive -Path 'bin\*' -DestinationPath 'bin\Bedrock.Fixes.zip' -Force">nul 2>&1
