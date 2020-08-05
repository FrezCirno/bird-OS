// (c) 2009, 2010 Lutz Sammer, License: AGPLv3
#pragma once
#include <types.h>

	/// bitmap font structure
struct bitmap_font {
	u8 Width;		///< max. character width
	u8 Height;		///< character height
	u16 Chars;		///< number of characters in font
	const u8 *Widths;	///< width of each character
	const u16 *Index;	///< encoding to character index
	const u8 *Bitmap;	///< bitmap of all characters
};

extern const struct bitmap_font fonts;