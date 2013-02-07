unzip wavelet-matrix-cpp-0e7e59.zip
mv wavelet-matrix-cpp-0e7e5993e209eb214b7f08349a678b320e458157 wavelet-matrix-cpp
cd wavelet-matrix-cpp
python waf configure
python waf build
cp build/default/src/libwavelet-matrix.a ../lib




