# ccaptcha

ccaptcha is a 2ch.ru captcha-lookalike captcha renderer written in pure C.
Requires [libgd](https://bitbucket.org/libgd/gd-libgd/) to build and run.

## Usage

Pass lines one by one to the ccaptcha's stdin. After each '\n' character,
read its stdout for a PNG image.

An example:

    $ echo "sometext" | ./captcha > /tmp/cap.png

### Command-line options:
```
--captcha-safe-w        Safe drawing area width and height. The default
--captcha-safe-h        safe area size is 400x100 pixels which should be
                        (more than) enough for most cases.
--captcha-bg            Background and foreground colours in RRGGBB form.
--captcha-fg            The default background and foreground colours are
                        EEEEEE and 222222 respectively.
--captcha-sz            Text height in pixels. Default is 40.
--captcha-lw            Line width in pixels. Default is 2.
--captcha-nlines-min    Minimal and maximal number of curves over the
--captcha-nlines-max    image. Defaults are 2/3.
--captcha-dist          Maximal distortion ratio. Default is 0.1, with
                        something over 0.2 being barely acceptable.
--bezier-steps          More = nicer curves and slower. Defaults to 16.
--letters               Letters specification file.
                        Defaults to ./letters.txt.
```

## Known bugs

Will segfault if you try to run without letters.txt.

## TODO
  - Nothing [DONE]

## License
[GPLv2](http://www.gnu.org/licenses/gpl-2.0.html)
