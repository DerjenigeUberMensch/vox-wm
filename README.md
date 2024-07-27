## Preview (Monocle)
![example_image_monocle](https://github.com/DerjenigeUberMensch/vox-wm/blob/Experimental/images/example.png)

## Table of Contents
1. [Requirements](#Requirements)  
2. [Usage](#Usage)  
3. [Compiling](#Compiling)  
4. [Uninstalling](#Uninstalling)  
5. [How to Run/Use](#How-to-Run/Use)  
6. [Troubleshoot](#Troubleshoot)  

<a name="headers"/>

## Early Release
This build is ***not*** fully feature ready.

## Why Use this.
- **Low Memory Usage (~4-7 MiB) (When compiled in Release Mode)**
- **Low CPU usage (~.1-2%) idle**
- **Low CPU usage when doing expensive stuff (mapping windows) (~4-10%)**

## Why Not Use this.
- **Doesnt use C++**
- **XCB**

## Requirements
- **dmenu**
- **st**
- **xorg**
- **xorg-xinit**
- **libxcb**
- **xcb-proto**
- **xcb-util**
- **xcb-util-errors**
- **xcb-util-cursor**
- **xcb-util-image**
- **xcb-util-keysyms**
- **xcb-util-wm**

**Pacman && yay**
```
sudo pacman -S --needed dmenu xorg xorg-xinit libxcb xcb-util xcb-util-errors xcb-util-cursor xcb-util-image xcb-util-keysyms xcb-util-wm && yay -S st
```

## Usage 
To **use** the wm you must first **compile it**.
Afterwards you must put `exec vox-wm` in your `~/.xinitrc` file. **See Below.**


## Compiling
1. Clone this repository.
```
git clone https://github.com/DerjenigeUberMensch/vox-wm.git
```
2. Change Directory to vox-wm.
```
cd vox-wm
```
3. Checkout the Experimental branch. (Legacy support)
```
git checkout origin/Experimental
```
4. Configure it See **Configuration** (Optional)
5. Compile using Make
```
make
```
6. Install on system, install location '/usr/local/bin'
```
sudo make install
```
7. Done.

## Uninstalling
1. Change Directory to vox-wm.
```
cd vox-wm
```
2. Uninstall using Make
```
sudo make uninstall
```

## How to Run/Use
1. Compile, See Above.
2. Choose option(s) below.
3. Locate binary 
    - if installed should be **"vox-wm"**
    - else should be located under **"bin/release/"** folder as **"vox-wm"**

### Normal Usage
4. Put **"exec YOUR_BINARY_LOCATION"** in **~/.xinitrc** or similiar XServer configuration.
    - if it doesnt work make sure that no window-manager/desktop manager is run before.

### Xephyr (Debug/Testing Usage)
4. Start a Xephyr server.
    - Example: `Xephyr :1 -ac c 25 +xinerama -resizeable -screen 680x480 &`
5. After locating the binary location make sure you are in the correct display server.
    - Example: `export DISPLAY=:1`
6. After setting the correct external xserver display variable simply run the binary file.
    - Example: `./bin/release/vox-wm` or `vox-wm`

## Configuration

****Currently Configuration is not supported in this version of vox-wm****

## Troubleshoot
This is an **_experimental_** build and may contain bugs,
Screen tearing can be mitigated by using a compositor or by enabling it in your driver setting

If you have any **major** **issues** with the **current** commit you can **revert** to a **previous commit** for a generally stabler version.
Or you can open an **[issue](https://github.com/DerjenigeUberMensch/vox-wm/issues)** about it.

## Planned Features
- Panel that user can just click buttons or type values to configure windows (kinda like a dev panel).
- Fully Extend XCB-TRL to use other features.
- More modularity 
- Less Complex Code Base
- Better Documentatin (Likely when I finish implementing the usability part of the Window manager (AKA when I switch over to this.))
