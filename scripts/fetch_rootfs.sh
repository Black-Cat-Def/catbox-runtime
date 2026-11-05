#!/usr/bin/env bash
set -euo pipefail

ROOTFS_DIR=".rootfs/busybox"
BUSYBOX_URL="https://busybox.net/downloads/binaries/1.35.0-x86_64-linux-musl/busybox"

echo "Подготовка корневой файловой системы (rootfs) в ${ROOTFS_DIR}..."

mkdir -p "${ROOTFS_DIR}/bin"
mkdir -p "${ROOTFS_DIR}/dev"
mkdir -p "${ROOTFS_DIR}/proc"
mkdir -p "${ROOTFS_DIR}/sys"
mkdir -p "${ROOTFS_DIR}/etc"

echo "Скачивание статически скомпилированного busybox..."
curl -fsSL "${BUSYBOX_URL}" -o "${ROOTFS_DIR}/bin/busybox"
chmod +x "${ROOTFS_DIR}/bin/busybox"

echo "Создание символических ссылок для стандартных утилит..."
cd "${ROOTFS_DIR}/bin"

if ./busybox --install -s . 2>/dev/null; then
    echo "Успешно установлены все доступные утилиты BusyBox."
else
    for cmd in sh bash ls ps cat echo kill sleep mkdir rmdir rm pwd env \
               hostname uname whoami id which grep egrep fgrep sed awk \
               find xargs top free df du head tail wc tr cut sort uniq \
               touch cp mv ln chmod chown ping ip netstat clear vi test; do
        ln -sf busybox "${cmd}"
    done
fi

echo "Rootfs успешно подготовлен в ${ROOTFS_DIR}!"
