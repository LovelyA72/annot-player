:: deploy-win-update.cmd
:: 8/13/2012
setlocal
cd /d d:/dev/build || exit /b 1

set VERSION=0.1.2.3
set APP=annot-player-updater
set TARGET=Annot Stream
set ZIPFILE=%APP%-%VERSION%-win.zip

set OPENSSL_HOME=/Volumes/win/dev/openssl/1.0.0j
set OPENSSL_DLLS=libeay32.dll,ssleay32.dll

set CYGWIN_HOME=
set CYGWIN_VERSION=cygwin1
set CYGWIN_EXES=lftp.exe,chmod.exe
set CYGWIN_DLLS=cyggcc_s-1.dll,cygstdc++-6.dll,cygcrypto-1.0.0.dll,cygncurses-10.dll,cygncursesw-10.dll,cygwin1.dll,cygexpat-1.dll,cygiconv-2.dll,cygintl-8.dll,cygreadline7.dll,cygssl-1.0.0.dll,cygz.dll

set BUILD=/Volumes/local/dev/annot-build-desktop/build.win
set SOURCE=/Volumes/local/dev/annot

:: deploy into app dir

rm -Rf "%TARGET%"
mkdir "%TARGET%"
cd "%TARGET%" || exit /b 1

cp -v "%BUILD%/[ Update ].exe" . || exit /b 1

cp -v "%SOURCE%/UPDATE" "Update.txt" || exit /b 1
unix2dos "Update.txt"

cp "%SOURCE%/COPYING" License.txt || exit /b 1
unix2dos License.txt

mkdir Update
cd Update || exit /b 1

cp -Rv "%SOURCE%"/src/updater/scripts/* .

mkdir %CYGWIN_VERSION% || exit /b 1
cp -v "%CYGWIN_HOME%"/bin/{%CYGWIN_EXES%,%CYGWIN_DLLS%} %CYGWIN_VERSION%/ || exit /b 1

cp -v "%BUILD%/annot-update.exe" . || exit /b 1

cd ..

:: desktop.ini
cp -v "%SOURCE%"/src/common/share/apps.ico icon.ico || exit /b 1
cp -v "%SOURCE%"/src/common/share/desktop.ini.txt desktop.ini || exit /b 1

:: repair permissions

chown -R jichi .
chmod -R 755 .

attrib +h Update

attrib +r License.tx || exit /b 1
::attrib +r Readme.txt
::attrib +r Changes.txt
attrib +r Update.tx || exit /b 1

:: See: http://msdn.microsoft.com/en-us/library/windows/desktop/cc144102(v=vs.85).aspx
attrib +h icon.ico
attrib +h +s desktop.ini
attrib +r .

:: archive
::call "Delete Caches.cmd"

cd ..

rm -f "%ZIPFILE%"
zip -9r "%ZIPFILE%" "%TARGET%" > nul

:: EOF
