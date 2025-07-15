Package: icu[core,tools]:x64-windows@74.2#6

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35207.1
-    vcpkg-tool version: 2025-06-20-ef7c0d541124bbdd334a03467e7edb6c3364d199
    vcpkg-scripts version: 30f771d4ac 2025-07-14 (8 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached icu4c-74_2-src.tgz
-- Cleaning sources at C:/Users/ibram/git clones/vcpkg/buildtrees/icu/src/c-74_2-src-2c60f3f6bd.clean. Use --editable to skip cleaning for the packages you specify.
-- Extracting source C:/Users/ibram/git clones/vcpkg/downloads/icu4c-74_2-src.tgz
-- Applying patch disable-escapestr-tool.patch
-- Applying patch remove-MD-from-configure.patch
-- Applying patch fix_parallel_build_on_windows.patch
-- Applying patch fix-extra.patch
-- Applying patch mingw-dll-install.patch
-- Applying patch disable-static-prefix.patch
-- Applying patch fix-win-build.patch
-- Applying patch vcpkg-cross-data.patch
-- Applying patch darwin-rpath.patch
-- Applying patch mingw-strict-ansi.diff
-- Applying patch cleanup_msvc.patch
-- Using source at C:/Users/ibram/git clones/vcpkg/buildtrees/icu/src/c-74_2-src-2c60f3f6bd.clean
-- Warning: Paths with embedded space may be handled incorrectly by configure:
   C:/Users/ibram/git clones/vcpkg/buildtrees/icu
   C:/Users/ibram/git clones/vcpkg/packages/icu_x64-windows
   Please move the path to one without whitespaces!
-- Getting CMake variables for x64-windows
-- Loading CMake variables from C:/Users/ibram/git clones/vcpkg/buildtrees/icu/cmake-get-vars_C_CXX-x64-windows.cmake.log
-- Using cached msys2-autoconf-wrapper-20240607-1-any.pkg.tar.zst
-- Using cached msys2-automake-wrapper-20240607-1-any.pkg.tar.zst
-- Using cached msys2-autoconf-archive-2023.02.20-1-any.pkg.tar.zst
-- Using cached msys2-binutils-2.44-1-x86_64.pkg.tar.zst
-- Using cached msys2-libtool-2.5.4-1-x86_64.pkg.tar.zst
-- Using cached msys2-make-4.4.1-2-x86_64.pkg.tar.zst
-- Using cached msys2-which-2.23-4-x86_64.pkg.tar.zst
-- Using cached msys2-bash-5.2.037-2-x86_64.pkg.tar.zst
-- Using cached msys2-coreutils-8.32-5-x86_64.pkg.tar.zst
-- Using cached msys2-file-5.46-2-x86_64.pkg.tar.zst
-- Using cached msys2-gawk-5.3.2-1-x86_64.pkg.tar.zst
-- Using cached msys2-grep-1~3.0-7-x86_64.pkg.tar.zst
-- Using cached msys2-gzip-1.14-1-x86_64.pkg.tar.zst
-- Using cached msys2-diffutils-3.12-1-x86_64.pkg.tar.zst
-- Using cached msys2-pkgconf-2.4.3-1-x86_64.pkg.tar.zst
-- Using cached msys2-sed-4.9-1-x86_64.pkg.tar.zst
-- Using cached msys2-msys2-runtime-3.6.2-2-x86_64.pkg.tar.zst
-- Using cached msys2-autoconf2.72-2.72-3-any.pkg.tar.zst
-- Using cached msys2-automake1.16-1.16.5-1-any.pkg.tar.zst
-- Using cached msys2-automake1.17-1.17-1-any.pkg.tar.zst
-- Using cached msys2-libiconv-1.18-1-x86_64.pkg.tar.zst
-- Using cached msys2-libintl-0.22.5-1-x86_64.pkg.tar.zst
-- Using cached msys2-zlib-1.3.1-1-x86_64.pkg.tar.zst
-- Using cached msys2-findutils-4.10.0-2-x86_64.pkg.tar.zst
-- Using cached msys2-tar-1.35-2-x86_64.pkg.tar.zst
-- Using cached msys2-gmp-6.3.0-1-x86_64.pkg.tar.zst
-- Using cached msys2-gcc-libs-13.3.0-1-x86_64.pkg.tar.zst
-- Using cached msys2-libbz2-1.0.8-4-x86_64.pkg.tar.zst
-- Using cached msys2-liblzma-5.8.1-1-x86_64.pkg.tar.zst
-- Using cached msys2-libzstd-1.5.7-1-x86_64.pkg.tar.zst
-- Using cached msys2-libreadline-8.2.013-1-x86_64.pkg.tar.zst
-- Using cached msys2-mpfr-4.2.2-1-x86_64.pkg.tar.zst
-- Using cached msys2-libpcre-8.45-5-x86_64.pkg.tar.zst
-- Using cached msys2-m4-1.4.19-2-x86_64.pkg.tar.zst
-- Using cached msys2-perl-5.38.4-2-x86_64.pkg.tar.zst
-- Using cached msys2-ncurses-6.5.20240831-2-x86_64.pkg.tar.zst
-- Using cached msys2-libxcrypt-4.4.38-1-x86_64.pkg.tar.zst
-- Using msys root at C:/Users/ibram/git clones/vcpkg/downloads/tools/msys2/8392cd453c24d30d
-- Generating configure for x64-windows
CMake Error at scripts/cmake/vcpkg_execute_required_process.cmake:127 (message):
    Command failed: "C:/Users/ibram/git clones/vcpkg/downloads/tools/msys2/8392cd453c24d30d/usr/bin/bash.exe" --noprofile --norc --debug -c "C:/Users/ibram/git clones/vcpkg/downloads/tools/msys2/8392cd453c24d30d/usr/bin/autoreconf -vfi"
    Working Directory: C:/Users/ibram/git clones/vcpkg/buildtrees/icu/src/c-74_2-src-2c60f3f6bd.clean/source
    Error code: 127
    See logs for more information:
      C:\Users\ibram\git clones\vcpkg\buildtrees\icu\autoconf-x64-windows-err.log

Call Stack (most recent call first):
  C:/Users/ibram/Projects/harmony/vcpkg_installed/x64-windows/share/vcpkg-make/vcpkg_make.cmake:41 (vcpkg_execute_required_process)
  C:/Users/ibram/Projects/harmony/vcpkg_installed/x64-windows/share/vcpkg-make/vcpkg_make.cmake:94 (vcpkg_run_shell)
  C:/Users/ibram/Projects/harmony/vcpkg_installed/x64-windows/share/vcpkg-make/vcpkg_make_configure.cmake:62 (vcpkg_run_autoreconf)
  C:/Users/ibram/AppData/Local/vcpkg/registries/git-trees/ebd75351b43b485143b74a866381afafcd28b77b/portfile.cmake:68 (vcpkg_make_configure)
  scripts/ports.cmake:206 (include)



```

<details><summary>C:\Users\ibram\git clones\vcpkg\buildtrees\icu\autoconf-x64-windows-err.log</summary>

```
/usr/bin/bash: line 1: C:/Users/ibram/git: No such file or directory
```
</details>

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "harmony",
  "version": "0.1.0",
  "dependencies": [
    "fmt",
    "boost-system",
    "qtbase"
  ]
}

```
</details>
