spacescape
==========

Wallpaper generator

Inspired by http://wwwtyro.github.io/procedural.js/space/

Uses GLSL to generate large images quickly, with the intention of generating wallpapers (tests on my machine took about 1ms to generate a 3840x1080 image)

Probably poorly coded as I've been learning OpenGL / GLSL along the way

==========
###Usage:

Commandline options:

flag                              | action
----------------------------------|-----------------
-h, --help                        | display this help and exit
-o, --output=\[value\]            | no value  => save 1 image (to \<timestamp\>.png)<br>int value => save $value images<br>str value => save 1 image to filename $value
-s, --size=\<width*height\>       | image size (default = 1920*1080)


Windowed mode:

key    | action
-------|------------------------
space  | generate a new image
s      | save the image as a PNG file
escape | exit

==========
###Example images:

* http://i.imgur.com/pkVYWmW.png
* http://i.imgur.com/Gkt1ww2.jpg
* http://i.imgur.com/N7JMQkS.jpg
* http://i.imgur.com/rH0SiX4.jpg
* http://i.imgur.com/KWXJpo8.jpg
* http://i.imgur.com/raXiI2w.jpg

==========

Currently generates stars and suns similarly to wwwtyro's, but the nebula generation is completely different (and in most cases not as nice looking), because GLSL doesn't allow recursion
