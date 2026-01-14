



# The Nightmare "Magical" MPEG Assets

I am using the term "magical" to describe these MPEG files.

For whatever reason, most ReelMagic game MPEG video asset files have a reserved
`picture_rate` code (0xC and 0xD seen so far) in the MPEG sequence header...
according to ISO/IEC 11172-2, anything above 0x8 is a "reserved" value. Looking
at the PTS values in the Return to Zork intro video "FINTRO01.MPG", the actual
framerate is roughly 30 fps. Therefore, the expected value for this asset would be
0x5, not 0xD. However, just overwriting/forcing this value to 0x5 and playing the
video using stock PLMPEG, VLC, or another media player yields a terribly corrupt
and completely trashed video. In-depth analysis of the video shows decompression
of bitstream data fails/misaligns on only P and B picture slices that contain
macroblocks with motion vectors. Since the `motion*_r` values are variable-length 
bit fields dictated by the picture `f_code` sizes, if the `f_code` is
not the exact value used at the time of encoding/compression, decompression
during motion vector VLC table lookup will completely screw up the state of
the decoder as garbage data is being decompressed.

I discovered that hardcoding an `f_code` of some value (`motion*_r` bitfield
sizes inherintly now hardcoded to some value -1) for both forward and backward
corrects the motion vector VLC table/decompression problems.


The moving `f_code` values in the picture header appear to be related to the
temporal sequence number. P and B picture with a temporal sequence number of
either 3 or 8 seem to contain a truthful `f_code` value for Return to Zork
and Lord of the Rings assets. P and B picture with a temporal sequence number
of 4 seem to contain a truthful `f_code` for The Horde, however, these assets
also contain empty "user data" between the picture and slice.




Facts:
  * Only have currently seen this on muxed (MPEG-1 PS) files.
  * Magical assets can be identified by having a `picture_rate` code of 0xC or 0xD
  * Normal assets having a standards-compliant `picture_rate` code, such as the "SIGMA.MPG" file, decode just fine with any standard MPEG-1 compliant decoder/player.
  * A static `f_code` of 4 seems to work for most of the Return to Zork magical MPEG asset files.
    * Exceptions: `FENDING0.MPG` wants 3 and `FFBTRD01.MPG` wants 1
  * A static `f_code` of 2 seems to work for most of the Lord of the Rings magical MPEG asset files.
  * This behavior is impacted by a `driver_call(9, X, 0208h, ...)` call. (See `RMDOS_API.md`)


## Open Questions

* Can the `f_code` update per picture as with standard MPEG-1 video, or is it limited to one static forward/backward static value pair per MPEG file/sequence?
* What is the exact relationship between temporal sequence number and `f_code`?
* How exactly does the `driver_call(9, X, 0208h, ...)` (aka. "Magic Key") call directly impact this?

## Current Emulator Workaround

The emulator takes uses `driver_call(9, X, 0208h, ...)` (aka "Set Magic Key") to determine where to find a
truthful `f_code` value which gets applied to the entire MPEG asset when a "magical" `picture_rate` code
is specified. The MPEG asset file is quickly scrubbed at media handle open time to find a matchiing P or B
picture containing a truthful `f_code` value. Then this value is statically applied to the entire decoding
of said MPEG asset file.

Eventually, I think this needs to be handled on a per-picture basis.

Known "magic key" values:

* 0x40044041 -- Default value. Truthful `f_code` found in temporal sequence number of 3 or 8.
* 0xC39D7088 -- Used in The Horde. Truthful `f_code` found in temporal sequence number of 4.


## Analyzing and Inspecting These Files

There are a handful of programs I have included in the `tools/` directory that can
help with analyzing and debugging these files. Just hit `make` in that directory to
build everything.


### Detection

To detect a magical asset file, the `is_magical_asset` tool can be used.
For example:
```
  $ ./is_magical_asset ../FINTRO01.MPG
  Magical MPEG-1 PS asset detected. Frame rate code=0xD
  $ ./is_magical_asset ../FMPVSS00.MPG
  Normal MPEG-1 ES asset detected. Frame rate code=0x5
  $ ./is_magical_asset ../SIGMA.MPG
  Normal MPEG-1 PS asset detected. Frame rate code=0x4
```

Note: The `is_magical_asset` tool will only exit with a code of 0 if it detects that the file is a magical asset.



### Conversion

To retrieve a static `f_code` value to play a magical asset, the
`find_magical_f_code` tool can be used.
For example:
```
  $ ./find_magical_f_code ../FINTRO01.MPG
  Found f_code: 4
```

To convert a magical asset file to be playable in a standard MPEG-1 player, the
`unlock_the_magic_mpeg_ps` tool can be used. The `f_code` value that was discovered
in the previous call to `find_magical_f_code` must be passed as the first parameter
to this tool.
For example:

```
  $ ./unlock_the_magic_mpeg_ps 4 ../FINTRO01.MPG output.mpg
```

The above example uses the `f_code` value of 4 obtained from the previous example
call to `find_magical_f_code` and creates a new file called `output.mpg` which
should be playable in any MPEG-1 compliant media player.

Testing our "unlocked" file using these tools shows:
```
  $ ./is_magical_asset output.mpg
  Normal MPEG-1 PS asset detected. Frame rate code=0x5
```

### Further Analysis

Another tool is provided (MPEG PS Only) which can be used to dump and compare all
P and B picture temporal sequence numbers and their `f_code` values.
For example:
```
  $ ./superanalyze_mpeg_ps_f_code.pl ../FINTRO01.MPG 2>/dev/null
  tsn=03 ffcode=4
  tsn=01 ffcode=6
  tsn=02 ffcode=3
  tsn=06 ffcode=7
  tsn=04 ffcode=2
  tsn=05 ffcode=3
  tsn=09 ffcode=5
  tsn=07 ffcode=1
  tsn=08 ffcode=4
  tsn=00 ffcode=5
  tsn=01 ffcode=6
  tsn=05 ffcode=3
  tsn=03 ffcode=4
  tsn=04 ffcode=2
  tsn=08 ffcode=4
  ...
```

A target `f_code` value can even be provided to this tool so that it prints the
deltas. For example:
```
  $ ./superanalyze_mpeg_ps_f_code.pl ../FINTRO01.MPG 4 2>/dev/null | head -n 30
  tsn=03 ffcode=4 delta=0
  tsn=01 ffcode=6 delta=-2
  tsn=02 ffcode=3 delta=1
  tsn=06 ffcode=7 delta=-3
  tsn=04 ffcode=2 delta=2
  tsn=05 ffcode=3 delta=1
  tsn=09 ffcode=5 delta=-1
  tsn=07 ffcode=1 delta=3
  tsn=08 ffcode=4 delta=0
  tsn=00 ffcode=5 delta=-1
  tsn=01 ffcode=6 delta=-2
  tsn=05 ffcode=3 delta=1
  tsn=03 ffcode=4 delta=0
  tsn=04 ffcode=2 delta=2
  tsn=08 ffcode=4 delta=0
  ...
```

A delta of 0 shows us when we get an `f_code` value that matches the targeted
value (4 in the above example) we provided the tool. As you can see, temporal
sequence numbers of `03` and `08` yield matching `f_code` values.


