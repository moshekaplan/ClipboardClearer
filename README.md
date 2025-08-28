# Clipboard Clearer
Automatically clear clipboard after 5 minutes of inactivity, to avoid accidental pasting.

# More Details
This is a simple Windows C++ application to automatically clear the clipboard after inactivity. This code was written entirely with the following two LLM prompts:

Write a Windows C++ program that does the following:
1) Monitors for when the clipboard is modified
2) When the clipboard is modified, set a timer for 5 minutes
3) At the 5 minute mark, clear the clipboard.
4) If the clipboard is modified during those 5 minutes, reset the timer.


Modify the program so that:
1) When it launches, it displays a dialog box that the clipboard will be cleared every 5 minutes.
2) After the user pushes OK, it minimizes to being a taskbar icon
3) The taskbar icon should have a 'quit' context menu.
