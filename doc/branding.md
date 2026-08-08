Branding
========

The myLinux identity is a 1970s UNIX logotype: the wordmark over a four
stripe slab.  It is purely typographic, there is no mascot.  The penguin
lives in `board/common/rootfs/etc/motd`, where it greets you on boot.


Palette
-------

The colours come from a classic UNIX t-shirt, sampled off the fabric
rather than picked by eye.

| Role      | Hex       | Used for                                     |
|-----------|-----------|----------------------------------------------|
| cream     | `#EFE4D4` | wordmark on dark grounds                     |
| yellow    | `#F1CE36` | `my` on dark grounds, first stripe           |
| orange    | `#E98744` | second stripe                                |
| red       | `#DC4A44` | third stripe                                 |
| deep red  | `#8E3A34` | fourth stripe                                |
| deep grey | `#413B38` | the plate, and the social preview ground     |
| ink       | `#221E1C` | wordmark on light grounds                    |
| my/light  | `#B0362B` | `my` on light grounds                        |
| ok green  | `#9BD8A8` | `[ OK ]` boot status                         |

Two rules matter more than the rest:

**Yellow is for dark grounds only.**  On white it lands at 1.54:1 and is
unreadable.  Light grounds use `#B0362B` for `my` instead, which is
4.90:1 on cream.

**Yellow is the brand accent and must never also mean "warning."**  When
the Clixon WebUI arrives it needs its own warning and critical hues,
deliberately unlike the accent, or every screen reads as alarmed.  Green
is already spoken for by `[ OK ]` and "interface up."

The t-shirt's own warm grey `#756A69` is not used on screen.  Cream on it
is 4.16:1 and yellow on it 3.39:1, both under WCAG AA.  Dropping the
ground to `#413B38` takes those to 8.76:1 and 7.14:1.


Typography
----------

Lato Black, tracking -0.01em, **converted to outlines**.  No SVG here
depends on a font being installed, and none should: do not re-typeset the
wordmark with live text.

To regenerate it you need `fontTools` and Lato Black.  The outlines in
`doc/logo.svg` are the source of truth, so ordinary edits need neither.


Geometry
--------

Everything is proportional to the type size, so the logo scales cleanly.

| Measure         | Value               |
|-----------------|---------------------|
| stripe bar width| the exact ink width of the word |
| stripe bar height | 0.42 × type size  |
| gap, word to bar| 0.16 × type size    |
| padding         | 0.29 × type size    |

The bar is measured to the wordmark's ink, not its advance width, so the
left and right edges line up with the `m` and the `x`.


Files
-----

| Path                          | Purpose                                |
|-------------------------------|----------------------------------------|
| `doc/logo.svg`                | primary: switches light/dark by itself |
| `doc/logo-light.svg` `.png`   | transparent, for light grounds         |
| `doc/logo-dark.svg` `.png`    | transparent, for dark grounds          |
| `doc/logo-plate.svg`          | deep grey plate, works on any ground   |
| `doc/logo.png`                | raster of the plate; the safe default  |
| `doc/mark.svg`                | rounded stripe tile, the icon          |
| `doc/social-preview.png`      | 1280 × 640                             |
| `src/webif/www/favicon.svg`   | rounded tile                           |
| `src/webif/www/favicon.ico`   | 16/32/48                               |

`doc/social-preview.png` has to be uploaded by hand under repository
Settings.  GitHub does not pick it up from the tree.


Light and dark
--------------

Use `doc/logo.svg` and let it sort itself out:

    <img src="doc/logo.svg" alt="myLinux" width="356">

The switching lives in a stylesheet inside the file.  GitHub strips
`<style>` from SVG written inline in markdown, but not from a `.svg`
loaded through `<img>` — there the browser renders it as an ordinary SVG
document and the media query applies.

The stylesheet has three tiers, not two:

| Tier  | `Linux`   | `my`      | Applies when                          |
|-------|-----------|-----------|---------------------------------------|
| base  | `#8E7B6B` | `#C4553F` | the media query is never evaluated     |
| light | `#221E1C` | `#B0362B` | `prefers-color-scheme: light`          |
| dark  | `#EFE4D4` | `#F1CE36` | `prefers-color-scheme: dark`           |

The base tier is the important one.  `prefers-color-scheme` inside an
`<img>` loaded SVG follows the browser or OS setting, not a site's own
theme toggle, so the two can disagree and some renderers skip the query
altogether.  Ink on GitHub's dark ground is 1.14:1, invisible.  The base
colours are a deliberate compromise that clears 4:1 on both white and
GitHub dark, so the fallback is never wrong, only muted.

**Never let the fallback be a mode specific colour.**

Where the ground is known, reach for `logo-light` or `logo-dark` directly
and skip the guessing.


Icon
----

The stripe slab in a rounded tile, corner radius 22%.  It is nothing but
horizontal edges, so it stays crisp down to 16px, and it is the same
object as the bar under the wordmark — icon and logotype are one system.
