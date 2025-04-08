
> [!CAUTION]
> Only supported on Windows x64!

> [!IMPORTANT]
> This project modularizes [Stonecutter](https://github.com/Aetopia/Stonecutter) into modular components to be used standalone.  

# Bedrock Fixes
A package of modifications for fixing bugs in Minecraft: Bedrock Edition.

## Fixes

### `Bedrock.DirectX`

- Ability to properly disable V-Sync:

    - By default, it depends on the value of `gfx_vsync`.

    - You can force it off via `Bedrock.DirectX`'s configuration file.

- Ability to force or fallback to DirectX 11:

    - Must be toggled via `Bedrock.DirectX`'s configuration file.

### Configuration

`Bedrock.DirectX`'s file must be located in:

```
%LOCALAPPDATA%\Packages\Microsoft.MinecraftUWP_8wekyb3d8bbwe\RoamingState\Bedrock.DirectX.ini
```

```ini
[Bedrock.DirectX]
Force = 0
Fallback = 0
```

|Key|Value|
|-|-|
|Force|<ul><li>`0`: V-Sync is game controlled.</li><li>`1`: V-Sync is forced off.</li><ul>|
|Fallback|<ul><li>`0`: The game determines the graphics API.</li><li>`1`: Force the game to use DirectX 11.</li><ul>|


### `Bedrock.UWP`

- Ability to have the cursor be automatically centered when it is hidden.

### `Bedrock.Desktop`

- Ability to prevent the game from suspending, fixing any app suspension related issues.

### Configuration

- Open Windows Run & type `shell:startup`.

- Copy `Bedrock.Desktop.exe`.

Now the fix will applied on Windows Startup.


## Usage

- Download the latest version of Bedrock Fixes from [GitHub Releases](https://github.com/Aetopia/Bedrock.Fixes/releases).

- Following relevant instructions for each fix if applicable.

- Dynamic Link Library Based Fixes:

    - Can be injected using your favorite injectors & launchers.

    - Can be loaded by hijacking other dynamic link libraries.

- Executable Based Fixes:

    - Can be applied on Windows Startup.

    - Can be applied when the game is launched.

## Build
1. Install [MSYS2](https://www.msys2.org/) & [UPX](https://upx.github.io/) for optional compression.

2. Update the MSYS2 Environment until there are no pending updates using:

    ```bash
    pacman -Syu --noconfirm
    ```

3. Install GCC & [MinHook](https://github.com/TsudaKageyu/minhook) using:

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-MinHook --noconfirm
    ```

3. Make sure `<MSYS2 Installation Directory>\ucrt64\bin` is added to the Windows `PATH` environment variable.

4. Run [`Build.cmd`](src/Build.cmd).
