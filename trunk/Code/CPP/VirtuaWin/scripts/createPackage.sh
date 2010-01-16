#
# Script for automating the build and release process as much as possible.
#
# Preconditions:
# - shell already configured to run gcc with MinGW and CVS (i.e. CVSROOT and CVS_RSH set)
# - HTML Help compiler (set HELPCOMPILER)
# - Inno Setup (set SETUPCOMPILER if not in standard install location)
# - emacs (or set EDITOR to another good text editor)
# - zip (set ZIP) or winzip with commandline extension
# - ncftp (if uploading to SF)
#
# Note: The CVS program must be windows so line termination will be Windows format (i.e. \r\n) otherwise
#       the source package will be wrong and the README files will not be very readable in notepad.
#       This means that Cygwin's cvs cannot be used!!
#
# Variables
# MSYS need windows style /? arguments to be double slashes, i.e. //? use a varable to handle this.
# sed is used to set the version, but it must not change the line termination to unix style, use option -b
# for cygwin's sed and -c for msys.
SLASH="/"
SED="sed -b"
CDRIVE="/cygdrive/c"
if [ `uname | sed -e "s/^MINGW.*/MINGW/"` == 'MINGW' ] ; then
    SLASH="//"
    SED="sed -c"
    CDRIVE="/c"
fi
if [ -z "$VWPROGRAMFILES" ] ; then
    if [ -z "$PROGRAMW6432" ] ; then
        VWPROGRAMFILES="${CDRIVE}/Program Files"
    else
        VWPROGRAMFILES="${CDRIVE}/Program Files (x86)"
    fi
fi
if [ -z "$EDITOR" ] ; then
    EDITOR=emacs
fi
if [ -z "$HELPCOMPILER" ] ; then
    HELPCOMPILER="${VWPROGRAMFILES}/HTML Help Workshop/hhc"
fi
if [ -z "$SETUPCOMPILER" ] ; then
    SETUPCOMPILER="${VWPROGRAMFILES}/Inno Setup 5/Compil32"
fi
if [ -z "$WINZIP" ] ; then
    WINZIP="${VWPROGRAMFILES}/WinZip/wzzip"
fi

if [ -z "$1" ] ; then
    echo "Usage: createPackage [-TEST] <version> [<label>]"
    echo "E.g.:"
    echo "      createPackage 3.2.0.0 beta1"
    echo "      createPackage 3.2 beta2"
    echo "      createPackage 3.2"
    exit -1
fi
if [ "$1" == "-TEST" ] ; then
    if [ ! -r VirtuaWin.c ] ; then
        echo "Test Usage Error: must run script from the source directory"
        exit -1
    fi
    echo "Testing packaging"
    shift
    TEST_PACKAGE=1
fi

ver_mjr=`echo $1 | awk -F"." '{ print $1+0 }'`
ver_mnr=`echo $1 | awk -F"." '{ print $2+0 }'`
ver_rev=`echo $1 | awk -F"." '{ print $3+0 }'`
ver_bno=`echo $1 | awk -F"." '{ print $4 }'`
ver_lbl=$2
version=${ver_mjr}.${ver_mnr}
if [ "$ver_rev" != "0" ] ; then
    version=${version}.${ver_rev}
fi
file_ver=${version}
if [ -n "$ver_lbl" ] ; then
    version="${version} ${ver_lbl}"
    file_ver="${file_ver}_${ver_lbl}"
fi

echo Creating VirtuaWin package: $version - $file_ver

if [ -z "$TEST_PACKAGE" ] ; then
    mkdir -p ./$file_ver
    cd ./$file_ver

    cvs checkout README.TXT
    cvs update -d
fi

if [ -z "$ver_bno" ] ; then
    ver_bno=`grep FILEVERSION VirtuaWin.rc | awk -F"," '{ print $4+1 }'`
fi
echo Build: $ver_bno

mkdir -p tmp
mkdir -p tmp/standard
mkdir -p tmp/portable
mkdir -p tmp/portable/config
mkdir -p tmp/portable/modules
mkdir -p tmp/portable/icons
mkdir -p tmp/portable_unicode
mkdir -p tmp/portable_unicode/config
mkdir -p tmp/portable_unicode/modules
mkdir -p tmp/portable_unicode/icons
mkdir -p tmp/unicode

cat Defines.h | ${SED} -e "s/vwVIRTUAWIN_NAME_VERSION _T(\"VirtuaWin v.*\")/vwVIRTUAWIN_NAME_VERSION _T(\"VirtuaWin v$version\")/" > Defines.h.tmp
mv Defines.h.tmp Defines.h
cat VirtuaWin.rc | ${SED} -e "s/^ FILEVERSION .*/ FILEVERSION ${ver_mjr},${ver_mnr},${ver_rev},${ver_bno}/" > VirtuaWin.rc.tmp
cat VirtuaWin.rc.tmp | ${SED} -e "s/^ PRODUCTVERSION .*/ PRODUCTVERSION ${ver_mjr},${ver_mnr},${ver_rev},${ver_bno}/" > VirtuaWin.rc
cat VirtuaWin.rc | ${SED} -e "s/ VALUE \"FileVersion\", \"[.0-9]*\\\\0\"/ VALUE \"FileVersion\", \"${ver_mjr}.${ver_mnr}.${ver_rev}.${ver_bno}\\\\0\"/" > VirtuaWin.rc.tmp
cat VirtuaWin.rc.tmp | ${SED} -e "s/ VALUE \"ProductVersion\", \"[.0-9]*\\\\0\"/ VALUE \"ProductVersion\", \"${ver_mjr}.${ver_mnr}.${ver_rev}.${ver_bno}\\\\0\"/" > VirtuaWin.rc
rm VirtuaWin.rc.tmp
cat vwHook.rc | ${SED} -e "s/^ FILEVERSION .*/ FILEVERSION ${ver_mjr},${ver_mnr},${ver_rev},${ver_bno}/" > vwHook.rc.tmp
cat vwHook.rc.tmp | ${SED} -e "s/^ PRODUCTVERSION .*/ PRODUCTVERSION ${ver_mjr},${ver_mnr},${ver_rev},${ver_bno}/" > vwHook.rc
cat vwHook.rc | ${SED} -e "s/ VALUE \"FileVersion\", \"[.0-9]*\\\\0\"/ VALUE \"FileVersion\", \"${ver_mjr}.${ver_mnr}.${ver_rev}.${ver_bno}\\\\0\"/" > vwHook.rc.tmp
cat vwHook.rc.tmp | ${SED} -e "s/ VALUE \"ProductVersion\", \"[.0-9]*\\\\0\"/ VALUE \"ProductVersion\", \"${ver_mjr}.${ver_mnr}.${ver_rev}.${ver_bno}\\\\0\"/" > vwHook.rc
rm vwHook.rc.tmp
cat Help/VirtuaWin_Overview.htm | ${SED} -e "s/VirtuaWin v[.0-9]* Help/VirtuaWin v${ver_mjr}.${ver_mnr} Help/" > Help/VirtuaWin_Overview.htm.tmp
mv Help/VirtuaWin_Overview.htm.tmp Help/VirtuaWin_Overview.htm
cat WinList/winlist.rc | ${SED} -e "s/^CAPTION \"WinList v.*\"/CAPTION \"WinList v$version\"/" > WinList/winlist.rc.tmp
mv WinList/winlist.rc.tmp WinList/winlist.rc
cat scripts/virtuawin.iss | ${SED} -e "s/^AppVerName=VirtuaWin v.*/AppVerName=VirtuaWin v$version/" > scripts/virtuawin.iss.tmp
mv scripts/virtuawin.iss.tmp scripts/virtuawin.iss
$EDITOR HISTORY.TXT
cp Defines.h Messages.h Module/
read -p "Compile source? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo compiling helpfile
    cd Help
    "$HELPCOMPILER" virtuawin.hhp
    cd ..
    echo building standard
    make -f Makefile spotless
    make -f Makefile
    cd WinList
    make -f Makefile spotless
    make -f Makefile
    cd ..
    echo copying standard
    cp ./VirtuaWin.exe ./tmp/standard/
    cp ./vwHook.dll ./tmp/standard/
    cp ./Icons/1[0-9].ico ./tmp/standard/
    cp ./Icons/20.ico ./tmp/standard/
    cp ./WinList/WinList.exe ./tmp/standard/
    cp ./READMEII.TXT ./tmp/standard/README.TXT
    cp ./HISTORY.TXT ./tmp/standard/
    cp ./COPYING.TXT ./tmp/standard/
    cp ./Help/VirtuaWin.chm ./tmp/standard/VirtuaWin.chm
    cp ./scripts/VirtuaWin5.0.ISL ./tmp/standard/
    cp ./scripts/virtuawin.iss ./tmp/standard/
    echo done standard
    echo copying portable
    cp ./VirtuaWin.exe ./tmp/portable/
    cp ./vwHook.dll ./tmp/portable/
    cp ./Icons/1[0-9].ico ./tmp/portable/icons/
    cp ./Icons/20.ico ./tmp/portable/icons/
    cp ./WinList/WinList.exe ./tmp/portable/modules/
    cp ./READMEII.TXT ./tmp/portable/README.TXT
    cp ./HISTORY.TXT ./tmp/portable/
    cp ./COPYING.TXT ./tmp/portable/
    cp ./Help/VirtuaWin.chm ./tmp/portable/VirtuaWin.chm
    cp ./scripts/portable_userpath.cfg ./tmp/portable/userpath.cfg
    echo done portable
    echo building unicode
    make -f Makefile spotless
    make -f Makefile vwUNICODE=1
    cd WinList
    make -f Makefile spotless
    make -f Makefile vwUNICODE=1
    cd ..
    echo copying unicode
    cp ./VirtuaWin.exe ./tmp/unicode/
    cp ./vwHook.dll ./tmp/unicode/
    cp ./Icons/1[0-9].ico ./tmp/unicode/
    cp ./Icons/20.ico ./tmp/unicode/
    cp ./WinList/WinList.exe ./tmp/unicode/
    cp ./READMEII.TXT ./tmp/unicode/README.TXT
    cp ./HISTORY.TXT ./tmp/unicode/
    cp ./COPYING.TXT ./tmp/unicode/
    cp ./Help/VirtuaWin.chm ./tmp/unicode/VirtuaWin.chm
    cp ./scripts/VirtuaWin5.0.ISL ./tmp/unicode/
    cat ./scripts/virtuawin.iss | sed -e "s/^AppVerName=VirtuaWin/AppVerName=VirtuaWin Unicode/" > ./tmp/unicode/virtuawin.iss
    echo done unicode
    echo copying portable_unicode
    cp ./VirtuaWin.exe ./tmp/portable_unicode/
    cp ./vwHook.dll ./tmp/portable_unicode/
    cp ./Icons/1[0-9].ico ./tmp/portable_unicode/icons/
    cp ./Icons/20.ico ./tmp/portable_unicode/icons/
    cp ./WinList/WinList.exe ./tmp/portable_unicode/modules/
    cp ./READMEII.TXT ./tmp/portable_unicode/README.TXT
    cp ./HISTORY.TXT ./tmp/portable_unicode/
    cp ./COPYING.TXT ./tmp/portable_unicode/
    cp ./Help/VirtuaWin.chm ./tmp/portable_unicode/VirtuaWin.chm
    cp ./scripts/portable_userpath.cfg ./tmp/portable_unicode/userpath.cfg
    echo done portable_unicode
fi

read -p "Create packages? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo Creating standard package
    cd ./tmp/standard
    "$SETUPCOMPILER" ${SLASH}cc virtuawin.iss

    echo Creating portable package
    cd ../portable
    rm -f VirtuaWin_portable_$file_ver.zip
    if [ -z "$ZIP" ] ; then
        "$WINZIP" VirtuaWin_portable_$file_ver.zip -P @./scripts/portable_filelist
    else
        $ZIP -9 -r -@ < ../../scripts/portable_filelist
        mv zip.zip VirtuaWin_portable_$file_ver.zip
    fi

    echo Creating unicode package
    cd ../unicode
    "$SETUPCOMPILER" ${SLASH}cc virtuawin.iss

    echo Creating portable_unicode package
    cd ../portable_unicode
    rm -f VirtuaWin_portable_unicode_$file_ver.zip
    if [ -z "$ZIP" ] ; then
        "$WINZIP" VirtuaWin_portable_unicode_$file_ver.zip -P @./scripts/portable_filelist
    else
        $ZIP -9 -r -@ < ../../scripts/portable_filelist
        mv zip.zip VirtuaWin_portable_unicode_$file_ver.zip
    fi

    cd ../..

    echo Creating source package
    rm -f VirtuaWin_source_$file_ver.zip
    if [ -z "$ZIP" ] ; then
        "$WINZIP" VirtuaWin_source_$file_ver.zip -P @./scripts/filelist
    else
        $ZIP -9 -@ < ./scripts/filelist
        mv zip.zip VirtuaWin_source_$file_ver.zip
    fi

    echo Creating SDK package
    rm -f VirtuaWin_SDK_$file_ver.zip
    cd Module
    if [ -z "$ZIP" ] ; then
        "$WINZIP" ../VirtuaWin_SDK_$file_ver.zip -P @../scripts/SDK_filelist
    else
        $ZIP -9 -@ < ../scripts/SDK_filelist
        mv zip.zip ../VirtuaWin_SDK_$file_ver.zip
    fi
    cd ..

    echo Done!
fi
if [ -n "$TEST_PACKAGE" ] ; then
    echo "Test complete"
    exit 0
fi

read -p "Move packages? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    mkdir -p ../Distribution
    mv ./tmp/standard/output/setup.exe ../Distribution/VirtuaWin_setup_$file_ver.exe
    mv ./tmp/portable/VirtuaWin_portable_$file_ver.zip ../Distribution/
    mv ./tmp/unicode/output/setup.exe ../Distribution/VirtuaWin_unicode_setup_$file_ver.exe
    mv ./tmp/portable_unicode/VirtuaWin_portable_unicode_$file_ver.zip ../Distribution/
    mv ./VirtuaWin_source_$file_ver.zip ../Distribution/
    mv ./VirtuaWin_SDK_$file_ver.zip ../Distribution/
fi

read -p "Commit changes? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo Committing changes with comment: Changed to V$version
    cvs commit -m "Changed to V$version"
fi

read -p "Label repository? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    export LABEL=`echo V$file_ver | sed s/'\.'/_/g`
    echo Labelling with: $LABEL
    cvs tag -R -F $LABEL
fi

read -p "Upload to SourceForge? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_setup_$file_ver.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_portable_$file_ver.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_unicode_setup_$file_ver.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_portable_unicode_$file_ver.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_source_$file_ver.zip
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_SDK_$file_ver.zip
    echo Done! Go to SourceForge and click Admin-Edit/Release Files-Add Release and then type $version and follow the instructions.
fi

read -p "Cleanup temporary files? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo cleanup temporary files
    rm -r ./tmp
    echo done!

    cd ..

    read -p "Delete $file_ver directory? [y/n] " -n 1
    echo
    if [ $REPLY == 'y' ] ; then
        rm -r ./$file_ver
    fi
fi

echo Packages created in "Distribution"!
