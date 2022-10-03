# QtGuiConsoleApp

**[BAD?]** Example of a single Qt Windows **GUI** exe that can also be run as a Console app.

The cpp code in this example could easily be used in a non-Qt Windows app.

Only Windows appears to be unable to have a single exe that can run as both a GUI app and a Console app.  
The problem has [probably] been around since Windows 1.0.  
No other non-Windows OS has this problem.  

## DO NOT USE?

`**YOU PROBABLY DO NOT WANT USE THIS SOLUTION**` for the biggest reason that `**while the console app is running**` the user `**can still enter console commands!**` :/

This may be acceptable for quick atomic command-line operations.  
This is unacceptable for longer/indefinite/ongoing command-line operations.

There are also reports that this may not redirect/pipe console output to a file as expected.

**This current code will launch as a Console app if launched from a GUI app that launches this app from an embedded console (think "IDE"). :/**

This code is only here so that someone can iterate on it further to improve this workaround for a 20+ year old Windows bug/limitation. :/

Perhaps ignoring all user console input except for Ctrl-Break might be a sufficient trick. :)

## Summary

This app is compiled as a **GUI** app that:
1. Attempts to detect if it can attach to an existing console.
2. If an existing console was successfully attached to then it attempts to detect if there is a prompt.
3. If a prompt is detected then it:
    1. Saves the prompt text
    2. Clears the prompt (so that its presence does not confuse the user)
    2. When the app exits it restores the saved prompt text

This is a `"GUI app that attaches to a console"` alternative to the usual `"Console app that launches a GUI"` solution.

Both have their pros and cons.

There are basically 4 choices overall:
1. Have two apps, `myapp.exe` that runs as a GUI, and `myapp.com` that runs as a Console.  
   This is what Visual Studio's `dev.exe` and `dev.com` do.  
   There are many other apps that go this route.  
   Personally, this seems like an over the top ugly hack that I would prefer to avoid completely.  
   Practically it seems fine, but I really don't want to confuse the user with two launchable binaries. :/  
   What happens when they launch the exe GUI app [most likely from auto-complete] with command-line options?  
   Does the world end? No, but it seems ultra-cheesy and confusing.
2. Have two apps, `[w]myapp.exe` that runs as a GUI, and `cmyapp.exe` that runs as a Console.  
   This is an almost acceptable variation of #1...but with effectively the same problems.
3. Have a `Console exe` that determines if it should spawn a detached GUI.  
   This has the one drawback of, **when not launched from a console** (ex: Explorer/StartMenu/shortcut/etc, which will,
   admittedly, unfortunately, be 99% of the time), there is a tiny flicker of a console app window launching. :/  
   **That's it!** That is its only drawback... and the flicker is practically unnoticable on today's faster PCs.
4. Have a `GUI exe` that determines if it was spawned from a console and should re-attach to it.  
   This sounds promising, but I have a ton a code written to prove that it just isn't practical. :/  
   All implementations I have seen and done all have the horrible side-effect of **when launching from a console**:
     1. The launching console returns back to the console prompt after launching the detached **GUI exe** (this is the only way **GUI exes** can be launched)
     2. While the detached GUI is running it outputs the command-line output in the console that is [or rather, was] sitting at the command prompt.
     3. **While the detached GUI is running the user can still `dir` or `cd` or run any other console command that they want.**
     4. When the detached GUI exits the command prompt cursor sits blinking **at the end of the last GUI app console output.**
   
   There are hacks that fake their way around problems 1, 2, and 4 to be **almost** unnoticable to the user what is going on.  
   The best implementation I found was this low voted one:  
     https://stackoverflow.com/a/59340459/252308  
   But problem 3 is huge.  
   For quick atomic operations this is tollerable.  
   For longer/indefinite/ongoing operations this is absolutely unacceptable.  
   This is the reason I gave up on using this solution and am using #3 in my app(s).

Yes, **all** of these options suck [why can't it just work like Linux?], but #3 seems to have the least drawbacks.

This repo is my [bad] attempt at #4.

## References

* The defacto expert seems to be Raymond Chen:
  https://devblogs.microsoft.com/oldnewthing/20090101-00/?p=19643
* Here is another:
  https://www.devever.net/~hl/win32con
* One of the better open source implementations out there is:
  http://bazaar.launchpad.net/~inkscape.dev/inkscape/trunk/view/head:/src/winconsole.cpp
* There are tons of hacks:
  * https://stackoverflow.com/q/493536/252308
  * https://stackoverflow.com/q/1305257/252308
  * ...
