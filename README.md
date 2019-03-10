# Protofall's VWing ".LEV" to PNG Converter

This program will take a ".LEV" file made with the MS-DOS game VWing's level converter and converts it into a Png. The levels are actually 256-colour palette, 640 by 800 pcx files, except the 128-byte header has different content. Because pcx files aren't really supported by modern OSes anymore, I decided to convert them to PNGs instead. I've also included the original MS-DOS game in this repo since its a bit hard to find and it is shareware.

This program won't see much use since almost no one is playing this game anymore, but I remember playing it as a kid and finding it fun and I wanted to see the whole level in their glory.

Dependencies:

+ libpng

Run `make` to compile the program.
Run `./VWing-LEV-To-Png [LEV_filename] [Png_filename]`

