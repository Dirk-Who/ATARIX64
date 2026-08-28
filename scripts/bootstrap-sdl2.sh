#!/bin/sh

set -eu

SDL_VERSION="2.32.10"
SDL_SHA256="4a7ac31640d70214e848f994be8a12849c0f97918a7e6c2e27a40036166d1a7f"
SDL_URL="https://www.libsdl.org/release/SDL2-${SDL_VERSION}.dmg"

if [ "$(uname -s)" != "Darwin" ]; then
	echo "This bootstrap script must run on macOS." >&2
	exit 1
fi

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_DIR=$(CDPATH= cd -- "${SCRIPT_DIR}/../src/AtariX-MT/AtariX" && pwd)
TARGET_FRAMEWORK="${PROJECT_DIR}/SDL2.framework"
WORK_DIR=$(mktemp -d "${TMPDIR:-/tmp}/atarix-sdl2.XXXXXX")
MOUNT_DIR="${WORK_DIR}/volume"
DMG_PATH="${WORK_DIR}/SDL2.dmg"
NEW_FRAMEWORK="${WORK_DIR}/SDL2.framework.new"
OLD_FRAMEWORK="${WORK_DIR}/SDL2.framework.old"
MOUNTED=false

cleanup()
{
	if [ "${MOUNTED}" = true ]; then
		hdiutil detach "${MOUNT_DIR}" -quiet || true
	fi
	rm -rf "${WORK_DIR}"
}
trap cleanup EXIT HUP INT TERM

mkdir "${MOUNT_DIR}"
echo "Downloading SDL ${SDL_VERSION}..."
curl --fail --location --retry 3 --output "${DMG_PATH}" "${SDL_URL}"

ACTUAL_SHA256=$(shasum -a 256 "${DMG_PATH}" | awk '{print $1}')
if [ "${ACTUAL_SHA256}" != "${SDL_SHA256}" ]; then
	echo "SDL checksum mismatch: ${ACTUAL_SHA256}" >&2
	exit 1
fi

hdiutil attach "${DMG_PATH}" -nobrowse -readonly -mountpoint "${MOUNT_DIR}" -quiet
MOUNTED=true

if [ ! -d "${MOUNT_DIR}/SDL2.framework" ]; then
	echo "SDL2.framework was not found in the downloaded disk image." >&2
	exit 1
fi

# Framework bundles contain the usual Headers/Resources/Versions symlink
# layout. macOS 26's ditto may follow those links and then fail with
# "Is a directory"; cp -R preserves the framework links as links.
cp -R "${MOUNT_DIR}/SDL2.framework" "${NEW_FRAMEWORK}"

ARCHITECTURES=$(lipo -archs "${NEW_FRAMEWORK}/SDL2")
case " ${ARCHITECTURES} " in
	*" arm64 "*) ;;
	*)
		echo "The installed SDL framework has no arm64 slice: ${ARCHITECTURES}" >&2
		exit 1
		;;
esac

if [ -e "${TARGET_FRAMEWORK}" ] || [ -L "${TARGET_FRAMEWORK}" ]; then
	mv "${TARGET_FRAMEWORK}" "${OLD_FRAMEWORK}"
fi

if ! mv "${NEW_FRAMEWORK}" "${TARGET_FRAMEWORK}"; then
	if [ -e "${OLD_FRAMEWORK}" ] || [ -L "${OLD_FRAMEWORK}" ]; then
		mv "${OLD_FRAMEWORK}" "${TARGET_FRAMEWORK}"
	fi
	echo "Could not install the new SDL framework." >&2
	exit 1
fi

echo "Installed SDL ${SDL_VERSION} (${ARCHITECTURES}) at ${TARGET_FRAMEWORK}"
