## Third Party Viewer

As a third party maintained fork of the [Second Life][] viewer, which includes Apple Silicon native builds, Megapahit viewer is indexed in the [Third Party Viewer Directory][tpv].

## Download

Most people use a pre-built viewer release to access Second Life. FreeBSD, GNU/Linux, macOS and Windows builds are
[published on the official website][download]. More experimental viewers, such as release candidates and
project viewers, would be detailed on the same page, [in-world group][] notices, or [Discord][] server.

## Build Instructions

```
$ git clone git://megapahit.org/viewer.git
$ cd viewer
$ mkdir build-`uname -s|tr '[:upper:]' '[:lower:]'`-`uname -m`
$ cd build-`uname -s|tr '[:upper:]' '[:lower:]'`-`uname -m`
```

### Arch
```
$ sudo pacman -S cmake base-devel python apr-util boost fltk glm glu hunspell minizip nanosvg libnghttp2 openjpeg2 libpipewire sdl2 vlc libvorbis xxhash
$ export LL_BUILD="-O3 -std=c++20 -fPIC -DLL_LINUX=1"
```
build with FMOD Studio: ([register with fmod to get linux package][fmod] and then place the `.tar.gz` in `~/Downloads`)
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=OFF -DUSE_FMODSTUDIO:BOOL=ON -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
OR, use openal audio: (currently breaks CEF) 
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
```
$ make -j`nproc`
$ makepkg -R
$ sudo pacman -U megapahit-`cat newview/viewer_version.txt|sed 's/\(.*\)\./\1-/'`-`uname -m`.pkg.tar.zst
$ megapahit
```

### Debian

```
$ sudo apt install cmake pkg-config libxml2-utils libalut-dev libaprutil1-dev libboost-fiber1.81-dev libboost-json1.81-dev libboost-program-options1.81-dev libboost-regex1.81-dev libboost-url1.81-dev libexpat1-dev libfltk1.3-dev libfontconfig-dev libfreetype-dev libglu1-mesa-dev libhunspell-dev libjpeg-dev libmeshoptimizer-dev libminizip-dev libnghttp2-dev libpipewire-0.3-dev libpng-dev libsdl2-dev libvlc-dev libvlccore-dev libvorbis-dev libxft-dev libxml2-dev libxxhash-dev
$ export LL_BUILD="-O3 -std=c++20 -fPIC -DLL_LINUX=1"
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
$ make -j`nproc`
$ cpack -G DEB
$ sudo apt install ./megapahit-`cat newview/viewer_version.txt`-Linux.deb
$ megapahit
```

### Fedora

```
$ sudo dnf install cmake gcc-c++ patch patchelf rpm-build perl-FindBin apr-util-devel boost-devel boost-url expat-devel fltk-devel glm-devel mesa-libGLU-devel hunspell-devel minizip-ng-compat-devel libnghttp2-devel nanosvg-devel openjpeg-devel pipewire-devel pulseaudio-libs-devel SDL2-devel vlc-devel libvorbis-devel libXcursor-devel libXfixes-devel libXinerama-devel xxhash-devel
$ export LL_BUILD="-O3 -std=c++20 -fPIC -DLL_LINUX=1"
```
build with FMOD Studio: ([register with fmod to get linux package][fmod] and then place the `.tar.gz` in `~/Downloads`)
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=OFF -DUSE_FMODSTUDIO:BOOL=ON -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
OR, use openal audio: (currently breaks CEF) 
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
```
$ make -j`nproc`
$ cpack -G RPM
$ sudo dnf install megapahit-`cat newview/viewer_version.txt`-Linux.rpm
$ megapahit
```

### FreeBSD
(Currently does not have CEF or WebRTC Voice)
```
$ su -
# portmaster devel/cmake devel/pkgconf audio/freealut devel/apr1 devel/boost-libs x11-toolkits/fltk math/glm textproc/hunspell misc/meshoptimizer archivers/minizip graphics/nanosvg www/libnghttp2 graphics/openjpeg devel/sdl20 multimedia/vlc audio/libvorbis devel/xxhash
# exit
$ setenv LL_BUILD "-O3 -std=c++20 -fPIC"
$ setenv PYTHON /usr/local/bin/python3.11
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=OFF -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
$ make -j`nproc`
$ cd ..
$ mv -i indra/newview/app_settings/windlight ..
$ cd -
$ sudo cpack -G FREEBSD
$ sudo pkg add megapahit-`cat newview/viewer_version.txt`-`uname -s`.pkg
$ cd ..
$ mv -i ../windlight indra/newview/app_settings/
$ sudo pkg set -yA 1 freealut apr1 boost-libs fltk hunspell meshoptimizer minizip libnghttp2 openjpeg sdl20 vlc libvorbis
$ megapahit
```

### Gentoo
```
$ su -
# emerge -a eselect-repository
# eselect repository add megapahit git git://megapahit.org/ebuild.git
# emaint sync -r megapahit
# emerge -a megapahit
# exit
$ megapahit
```

### macOS

```
$ sudo port install cmake pkgconfig freealut +universal apr-util +universal boost187 +universal glm hunspell +universal freetype +universal minizip +universal openjpeg +universal libvorbis +universal xxhashlib
$ export LL_BUILD="-O3 -gdwarf-2 -stdlib=libc++ -mmacosx-version-min=11 -iwithsysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -std=c++20 -fPIC -DLL_RELEASE=1 -DLL_RELEASE_FOR_DOWNLOAD=1 -DNDEBUG -DPIC -DLL_DARWIN=1"
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=OFF -DCMAKE_INSTALL_PREFIX:PATH=newview/Megapahit.app/Contents/Resources -DCMAKE_OSX_ARCHITECTURES:STRING=`uname -m` -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=11 -DENABLE_SIGNING:BOOL=ON -DSIGNING_IDENTITY:STRING=- ../indra
$ make -j`sysctl -n hw.ncpu`
$ make install
$ open newview/Megapahit.app
```

### openSUSE Tumbleweed

```
$ sudo zypper install gcc-c++ patchelf apr-util-devel boost-devel libboost_program_options-devel libboost_url1_88_0 libboost_url1_88_0-devel libboost_context-devel libboost_fiber-devel libboost_filesystem-devel libboost_regex-devel libboost_system-devel libboost_thread-devel libpng16-devel libxml++-devel libexpat-devel fltk-devel glu-devel hunspell-devel minizip-devel nanosvg-devel libnghttp2-devel openjpeg2-devel pipewire-devel libpulse-devel libSDL2_gfx-1_0-0 libSDL2_gfx-devel sdl2-compat-devel vlc-devel libvorbis-devel xxhash-devel zlib-ng-devel libXrender-devel libXcursor-devel libXfixes-devel libXext-devel libXft-devel libXinerama-devel freetype2-devel fontconfig-devel libjpeg8-devel libjpeg8-devel freealut-devel
$ export LL_BUILD="-O3 -std=c++20 -fPIC -DLL_LINUX=1"
```
build with FMOD Studio: ([register with fmod to get linux package][fmod] and then place the `.tar.gz` in `~/Downloads`)
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=OFF -DUSE_FMODSTUDIO:BOOL=ON -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
OR, use openal audio: (currently breaks CEF) 
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
```
$ make -j`nproc`
$ cpack -G RPM
$ rpm --addsign megapahit-`cat newview/viewer_version.txt`-Linux.rpm (Set up pgp public key first)
$ sudo rpm -i megapahit-`cat newview/viewer_version.txt`-Linux.rpm
$ megapahit
```

### Ubuntu

```
$ sudo apt install cmake pkg-config libxml2-utils libaprutil1-dev libboost-fiber-dev libboost-json-dev libboost-program-options-dev libboost-regex-dev libboost-url-dev libexpat1-dev libfltk1.3-dev libfontconfig-dev libfreetype-dev libglu1-mesa-dev libhunspell-dev libjpeg-dev libmeshoptimizer-dev libminizip-dev libnanosvg-dev libnghttp2-dev libpipewire-0.3-dev libpng-dev libsdl2-dev libvlc-dev libvlccore-dev libvorbis-dev libxft-dev libxml2-dev libxxhash-dev
$ export LL_BUILD="-O3 -std=c++20 -fPIC -DLL_LINUX=1"
```
build with FMOD Studio: ([register with fmod to get linux package][fmod] and then place the `.tar.gz` in `~/Downloads`)
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=OFF -DUSE_FMODSTUDIO:BOOL=ON -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
OR, use openal audio: (currently breaks CEF)
```
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=ON -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON ../indra
```
```
$ make -j`nproc`
$ cpack -G DEB
$ sudo apt install ./megapahit-`cat newview/viewer_version.txt`-Linux.deb
$ megapahit
```

### Windows arm64
```
$ vcpkg install pkgconf python3 freealut apr-util boost cpuinfo curl freetype glm hunspell libjpeg-turbo meshoptimizer minizip nghttp2 openjpeg sse2neon libvorbis libxml2[tools] xxhash
$ vcpkg install --allow-unsupported boost-fiber
$ export LL_BUILD="/MD /O2 /Ob2 /std:c++20 /Zc:wchar_t- /Zi /GR /DLL_RELEASE=1 /DLL_RELEASE_FOR_DOWNLOAD=1 /DNDEBUG /D_SECURE_STL=0 /D_HAS_ITERATOR_DEBUGGING=0 /DWIN32 /D_WINDOWS /DLL_WINDOWS=1 /DUNICODE /D_UNICODE /DWINVER=0x0602 /D_WIN32_WINNT=0x0602 /Zc:preprocessor"
$ export PATH="$VCPKG_ROOT/downloads/tools/cmake-3.30.1-windows/cmake-3.30.1-windows-arm64/bin:$VCPKG_ROOT/downloads/tools/msys2/2db36fb050d01f45/usr/bin:$VCPKG_ROOT/installed/arm64-windows/tools/libxml2:/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin:$PATH"
$ export PKG_CONFIG_LIBDIR="$VCPKG_ROOT/installed/arm64-windows/lib/pkgconfig"
$ export PYTHON="$VCPKG_ROOT/installed/arm64-windows/tools/python3/python.exe"
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=OFF -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=OFF -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=Megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON -DCMAKE_TOOLCHAIN_FILE:FILEPATH=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVS_DISABLE_FATAL_WARNINGS:BOOL=ON ../indra
$ MSBuild.exe Megapahit.sln -p:Configuration=Release
$ cpack -G NSIS
$ start Megapahit-`cat newview/viewer_version.txt`-win64.exe
```

### Windows x64
```
$ vcpkg install pkgconf python3 freealut apr-util boost freetype glm hunspell libjpeg-turbo meshoptimizer minizip nghttp2 openjpeg libvorbis libxml2[tools] xxhash
$ export LL_BUILD="/MD /O2 /Ob2 /std:c++20 /Zc:wchar_t- /Zi /GR /DLL_RELEASE=1 /DLL_RELEASE_FOR_DOWNLOAD=1 /DNDEBUG /D_SECURE_STL=0 /D_HAS_ITERATOR_DEBUGGING=0 /DWIN32 /D_WINDOWS /DLL_WINDOWS=1 /DUNICODE /D_UNICODE /DWINVER=0x0602 /D_WIN32_WINNT=0x0602"
$ export PATH="$VCPKG_ROOT/downloads/tools/cmake-3.30.1-windows/cmake-3.30.1-windows-i386/bin:$VCPKG_ROOT/downloads/tools/msys2/2db36fb050d01f45/usr/bin:$VCPKG_ROOT/installed/x64-windows/tools/libxml2:/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin:$PATH"
$ export PKG_CONFIG_LIBDIR="$VCPKG_ROOT/installed/x64-windows/lib/pkgconfig"
$ export PYTHON="$VCPKG_ROOT/installed/x64-windows/tools/python3/python.exe"
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DADDRESS_SIZE:STRING=64 -DUSE_OPENAL:BOOL=ON -DUSE_FMODSTUDIO:BOOL=OFF -DENABLE_MEDIA_PLUGINS:BOOL=ON -DLL_TESTS:BOOL=OFF -DNDOF:BOOL=OFF -DROOT_PROJECT_NAME:STRING=Megapahit -DVIEWER_CHANNEL:STRING=Megapahit -DVIEWER_BINARY_NAME:STRING=Megapahit -DBUILD_SHARED_LIBS:BOOL=OFF -DINSTALL:BOOL=ON -DPACKAGE:BOOL=ON -DCMAKE_TOOLCHAIN_FILE:FILEPATH=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVS_DISABLE_FATAL_WARNINGS:BOOL=ON ../indra
$ MSBuild.exe Megapahit.sln -p:Configuration=Release
$ cpack -G NSIS
$ start Megapahit-`cat newview/viewer_version.txt`-win64.exe
```

## Contribute

Help make Megapahit better! You can get involved with improvements by filing bugs, suggesting enhancements, submitting
pull requests and more. See the [CONTRIBUTING][] and the [open source portal][] for details.

[Second Life]: https://secondlife.com/
[download]: https://megapahit.net
[tpv]: http://wiki.secondlife.com/wiki/Third_Party_Viewer_Directory/Megapahit
[open source portal]: http://wiki.secondlife.com/wiki/Open_Source_Portal
[contributing]: https://megapahit.org/viewer.git/tree/CONTRIBUTING.md
[in-world group]: https://world.secondlife.com/group/1142646c-5fb2-162c-ecf8-c5e422ab5c6d
[Discord]: https://discord.gg/jpt33HPVEK
[fmod]: https://www.fmod.com/studio
