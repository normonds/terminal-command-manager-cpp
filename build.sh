rm cli
rm build.log
g++ -o cli cli.cpp /usr/lib/x86_64-linux-gnu/libncursesw.a -static-libgcc -static-libstdc++ /usr/lib/x86_64-linux-gnu/libmenu.a /usr/lib/x86_64-linux-gnu/libtinfo.a -lncursesw -lform -Wl,--verbose > build.log
chmod +x cli
./cli

# with linked libraries
# g++ cli.cpp -lncursesw -lform -o cli && chmod +x cli &&  ./cli