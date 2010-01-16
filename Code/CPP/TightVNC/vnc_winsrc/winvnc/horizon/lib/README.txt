
# $Id: README.txt 1896 2004-10-07 21:03:06Z jeffo3 $ 

HORIZON LIB README
------------------

the horizon wimba build of appshare statically links to zlib and libjpeg.
to build the libraries using cygwin, follow the instructions below.

additionally, the debug build of appshare.exe needs the follwing files
copied from the HorizonLive directory to the HorizonLiveDebug directory:

	VNCHooks.dll
	VNCHooks.lib
	omnithread_rt.dll
	omnithread_rt.lib

building libraries automagically
--------------------------------

> build-libs.sh

manually building libjpeg
-------------------------

> tar -zxvf ./downloads/jpegsrc.v6b.tar.gz
> mv jpeg-6b libjpeg
> cd libjpeg
> ./configure
> make
> cd ../
> patch -p0 < ./patches/jmorecfg.h.patch

manually building zlib
----------------------

> tar -zxvf ./downloads/zlib-1.2.1.tar.gz
> mv zlib-1.2.1 zlib
> cd zlib
> ./configure
> make
> cd ../
> patch -p0 < ./patches/zconf.h.patch

