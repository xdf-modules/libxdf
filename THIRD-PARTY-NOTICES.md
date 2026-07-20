# Third-party notices

libxdf's own source code is distributed under the BSD 2-Clause License (see
[LICENSE](LICENSE)). It bundles two third-party libraries, each of which
keeps its own original license — bundling them does not relicense them, and
they are not covered by libxdf's own BSD 2-Clause terms.

## smarc (`smarc/`)

Copyright (c) 2009-2011 Institut Télécom - Télécom ParisTech, dept. TSI
Authors: Benoit Mathieu, Jacques Prado

Licensed under the **GNU Lesser General Public License, version 3 or later**
(LGPL-3.0-or-later). See the header of any file under `smarc/` or
<https://www.gnu.org/licenses/lgpl-3.0.html> for the full license text.

smarc's `.c`/`.h` files are compiled directly into the `xdf` library target.
Because libxdf is distributed as complete, buildable source (including the
unmodified smarc sources), anyone receiving libxdf already has everything
needed to modify smarc and relink it against the rest of the library, which
satisfies the LGPLv3 §4 requirements for this kind of static combination.

## pugixml (`pugixml/`)

Copyright (c) 2006-2025 Arseny Kapoulkine

Licensed under the **MIT License**. See the header of `pugixml/pugixml.hpp`
or <https://opensource.org/license/mit> for the full license text.

pugixml is used either as a system package (`find_package(pugixml)`) or, if
unavailable, built from the bundled copy in `pugixml/`.
