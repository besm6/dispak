#!/bin/sh

#
# Download disk images from this site.
#
ARCHIVE="http://www.besm6.org/download/disks"

#
# Put disk images into this directory.
#
DISKDIR="$HOME/.besm6"

#
# First argument is a binary directory where disk 2099 is stored.
#
BINARY="$1"
shift

mkdir -p -v "$DISKDIR"
cd "$DISKDIR" || (
    echo "Cannot create $DISKDIR"
    exit 1
)

for d in $*
do
    if [ ! -e "$d" ]; then
        if [ "$d" = "2099" ]; then
            # Copy disk 2099 from binary directory.
            cp -v -a $BINARY/2099 $DISKDIR/2099
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
            "4002") filename="tape23.bin" ;;
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
