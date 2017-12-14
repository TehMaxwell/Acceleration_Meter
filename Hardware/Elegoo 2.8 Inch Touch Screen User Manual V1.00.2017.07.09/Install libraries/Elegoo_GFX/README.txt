This is the core graphics library for all our displays, providing a common set of graphics primitives (points, lines, circles, etc.).  It needs to be paired with a hardware-specific library for each display device we carry (to handle the lower-level functions).

Elegoo invests time and resources providing this open source code, please support Elegoo and open-source hardware by purchasing products from Elegoo!

Written by Limor Fried/Ladyada for Elegoo Industries.
BSD license, check license.txt for more information.
All text above must be included in any redistribution.

To download, click the DOWNLOAD ZIP button, uncompress and rename the uncompressed folder Elegoo_GFX. Confirm that the Elegoo_GFX folder contains Elegoo_GFX.cpp and Elegoo_GFX.h

Place the Elegoo_GFX library folder your <arduinosketchfolder>/Libraries/ folder. You may need to create the Libraries subfolder if its your first library. Restart the IDE.

Useful Resources
================

-  Image2Code
   This is a handy Java GUI utility to convert a BMP file into the array code necessary to display the image with the drawBitmap function.  Check out the code at ehubin's GitHub repository:
     https://github.com/ehubin/Elegoo-GFX-Library/tree/master/Img2Code

-  drawXBitmap function
   You can use the GIMP photo editor to save a .xbm file and use the array saved in the file to draw a bitmap with the drawXBitmap function. See the pull request here for more details:
     https://github.com/Elegoo/Elegoo-GFX-Library/pull/31
