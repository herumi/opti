tar xvf shellinford-r27.tgz
mv shellinford-r27 shellinford
cd shellinford
./configure && make
cp src/.libs/libshellinford.a ../lib

