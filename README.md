# infworld

This is a little OpenGL demo that displays an infinite world generated with
perlin noise.

## usage

```
./infworld -r|--range [number] -s|--seed [number]
```

The seed is a 32 bit integer that will be fed into the perlin noise function
to generate a unique world. By default, the seed is randomly chosen.

The range is an integer between 3 and 64 that will be the applications
"render distance" in terms of how many chunks the application will attempt
to render. The default it is 10 though you can set it to a higher number if you
desire, be warned that setting it to a higher value will lead to lower
performance.

## compile

Dependencies:
[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) |
[glad](https://github.com/Dav1dde/glad) |
[glfw](https://github.com/glfw/glfw) |
[glm](https://github.com/g-truc/glm)

Linux:

```
make -j$(nproc)
./infworld
```

Windows:

Use [mingw-w64](https://sourceforge.net/projects/mingw-w64) to compile:

```
mingw32-make -j$(nproc)
.\infworld.exe
```

## CREDITS

### textures
[grass.png](https://opengameart.org/content/seamless-grass-texture)

[sand.png](https://opengameart.org/content/seamless-beach-sand)

[stone.png](https://opengameart.org/node/15623)

[snow.png](https://opengameart.org/content/seamless-snow-texture-0)

[waternormal1.png & waternormal2.png](https://watersimulation.tumblr.com/post/115928250077/scrolling-normal-maps)

[waterdudv.png](https://www.dropbox.com/sh/lwvm5i223cwd5ue/AADedi_y3XTQ_j2aD2oH4DLKa?dl=0)
(linked from this [wonderful video by ThinMatrix](https://www.youtube.com/watch?v=7T5o4vZXAvI))

### skybox
downloaded from [https://opengameart.org/content/sky-box-sunny-day](https://opengameart.org/content/sky-box-sunny-day)

by KIIRA

License: [CC-BY 3.0](https://creativecommons.org/licenses/by/3.0/)
