# Homebrew macOS build workflow

Add a Homebrew macOS build workflow for the Megapahit viewer, beside the existing MacPorts workflow. Do not replace or retarget the MacPorts paths. A MacPorts build must keep using Boost 1.88 at `/opt/local/libexec/boost/1.88` with the `-mt` library suffix.

## Context

- Repo: Megapahit, a Second Life viewer fork. Source root contains `indra/`. Official macOS instructions are in `README.md` under "### macOS".
- Those instructions are MacPorts-only:

  ```
  sudo port install cmake pkgconfig freealut apr-util boost188 glm hunspell freetype minizip nghttp2 openjpeg libvorbis xxhashlib
  ```

- This machine is Apple Silicon (arm64), Xcode 27, CMake 4.4.3 at `/opt/homebrew/bin/cmake`. MacPorts is not installed. Dependencies were installed with Homebrew instead: apr, apr-util, boost 1.92.0, cmake, freealut, freetype, glm, hunspell, libnghttp2, libvorbis, minizip, nghttp2, openjpeg, pkgconf, xxhash. `openal-soft` is not installed.
- Homebrew Boost lives at ``brew --prefix boost`` (`/opt/homebrew/opt/boost`). Headers include `boost/filesystem.hpp`. Libraries are named `libboost_filesystem.dylib`, with no `-mt` suffix. There is no `libboost_system`; Boost.System is header-only in 1.92.
- Homebrew `freealut.pc` has `Requires: openal`, and nothing provides `openal.pc` until `brew install openal-soft`.
- pkg-config `libxml-2.0` on this machine has libdir `/usr/lib` (Homebrew's macOS pkgconfig overlay). Homebrew minizip's libdir is `/opt/homebrew/Cellar/minizip/1.3.2_1/lib`.

## What was run

From a build directory (`build-darwin-arm64`), configure was:

```
export LL_BUILD="-O3 -gdwarf-2 -stdlib=libc++ -mmacosx-version-min=12 -iwithsysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -std=c++20 -fPIC -DLL_RELEASE=1 -DLL_RELEASE_FOR_DOWNLOAD=1 -DNDEBUG -DPIC -DLL_DARWIN=1"
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=OFF -DCMAKE_INSTALL_PREFIX:PATH=newview/Megapahit.app/Contents/Resources -DCMAKE_OSX_ARCHITECTURES:STRING=$(uname -m) -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=12 -DENABLE_SIGNING:BOOL=ON -DSIGNING_IDENTITY:STRING=- ../indra
```

CMake exited 1. No Makefile was generated. Two fatal errors:

1. `indra/cmake/OPENAL.cmake` calls `pkg_search_module(Openal REQUIRED freealut)`. pkg-config finds `freealut.pc`, then fails because its `Requires: openal` is missing. Error text: `None of the required 'freealut' found`.

2. `indra/cmake/LLPrimitive.cmake` builds colladaDOM (3p-colladadom 2.3-r11) during configure. The Darwin Boost flags are:

   ```cmake
   set(BOOST_CFLAGS -I${Libxml2_LIBRARY_DIRS}exec/boost/1.88/include)
   set(BOOST_LIBS -L${Minizip_LIBRARY_DIRS}exec/boost/1.88/lib)
   set(BOOST_LIBRARY_SUFFIX -mt)
   ```

   That concatenation is a MacPorts trick: libdir `/opt/local/lib` becomes `/opt/local/libexec/boost/1.88`. With Homebrew it becomes `-I/usr/libexec/boost/1.88/include`, which does not exist. colladaDOM then fails with `fatal error: 'boost/filesystem.hpp' file not found`, and `find_library(COLLADADOM_LIBRARY)` fails for `libcollada14dom23-s.lib` / `collada14dom`. `try_compile` also passes `-DBoost_SYSTEM_LIBRARY:STRING=boost_system${BOOST_LIBRARY_SUFFIX}`, which Homebrew cannot satisfy.

`indra/cmake/Boost.cmake` has the same MacPorts assumption for the rest of the viewer:

```cmake
target_include_directories(ll::boost SYSTEM INTERFACE /opt/local/libexec/boost/1.88/include)
target_link_directories(ll::boost INTERFACE /opt/local/libexec/boost/1.88/lib)
set(sfx -mt)
```

and it links `boost_system${sfx}` on `DARWIN`.

Later, when media plugins link, these post-build steps rewrite MacPorts install names and will not match Homebrew dylibs. Leave them on the MacPorts path; give Homebrew its own:

- `indra/media_plugins/libvlc/CMakeLists.txt` (`install_name_tool -change` of `/opt/local/lib` and `/opt/local/libexec/boost/1.88/lib/libboost_*-mt.dylib`)
- `indra/media_plugins/cef/CMakeLists.txt` (same list)

## Goal

Two Darwin workflows in this tree:

1. **MacPorts, unchanged.** If `/opt/local/libexec/boost/1.88` exists, keep today's flags, `-mt` suffix, `boost_system-mt`, and the existing `install_name_tool -change` lists.
2. **Homebrew**, selected when that MacPorts prefix is absent (or via an explicit cache option if that is cleaner). Use ``brew --prefix boost`` for includes and libs, an empty library suffix, and do not link `boost_system`. Document a "### macOS (Homebrew)" section in `README.md` next to the existing MacPorts section. Homebrew packages: `cmake pkgconf freealut openal-soft apr-util boost glm hunspell freetype minizip libnghttp2 openjpeg libvorbis xxhash`. The build commands stay the same apart from the package install.

Use a fresh build directory. Do not reuse a CMakeCache from the failed run; it cached `freealut` as missing and configured colladaDOM with the bad Boost include path. Delete `CMakeCache.txt`, `CMakeFiles`, `3p-colladadom-2.3-r11`, and `packages/cmake_tracking/colladadom_installed` before reconfiguring.

Done means: the Homebrew configure succeeds and produces a Makefile, then `make -j$(sysctl -n hw.ncpu)` and `make install` complete, while the MacPorts branch of the CMake logic is still intact.
