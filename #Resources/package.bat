REM Shut up, I'm just as lazy as anyone else.
"C:\Program Files\7-Zip\7z" a -t7z -mx=9 -m0=lzma2 -aoa -mfb=64 -md=32m -ms=on  "Output/OBS-AMD-AMF-Plugin.7z" "../enc-amf/#Build/Release/*"
"C:\Program Files\7-Zip\7z" a -tzip -mx=9 "Output/OBS-AMD-AMF-Plugin.zip" "../enc-amf/#Build/Release/*"
Pause
"C:\Program Files (x86)\Inno Setup 5\Compil32.exe" /cc "Installer.iss"