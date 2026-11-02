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

echo "Создание символических ссылок для базовых утилит..."
cd "${ROOTFS_DIR}/bin"
for cmd in sh ls ps cat echo kill sleep mkdir rm pwd env; do
    ln -sf busybox "${cmd}"
done

echo "Rootfs успешно подготовлен в ${ROOTFS_DIR}!"
