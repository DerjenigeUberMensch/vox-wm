# xcb-wswm

### Filler background (dont care)
Initially I decided to delve into window managers and built upon a questionable window manager using Xlib, however the main point was for me to learn C and well I somewhat did.
After that however I decided to start a relative port of my window manager (based on dwm) and tried some different design choices instead.
Ultimately I choose this route as although dwm is "suckless" the software it uses, Xlib kinda isnt which sort of defeats the purpose.
I would like to rephrase that, dwm is suckless in its software (which is why it exists and is good) but suckless doesnt mean fast/efficient.
So thats why I decided to just port my version to xcb, the only thing that will be unlikely to be ported, atleast soon, is the text feature.
Xcb has proven to be quite the hastle with text, actually displaying text in general is quite the hastle so I decided not to continue that.


## Early Release
This build of xcb-wswm is ***not*** fully feature ready.

Currently no window movement is supported (TODO)

## Why Use this.
- **Low Memory Usage (~4-7 MiB)**
- **Low CPU usage (~.1-2%) idle**
- **Low CPU usage when doing expensive stuff (mapping windows) (~4-10%)**

## Why Not Use this.
- **Code base is, ok**
- **Few Features**
- **Bugs**
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
To **use** dwm you must first **compile it**.
Afterwards you must put `exec dwm` in your `~/.xinitrc` file. **See Below.**


## Compiling
1. Clone this repository.
```
git clone https://github.com/DerjenigeUberMensch/xcb-wswm.git
```
2. Change Directory to xcb-wswm.
```
cd xcb-wswm
```
3. Checkout the Experimental branch. (Legacy support)
```
git checkout origin/Experimental
```
4. Configure it See **Configuration** (Optional)
5. `sudo make install` to install or `make` to just compile.
6. Done.

## Configuration

****Currently Configuration is not supported in this version of xcb-wswm****

For help open an **[issue](https://github.com/DerjenigeUberMensch/xcb-wswm/issues)** about it.

## Troubleshoot
This is an **_experimental_** build and may contain bugs,
Screen tearing can be mitigated by using a compositor or by enabling it in your driver setting

If you have any **major** **issues** with the **current** commit you can **revert** to a **previous commit** for a generally stabler version.
Or you can open an **[issue](https://github.com/DerjenigeUberMensch/xcb-wswm/issues)** about it.

## Planned Features
- Panel that user can just click buttons or type values to configure windows (kinda like a dev panel).
- Fully Extend XCB-TRL to use other features.
- More modularity 
- Less Complex Code Base
- Better Documentatin (Likely when I finish implementing the usability part of the Window manager (AKA when I switch over to this.))
