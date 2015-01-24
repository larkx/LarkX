NOTE:

The source code of this software was originally released into the public domain
(see licenses/UNLICENSE.md). However, since this software incorporates code
licensed under the GNU Affero General Public License (AGPL) in the libraries/fc
submodule, and since this software links the readline library licensed under the
GNU General Public License (GPL), this is legally questionable if not
impossible.

Additionally, this software package incorporates various pieces of code licensed
under the BSD or MIT licenses and their variants. This adds the additional
requirement of mentioning the respective authors and licenses in binary
distributions as well. Obviously, these licenses must also be kept in the
source.

The only option really is to license the whole package under the AGPL as
follows, with the additional BSD / MIT licenses listed at the end of this file:

================================================================================

Copyright (C) 2015 The LarkX Team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

================================================================================

The following additional terms have to be published along with binaries
of this software. Some of these may be used in the form of dynamically linked
libraries distributed separately, most notably leveldb and libudt. In that case,
the respective licenses do not apply to the binary of this software.
Also, some of the files are only used for building binaries of this software,
without becoming an actual part of those binaries. Their respective licenses
also do not apply to the binaries.


libraries/fc/src/utf8 is copyright Nemanja Trifunovic and covered by the
MIT license (Thrift variant). See licenses/MIT_Thrift_LICENSE.md.

libraries/fc/src/crypto/base58.cpp is copyright Satoshi Nakamoto and
The Bitcoin Developers and is covered by the MIT/X11 license.
See licenses/MIT_X11_LICENSE.md.

libraries/fc/src/crypto/base64.cpp is copyright René Nyffenegger and covered by
the zlib/libpng license. See licenses/ZLIB_LICENSE.md.

libraries/fc/src/crypto/city.cpp and libraries/fc/include/fc/crypto/city.hpp are
copyright Google, Inc. and covered by the MIT license (modern style with
sublicense). See licenses/MIT_Modern.md.

libraries/fc/src/crypto/romix.cpp and libraries/fc/include/fc/crypto/romix.hpp
are copyright Armory Technologies, Inc. and specifically covered by the
GNU Affero GPL. See licenses/agpl-3.0.txt.

libraries/fc/src/uint128.cpp is copyright Evan Teran and covered by the MIT
license (old style, no advertising). See licenses/MIT_old.md.

libraries/fc/GitVersionGen/GetGitRevisionDescription.cmake is copyright
Iowa State University and covered by the Boost Software License.
See licenses/Boost_License-1.0.txt.

libraries/fc/CMakeModules/FindBoost.cmake is copyright Kitware, Inc., Andreas Schneider,
Wengo, Mike Jackson, Andreas Pakulat and Philip Lowman and covered by the BSD license
(3-clause). See licenses/CMake_License.txt.

libraries/fc/CMakeModules/FindWt.cmake is copyright Pau Garcia i Quiles and covered by the
BSD license (3-clause). See licenses/COPYING-CMAKE-SCRIPTS.

libraries/fc/vendor/udt4 is copyright University of Illinois and covered by the BSD license
(3-clause). See licenses/UDT_License.txt.

libraries/fc/vendor/cyoencode-1.0.2 is copyright Graham Bull and covered by the BSD license
(2-clause). See licenses/Cyo_License.txt.

libraries/leveldb and vendor/leveldb-1.12.0 are copyright The LevelDB Authors and covered by
the BSD license (3-clause). See licenses/LevelDB_License.txt.

vendor/miniupnp is copyright Thomas Bernard and covered by the BSD license (3-clause).
See licenses/MiniUPNP_License.txt.

CMakeModules/cotire.cmake is copyright Sascha Kratky and covered by the MIT license (modern
style with sublicense). See licenses/See MIT_Modern.md.

CrashRpt is copyright The CrashRpt project authors and covered by the BSD license (3-clause).
See CrashRpt/License.txt.

programs/utils/seeder.pl is copyright Peter Conrad and covered by the BSD license (3-clause).
See programs/utils/seeder.pl.
