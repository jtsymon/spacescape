spacescape
==========

Wallpaper generator

Inspired by http://wwwtyro.github.io/procedural.js/space/

Uses GLSL to generate large images quickly, with the intention of generating wallpapers (tests on my machine took about 1ms to generate a 3840x1080 image)

Probably poorly coded as I've been learning OpenGL / GLSL along the way

==========
###Usage:

There's no commandline options yet, but if you build and run it,

key    | action
-------|------------------------
space  | generate a new image
s      | save the image as a PNG file
escape | exit
--------------------------------

==========
###Example images:

* http://i.imgur.com/tjmbSdL.png
* http://i.imgur.com/yuasdnY.png
* http://i.imgur.com/8hdpa5q.png
* http://i.imgur.com/qMls9cT.png
* http://i.imgur.com/auJYuCh.png
* http://i.imgur.com/3d5Zovr.png

Old:
* http://i.imgur.com/nHVUcMD.png
* http://i.imgur.com/Eeqzjv0.png
* http://i.imgur.com/PM1vID0.png
* http://i.imgur.com/2UkWn9K.png

==========

Currently generates stars and suns similarly to wwwtyro's, but the nebula generation is completely different (and in most cases not as nice looking), because GLSL doesn't allow recursion

The result of a failed attempt at writing an iterative version of wwwtyro's nebula generation function: http://i.imgur.com/CvMQHp3.png

==========

###TODO:

* Find a way to implement wwwtyro's nebula algorithm (or any algorithm which generates nice looking clouds) in GLSL
* Add an option to run it headless and output an image file
