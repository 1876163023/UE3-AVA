del /f z:\avagame\content\referenceshadercache.upk
del /f z:\avagame\content\localshadercache.upk
call Z:\binaries\avaGame.com precompileshaders platform=pc -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log
call Z:\binaries\avaGame.com precompileshaders platform=pc_sm2 -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log
call Z:\binaries\avaGame.com precompileshaders platform=pc_sm2_poor -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log
deploy.pl ShaderCache deployFileList_ShaderCache.txt