#cd /home/milos/Dropbox/CVUT/Omnetpp/carobs/src
make clean
make MODE=release CONFIGNAME=gcc-release all CFLAGS="-O3 -march=native"
#cp ../out/gcc-release/src/carobs .
