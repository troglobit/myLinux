#!/bin/sh
. $BR2_CONFIG 2>/dev/null

TARGET_DIR=$1
MY_VERSION=$(git describe --always --dirty --tags)

# This is a symlink to /usr/lib/os-release, so we remove this to keep
# original Buildroot information.
rm $TARGET_DIR/etc/os-release

echo "NAME=myLinux"                           > "$TARGET_DIR/etc/os-release"
echo "VERSION=${MY_VERSION}"                 >> "$TARGET_DIR/etc/os-release"
echo "ID=mylinux"                            >> "$TARGET_DIR/etc/os-release"
echo "VERSION_ID=${MY_VERSION}"              >> "$TARGET_DIR/etc/os-release"
echo "PRETTY_NAME=\"myLinux ${MY_VERSION}\"" >> "$TARGET_DIR/etc/os-release"
echo "VARIANT=${BR2_ARCH}"                   >> "$TARGET_DIR/etc/os-release"
echo "VARIANT_ID=${BR2_ARCH}"                >> "$TARGET_DIR/etc/os-release"
echo "HOME_URL=https://troglobit.com"        >> "$TARGET_DIR/etc/os-release"
