NOTE:

This file explains the problematic licensing of the original source package
of the BitShares GUI variants (web_wallet and by extension qt_wallet).

The qt_wallet relies on the BitShares toolkit libraries to implement the
BitShares network client functionality. The toolkit libraries are licensed
under the GNU Affero General Public License (AGPL), therefore the qt_wallet
must also be published under the AGPL. The qt_wallet relies on the web_wallet
to provide the GUI frontend for the BitShares client.

The web_wallet, however, includes code licensed under the
Creative Commons Attribution-NonCommercial 3.0 License (CC BY-NC 3.0), which
is not compatible with the AGPL. The web_wallet *may* be regarded as a
standalone work that is loosely coupled with an RPC backend provided by the
AGPL backend. It *may* therefore be legal to publish the web_wallet under the
same CC BY-NC 3.0 license.

I currently don't see a way to create a valid license for the qt_wallet.


Additionally, the web_wallet includes code from various other authors, mostly
licensed under the MIT license (Modern variant with sublicense).

Code from the AngularJS project is copyright Google, Inc. and licensed under
the terms of the MIT license (modern style with sublicense).
See licenses/MIT_Modern.md.

jsonpath.js is copyright Stefan Goessner and licensed under the terms of the
MIT license (modern style with sublicense).  See licenses/MIT_Modern.md.

jasny-bootstrap.js is copyright Jasny BV, Netherlands and licensed under the
Apache License, Version 2.0. See licenses/apl-2.0.txt.

spin.js is copyright Felix Gnass and licensed under the terms of the MIT
license (modern style with sublicense).  See licenses/MIT_Modern.md.

bootstrap.js and bootstrap.css are copyright Twitter, Inc. and licensed under
the Apache License, Version 2.0. See licenses/apl-2.0.txt.

jquery.js is copyright jQuery Foundation, Inc. and other contributors and
licensed under the terms of the MIT license (modern style with sublicense).
See licenses/MIT_Modern.md.

ui-grid.js has unknown copyright but is licensed under the terms of the
MIT license.

highcharts.src.js and highstock.src.js are licensed under the
Creative Commons Attribution-NonCommercial 3.0 License.
See licenses/CC_BY-NC_3.0.txt .

jquery.growl.js and jquery.growl.css are Copyright Kevin Sylvestre and are
licensed  under the terms of the MIT license (modern style with sublicense).
See licenses/MIT_Modern.md .

The fontawesome font is copyright Dave Gandy and licensed under the SIL
Open Font License 1.1. See licenses/OFL.txt. font-awesome.css from the
same author is licensed under the MIT license (modern style with sublicense).
See licenses/MIT_Modern.md .

The glyphicons-halflings font is copyright Jan Kovařík and licensed under
the Apache License, Version 2.0. See licenses/apl-2.0.txt.

The opensans font is copyright Steve Matteson and licensed under
the Apache License, Version 2.0. See licenses/apl-2.0.txt.
