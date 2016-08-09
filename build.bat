call gyp build.gyp -DOS=win --depth=. -f msvs -G msvs_version=2015 --generator-output=./build_win
call MSBuild build_win\build.sln /t:Build /p:Configuration=Release /p:Platform=x64