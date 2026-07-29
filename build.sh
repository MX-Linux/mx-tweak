#!/bin/bash
# Build script - see below for command information.
set -e
case "${1:-all}" in
	clean)
		echo "Performing ultimate clean..."
		rm -rf _build_
		;;

	configure)
		echo "Configuring project..."
		cmake --preset default
		;;

	make-clean)
		echo "Cleaning build artifacts..."
		cmake --build --preset default --target clean
		;;

	make)
		echo "Building project..."
		cmake --build --preset default
		;;

	all)
		echo "Configuring and building project..."
		cmake --workflow --preset default
		;;

	fresh)
		echo "Fresh build (clean first, then configure and build)..."
		cmake --workflow --preset default --fresh
		;;

	--debian)
		echo "Building Debian package..."
		debuild -us -uc

		echo "Creating debs directory and moving debian artifacts..."
		mkdir -p debs
		mv ../mx-tweak_*.deb debs/ 2>/dev/null || true
		mv ../mx-tweak_*.changes debs/ 2>/dev/null || true
		mv ../mx-tweak_*.dsc debs/ 2>/dev/null || true
		mv ../mx-tweak_*.tar.* debs/ 2>/dev/null || true
		mv ../mx-tweak_*.buildinfo debs/ 2>/dev/null || true

		echo "Cleaning build directory and debian artifacts..."
		rm -rf _build_
		rm -f debian/*.debhelper.log debian/*.substvars debian/files
		rm -rf debian/.debhelper/ debian/mx-tweak/ obj-*/

		echo "Debian package build completed!"
		echo "Debian artifacts moved to debs/ directory"
		;;

	*)
		echo "Usage: $0 [command]"
		echo "Commands:"
		echo "  clean        - Ultimate clean (rm -rf build)"
		echo "  configure    - Configure only"
		echo "  make-clean   - Clean build artifacts only"
		echo "  make         - Build only"
		echo "  all          - Configure and build (default)"
		echo "  fresh        - Clean first then configure and build"
		echo "  --debian     - Build Debian package (debuild) and move artifacts to debs/"
		exit 1
		;;
esac