tar xvf wat_array-0.0.6.tar.bz2
mv wat_array-0.0.6 wat_array
cd wat_array
python waf configure
python waf build
cp build/default/src/libwat_array.a ../lib




