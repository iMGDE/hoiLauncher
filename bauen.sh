# hook
#make 

# client
/usr/bin/c++ -I/usr/local/include -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_THREAD_SAFE -D_REENTRANT -o 'bin/AmogClient' 'src/AmogClient/main.cpp' /usr/local/lib/libfltk.a -lm -lX11 -lXext -lpthread -lXinerama -lXfixes -lXcursor -lXft -lXrender -lm -lfontconfig -ldl
echo "[100%] Built target AmogClient (?)"