# Included by the configured FixBundle.cmake. ${dirs} must be expanded here:
# CMake 4 configure_file clears ${dirs} in the .in file.
fixup_bundle("${_fixup_app}" "" "${dirs}")
