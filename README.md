# JuicedFixes
A set of fixes for Juiced game (2005). Still a WIP, but feel free to open an issue if you find any, or a pull request if you get something done.

## Fixes  
The following had been fixed:  
  *  "Juiced requires Virtual Memory to be enabled" error  
  * Frequent crashes on calendar screen
  * Proper XInput gamepad support

## Instalation
Just drag and drop (or copy and paste) the `dinput8.dll` and `scripts` folder into Juiced main folder. You can configure the fixes by editing `fixes.ini` inside `scripts` folder. 

## XInput support
Because of how the game works, patching input required rewriting it from scratch, so be warned that keyboard **won't work** if you patch the input.
At least for now. There is a plan to add keyboard support on top of the new input.  
There is no *yet* a GUI for configuring the controls, but it is planned. To configure your gamepad edit the `fixes.ini` in `scripts` folder.

## Known issues
There are those known issues:
  * Unable to change AI companion aggression
  * Pausing the game may lead to unforceen consequences
  * Trying to bring controller settings in-game will not change the bindings  
    
Other issues might be present as well. If you happen to find one - please fire an issue on this github page.

## Future plans
A few extra features are planned:
  * Make a GUI for controller setup
  * Replace the game's built-in controller setup GUI with a new one
  * Add back keyboard settings
