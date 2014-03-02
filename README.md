spacescape
==========

Wallpaper generator

Inspired by http://wwwtyro.github.io/procedural.js/space/

Uses GLSL to generate large images quickly, with the intention of generating wallpapers (tests on my machine took about 1ms to generate a 3840x1080 image)

Probably poorly coded as I've been learning OpenGL / GLSL along the way

==========

Currently generates stars and suns similarly to wwwtyro's, but the nebula generation currently is just simplex noise (can't use wwwtyro's recursive function in GLSL, and my attempt at an iterative solution resulted in this: http://i.imgur.com/CvMQHp3.png

If you build and run it you can press space to generate a new image (possibly also leaking memory in the process, I haven't tested that)

==========

Examples of generated images:
* http://i.imgur.com/nHVUcMD.png
* http://i.imgur.com/Eeqzjv0.png
* http://i.imgur.com/PM1vID0.png
* http://i.imgur.com/2UkWn9K.png

(The stars are currently too big and dark, and also the background stars shouldn't be showing through the nebula like that)

==========

TODO:

* Find a way to implement wwwtyro's nebula algorithm (or any algorithm which generates nice looking clouds) in GLSL
* Add an option to run it headless and output an image file
