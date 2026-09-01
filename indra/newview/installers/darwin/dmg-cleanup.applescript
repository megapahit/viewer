-- First, convert the disk image to "read-write" format with Disk Utility or hdiutil
-- Mount the image, open the disk image window in the Finder and make it frontmost, then run this script from inside Script Editor
-- After running the script, unmount the disk image, re-mount it, and copy the .DS_Store file off from the command line.

tell application "Finder"
	
	-- set foo to every item in front window
	-- repeat with i in foo
		-- if the name of i is "Applications" then
			-- set the position of i to {391, 165}
		-- else if the name of i ends with ".app" then
			-- set the position of i to {121, 166}
		-- end if
	-- end repeat
	
	-- There doesn't seem to be a way to set the background picture with applescript, but all the saved .DS_Store files should already have that set correctly.
	
	-- set foo to front window
	-- set current view of foo to icon view
	-- set toolbar visible of foo to false
	-- set statusbar visible of foo to false
	-- set the bounds of foo to {100, 100, 600, 449}
	
	-- set the position of front window to {100, 100}
	-- get {name, position} of every item of front window
	
	-- get properties of front window

	tell disk "Megapahit Installer"
		open
		set current view of container window to icon view
		set toolbar visible of container window to false
		set statusbar visible of container window to false
		set the bounds of container window to {400, 100, 900, 500}
		set theViewOptions to the icon view options of container window
		set arrangement of theViewOptions to not arranged
		set icon size of theViewOptions to 128
		set background picture of theViewOptions to file ".background:background.jpg"
		make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
		set position of item "Megapahit" of container window to {125, 160}
		set position of item "Applications" of container window to {375, 160}
		update without registering applications
		delay 5
		close
	end tell
end tell
