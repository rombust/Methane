/*
**  ClanLib SDK
**  Copyright (c) 1997-2020 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    Harry Storbacka
*/

#include "precomp.h"
#include "API/Core/System/exception.h"
#include "API/Display/2D/color.h"
#include "API/Core/Text/string_format.h"
#include "API/Core/Text/string_help.h"
#include <map>

#ifndef WIN32
#include <cstdlib>
#endif

namespace clan
{
	Colorf Colorf::aliceblue(StandardColorf::aliceblue());
	Colorf Colorf::antiquewhite(StandardColorf::antiquewhite());
	Colorf Colorf::aqua(StandardColorf::aqua());
	Colorf Colorf::aquamarine(StandardColorf::aquamarine());
	Colorf Colorf::azure(StandardColorf::azure());
	Colorf Colorf::beige(StandardColorf::beige());
	Colorf Colorf::bisque(StandardColorf::bisque());
	Colorf Colorf::black(StandardColorf::black());
	Colorf Colorf::blanchedalmond(StandardColorf::blanchedalmond());
	Colorf Colorf::blue(StandardColorf::blue());
	Colorf Colorf::blueviolet(StandardColorf::blueviolet());
	Colorf Colorf::brown(StandardColorf::brown());
	Colorf Colorf::burlywood(StandardColorf::burlywood());
	Colorf Colorf::cadetblue(StandardColorf::cadetblue());
	Colorf Colorf::chartreuse(StandardColorf::chartreuse());
	Colorf Colorf::chocolate(StandardColorf::chocolate());
	Colorf Colorf::coral(StandardColorf::coral());
	Colorf Colorf::cornflowerblue(StandardColorf::cornflowerblue());
	Colorf Colorf::cornsilk(StandardColorf::cornsilk());
	Colorf Colorf::crimson(StandardColorf::crimson());
	Colorf Colorf::cyan(StandardColorf::cyan());
	Colorf Colorf::darkblue(StandardColorf::darkblue());
	Colorf Colorf::darkcyan(StandardColorf::darkcyan());
	Colorf Colorf::darkgoldenrod(StandardColorf::darkgoldenrod());
	Colorf Colorf::darkgray(StandardColorf::darkgray());
	Colorf Colorf::darkgreen(StandardColorf::darkgreen());
	Colorf Colorf::darkgrey(StandardColorf::darkgrey());
	Colorf Colorf::darkkhaki(StandardColorf::darkkhaki());
	Colorf Colorf::darkmagenta(StandardColorf::darkmagenta());
	Colorf Colorf::darkolivegreen(StandardColorf::darkolivegreen());
	Colorf Colorf::darkorange(StandardColorf::darkorange());
	Colorf Colorf::darkorchid(StandardColorf::darkorchid());
	Colorf Colorf::darkred(StandardColorf::darkred());
	Colorf Colorf::darksalmon(StandardColorf::darksalmon());
	Colorf Colorf::darkseagreen(StandardColorf::darkseagreen());
	Colorf Colorf::darkslateblue(StandardColorf::darkslateblue());
	Colorf Colorf::darkslategray(StandardColorf::darkslategray());
	Colorf Colorf::darkslategrey(StandardColorf::darkslategrey());
	Colorf Colorf::darkturquoise(StandardColorf::darkturquoise());
	Colorf Colorf::darkviolet(StandardColorf::darkviolet());
	Colorf Colorf::deeppink(StandardColorf::deeppink());
	Colorf Colorf::deepskyblue(StandardColorf::deepskyblue());
	Colorf Colorf::dimgray(StandardColorf::dimgray());
	Colorf Colorf::dimgrey(StandardColorf::dimgrey());
	Colorf Colorf::dodgerblue(StandardColorf::dodgerblue());
	Colorf Colorf::firebrick(StandardColorf::firebrick());
	Colorf Colorf::floralwhite(StandardColorf::floralwhite());
	Colorf Colorf::forestgreen(StandardColorf::forestgreen());
	Colorf Colorf::fuchsia(StandardColorf::fuchsia());
	Colorf Colorf::gainsboro(StandardColorf::gainsboro());
	Colorf Colorf::ghostwhite(StandardColorf::ghostwhite());
	Colorf Colorf::gold(StandardColorf::gold());
	Colorf Colorf::goldenrod(StandardColorf::goldenrod());
	Colorf Colorf::gray(StandardColorf::gray());
	Colorf Colorf::grey(StandardColorf::grey());
	Colorf Colorf::green(StandardColorf::green());
	Colorf Colorf::greenyellow(StandardColorf::greenyellow());
	Colorf Colorf::honeydew(StandardColorf::honeydew());
	Colorf Colorf::hotpink(StandardColorf::hotpink());
	Colorf Colorf::indianred(StandardColorf::indianred());
	Colorf Colorf::indigo(StandardColorf::indigo());
	Colorf Colorf::ivory(StandardColorf::ivory());
	Colorf Colorf::khaki(StandardColorf::khaki());
	Colorf Colorf::lavender(StandardColorf::lavender());
	Colorf Colorf::lavenderblush(StandardColorf::lavenderblush());
	Colorf Colorf::lawngreen(StandardColorf::lawngreen());
	Colorf Colorf::lemonchiffon(StandardColorf::lemonchiffon());
	Colorf Colorf::lightblue(StandardColorf::lightblue());
	Colorf Colorf::lightcoral(StandardColorf::lightcoral());
	Colorf Colorf::lightcyan(StandardColorf::lightcyan());
	Colorf Colorf::lightgoldenrodyellow(StandardColorf::lightgoldenrodyellow());
	Colorf Colorf::lightgray(StandardColorf::lightgray());
	Colorf Colorf::lightgreen(StandardColorf::lightgreen());
	Colorf Colorf::lightgrey(StandardColorf::lightgrey());
	Colorf Colorf::lightpink(StandardColorf::lightpink());
	Colorf Colorf::lightsalmon(StandardColorf::lightsalmon());
	Colorf Colorf::lightseagreen(StandardColorf::lightseagreen());
	Colorf Colorf::lightskyblue(StandardColorf::lightskyblue());
	Colorf Colorf::lightslategray(StandardColorf::lightslategray());
	Colorf Colorf::lightslategrey(StandardColorf::lightslategrey());
	Colorf Colorf::lightsteelblue(StandardColorf::lightsteelblue());
	Colorf Colorf::lightyellow(StandardColorf::lightyellow());
	Colorf Colorf::lime(StandardColorf::lime());
	Colorf Colorf::limegreen(StandardColorf::limegreen());
	Colorf Colorf::linen(StandardColorf::linen());
	Colorf Colorf::magenta(StandardColorf::magenta());
	Colorf Colorf::maroon(StandardColorf::maroon());
	Colorf Colorf::mediumaquamarine(StandardColorf::mediumaquamarine());
	Colorf Colorf::mediumblue(StandardColorf::mediumblue());
	Colorf Colorf::mediumorchid(StandardColorf::mediumorchid());
	Colorf Colorf::mediumpurple(StandardColorf::mediumpurple());
	Colorf Colorf::mediumseagreen(StandardColorf::mediumseagreen());
	Colorf Colorf::mediumslateblue(StandardColorf::mediumslateblue());
	Colorf Colorf::mediumspringgreen(StandardColorf::mediumspringgreen());
	Colorf Colorf::mediumturquoise(StandardColorf::mediumturquoise());
	Colorf Colorf::mediumvioletred(StandardColorf::mediumvioletred());
	Colorf Colorf::midnightblue(StandardColorf::midnightblue());
	Colorf Colorf::mintcream(StandardColorf::mintcream());
	Colorf Colorf::mistyrose(StandardColorf::mistyrose());
	Colorf Colorf::moccasin(StandardColorf::moccasin());
	Colorf Colorf::navajowhite(StandardColorf::navajowhite());
	Colorf Colorf::navy(StandardColorf::navy());
	Colorf Colorf::oldlace(StandardColorf::oldlace());
	Colorf Colorf::olive(StandardColorf::olive());
	Colorf Colorf::olivedrab(StandardColorf::olivedrab());
	Colorf Colorf::orange(StandardColorf::orange());
	Colorf Colorf::orangered(StandardColorf::orangered());
	Colorf Colorf::orchid(StandardColorf::orchid());
	Colorf Colorf::palegoldenrod(StandardColorf::palegoldenrod());
	Colorf Colorf::palegreen(StandardColorf::palegreen());
	Colorf Colorf::paleturquoise(StandardColorf::paleturquoise());
	Colorf Colorf::palevioletred(StandardColorf::palevioletred());
	Colorf Colorf::papayawhip(StandardColorf::papayawhip());
	Colorf Colorf::peachpuff(StandardColorf::peachpuff());
	Colorf Colorf::peru(StandardColorf::peru());
	Colorf Colorf::pink(StandardColorf::pink());
	Colorf Colorf::plum(StandardColorf::plum());
	Colorf Colorf::powderblue(StandardColorf::powderblue());
	Colorf Colorf::purple(StandardColorf::purple());
	Colorf Colorf::red(StandardColorf::red());
	Colorf Colorf::rosybrown(StandardColorf::rosybrown());
	Colorf Colorf::royalblue(StandardColorf::royalblue());
	Colorf Colorf::saddlebrown(StandardColorf::saddlebrown());
	Colorf Colorf::salmon(StandardColorf::salmon());
	Colorf Colorf::sandybrown(StandardColorf::sandybrown());
	Colorf Colorf::seagreen(StandardColorf::seagreen());
	Colorf Colorf::seashell(StandardColorf::seashell());
	Colorf Colorf::sienna(StandardColorf::sienna());
	Colorf Colorf::silver(StandardColorf::silver());
	Colorf Colorf::skyblue(StandardColorf::skyblue());
	Colorf Colorf::slateblue(StandardColorf::slateblue());
	Colorf Colorf::slategray(StandardColorf::slategray());
	Colorf Colorf::slategrey(StandardColorf::slategrey());
	Colorf Colorf::snow(StandardColorf::snow());
	Colorf Colorf::springgreen(StandardColorf::springgreen());
	Colorf Colorf::steelblue(StandardColorf::steelblue());
	Colorf Colorf::tan(StandardColorf::tan());
	Colorf Colorf::teal(StandardColorf::teal());
	Colorf Colorf::thistle(StandardColorf::thistle());
	Colorf Colorf::tomato(StandardColorf::tomato());
	Colorf Colorf::transparent(StandardColorf::transparent());
	Colorf Colorf::turquoise(StandardColorf::turquoise());
	Colorf Colorf::violet(StandardColorf::violet());
	Colorf Colorf::wheat(StandardColorf::wheat());
	Colorf Colorf::white(StandardColorf::white());
	Colorf Colorf::whitesmoke(StandardColorf::whitesmoke());
	Colorf Colorf::yellow(StandardColorf::yellow());
	Colorf Colorf::yellowgreen(StandardColorf::yellowgreen());

	Colorf Colorf::gray10(StandardColorf::gray10());
	Colorf Colorf::gray20(StandardColorf::gray20());
	Colorf Colorf::gray30(StandardColorf::gray30());
	Colorf Colorf::gray40(StandardColorf::gray40());
	Colorf Colorf::gray50(StandardColorf::gray50());
	Colorf Colorf::gray60(StandardColorf::gray60());
	Colorf Colorf::gray70(StandardColorf::gray70());
	Colorf Colorf::gray80(StandardColorf::gray80());
	Colorf Colorf::gray90(StandardColorf::gray90());
}

