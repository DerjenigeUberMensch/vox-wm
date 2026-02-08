# Debugging

In vox-wm we use several tools to debug our program, many of them include...
gdb, kcachegrind, valgrind, bear, etc...

In this guide we will discuss how we use each and everyone of these.

## Debug Builds

To produce a debug build make sure you have all required packages as seen in the README.

Compiling a debug build is as easy as.
```
make debug
```

## Environment

To setup a environment you need to use Xephyr or any related xserver sub enviroment simulator.

To install Xephyr, you may install it via your package manager, as seen below.

**Arch**
```
sudo pacman -S xorg-server-xephyr
```

To run Xephyr you may use the command(s) below.

**NOTE: This command may FAIL, if you have another display open, you may want to change the display number if this is the case.**

```
export DISPLAY=:0 && Xephyr :1 -ac c 25 +xinerama -resizeable -screen 680x480 &
```
```
export DISPLAY=:1
```

These help setup a basic Xephyr environment.

## GDB

To install gdb, you may install it via your package manager, as seen below.

**Arch**
```
sudo pacman -S gdb
```

To run gdb you may use the command below.

NOTE: Make sure you are in the root directory `vox-wm` before running this command.

```
gdb ./bin/debug/vox-wm
```

To run a program *while in* gdb you may use the command below.

Afterwards you will be prompted to use debuginfo, continue with `yes` or `y`

```
run
```

To stop a program *while in* gdb you may press CTRL+C

```
CTRL+C
```

To continue a program *while in* gdb you may use the comand below.

```
continue
```

To set a break point *while in* gdb you may use the comand below.

Make sure to replace `YOUR_BREAK_POINT` with the function name you want to break at.

```
b YOUR_BREAK_POINT
```

**To use a fancy better looking ui you may use the command below**

**NOTE: Sometimes gdb may break the ui, and you must run this again**

```
lay spl
```

To exit gdb *while in gdb* you may use the command below.

```
exit
```

## Bear

To install bear, you may install it via your package manager, as seen below.

**Arch**
```
sudo pacman -S bear
```

To use bear, you must first clean any prior compiles with the command below

NOTE: Make sure you are in the root directory `vox-wm` before running this command.

```
make clean
```

After cleaning the directory, you may simply run the following.

```
bear -- make debug
```

However for testing release builds you may want to run.

```
bear -- make release
```

Bear is a important debugging tool, which helps linters like clang-tidy clangd and other linters identify code.

Back sometime before this was not required but due to some changes that we still dont know we made, we are now required

to have this inorder to run debugging instruments while coding.


## Valgrind

To install valgrind, you may install it via your package manager, as seen below.

**Arch**

```
sudo pacman -S valgrind
```

To use valgrind you simply run valgrind as follows, it is recomended to alias this shortcut, but not required.

```
valgrind -s --quiet --track-fds=yes --error-limit=no --vgdb=full --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --verbose --log-file="valgrind.txt" ./bin/debug/vox-wm
```

The valgrind shortcut is below.

```
alias valcheck=valgrind -s --quiet --track-fds=yes --error-limit=no --vgdb=full --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --verbose --log-file="valgrind.txt"
```

and if you use that and place it in your ~/.bashrc and refresh your bash rc, using `source ~/.bashrc`

you may run it using

```
valcheck ./bin/debug/vox-wm
```

Valgrind is a useful tool to detect memory leaks and such, unforunatetly applications such as gtk, SDL and many others believe that allowing memory to be freed at the end by the OS is a good idea, and thus using valgrind may have mixed results.

## Kcachegrind

To install kcachegrind, you may install it via your package manager, as seen below.

**NOTE: You must also have `valgrind` installed inorder to use kcachegrind, and related see [Valgrind](#valgrind) for more details.**

**Arch**
```
sudo pacman -S kcachegrind
```

To use kcachegrind you must first use callgrind inorder to see data.

```
valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./bin/debug/vox-wm
```

You may alias it as seen below

```
alias exerec="valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes $EXE"
```

To use kcachegrind to view the data.

You may use the command below and or use a gui to find it.

```
kcachegrind callgrind.out.*
```

Kcachegrind is a excellent tool that can help spot bottlenecks in performance.
