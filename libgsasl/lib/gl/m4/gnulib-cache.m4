# Copyright (C) 2002-2012 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http://www.gnu.org/licenses/>.
#
# As a special exception to the GNU General Public License,
# this file may be distributed as part of a program that
# contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the specification of how gnulib-tool is used.
# It acts as a cache: It is written and read by gnulib-tool.
# In projects that use version control, this file is meant to be put under
# version control, like the configure.ac and various Makefile.am files.


# Specification in the form of a command-line invocation:
#   gnulib-tool --import --dir=. --local-dir=gl/override --lib=libgl --source-base=gl --m4-base=gl/m4 --doc-base=doc --tests-base=gltests --aux-dir=build-aux --with-tests --avoid=vc-list-files-tests --lgpl=2 --no-conditional-dependencies --libtool --macro-prefix=gl --no-vc-files base64 c-ctype crypto/gc crypto/gc-hmac-md5 crypto/gc-hmac-sha1 crypto/gc-md5 crypto/gc-pbkdf2-sha1 crypto/gc-random crypto/gc-sha1 getline gettext gss-extra lib-msvc-compat lib-symbol-versions lib-symbol-visibility maintainer-makefile memmem memxor minmax strndup strnlen strverscmp vasprintf

# Specification in the form of a few gnulib-tool.m4 macro invocations:
gl_LOCAL_DIR([gl/override])
gl_MODULES([
  base64
  c-ctype
  crypto/gc
  crypto/gc-hmac-md5
  crypto/gc-hmac-sha1
  crypto/gc-md5
  crypto/gc-pbkdf2-sha1
  crypto/gc-random
  crypto/gc-sha1
  getline
  gettext
  gss-extra
  lib-msvc-compat
  lib-symbol-versions
  lib-symbol-visibility
  maintainer-makefile
  memmem
  memxor
  minmax
  strndup
  strnlen
  strverscmp
  vasprintf
])
gl_AVOID([vc-list-files-tests])
gl_SOURCE_BASE([gl])
gl_M4_BASE([gl/m4])
gl_PO_BASE([])
gl_DOC_BASE([doc])
gl_TESTS_BASE([gltests])
gl_WITH_TESTS
gl_LIB([libgl])
gl_LGPL([2])
gl_MAKEFILE_NAME([])
gl_LIBTOOL
gl_MACRO_PREFIX([gl])
gl_PO_DOMAIN([])
gl_WITNESS_C_MACRO([])
gl_VC_FILES([false])