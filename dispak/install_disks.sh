#!/bin/sh
#
# Download BESM-6 disk images.
# Usage:
#   install_disks.sh <destdir> <bindir> disk1 disk2 ...
#

#
# Download disk images from this site.
#
ARCHIVE="http://www.besm6.org/download/disks"

#
# First argument is a destination directory where disk images should be stored.
#
DESTDIR="$1"
shift

#
# Second argument is a binary directory where disk 2099 is stored.
#
BINARY="$1"
shift

mkdir -p -v "$DESTDIR"
cd "$DESTDIR" || (
    echo "Cannot create $DESTDIR"
    exit 1
)

for d in $*
do
    if [ ! -e "$d" ]; then
        if [ -f "$BINARY/$d" ]; then
            # Copy disk from binary directory.
            cp -v -a "$BINARY/$d" "$DESTDIR/$d"
        else
            # Download disk from website.
            case "$d" in
            "2048") filename="sbor2048.bin" ;;
            "2053") filename="sbor2053.bin" ;;
            "2148") filename="svs2048.bin" ;;
            "2153") filename="svs2053.bin" ;;
            "2113") filename="svs2113.bin" ;;
            "2248") filename="alt2048.bin" ;;
            "4001") filename="tape1.bin" ;;
            "2063") filename="krab2063.bin" ;;
            "2086") filename="krab2086.bin" ;;
            *)
                echo "Unknown disk $d"
                exit 1
            esac
            wget -nv -O "$d" $ARCHIVE/$filename
        fi
    fi
done
exit 0
