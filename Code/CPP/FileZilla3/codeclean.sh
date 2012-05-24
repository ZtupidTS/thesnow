#! /bin/sh

echo '--- FileZilla 3 Cleaning script ---'
rm -rf  configure config.log aclocal.m4 \
  config.status config autom4te.cache libtool
find . -name "Makefile.in" | xargs rm -f
find . -name "Makefile" | xargs rm -f
echo done