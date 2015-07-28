#!/bin/sh

export OUTDIR=/tmp/midifilters/
export KEEPOUTDIR=1

lv2ls \
	| grep gareus \
	| grep midifilter \
	| ~/data/coding/lv2toweb/tools/create_doc.sh

CUTLN=$(cat -n $OUTDIR/index.html | grep '<!-- start of page !-->' | cut -f 1 | tr -d ' ')
# TODO test if $CUTLN is an integer

head -n $CUTLN $OUTDIR/index.html > $OUTDIR/index.html_1

cat >> $OUTDIR/index.html_1 << EOF
<h1>Midifilter.lv2</h1>
<img src="midifiltres.png" alt="Midifilter Logo" style="float:left; width: 100px;"/>
<div style="margin-left:100px;">
<p><a href="https://github.com/x42/midifilter.lv2">Midifilter.lv2</a> - a collection of MIDI data filters in LV2 plugin format.</p>
<ul>
<li>One MIDI input, one MIDI output (no generators, no synths)</li>
<li>No custom GUI, control inputs only</li>
<li>Every control can be automated, the plugins handle dynamic parameter changes</li>
<li>All plugins report their latency to the host (for most of them it is zero)</li>
<li>DRY (Don't Repeat Yourself principle) - simple filters that can be combined in a network</li>
<li>Self-documenting: Built-in description and control-port documentation</li>
</ul>
</div>
EOF

tail -n +$CUTLN $OUTDIR/index.html >> $OUTDIR/index.html_1
mv $OUTDIR/index.html_1 $OUTDIR/index.html

rm doc/http__*
cp -a $OUTDIR/*.html doc/
cp -a $OUTDIR/*.png doc/

grep '<tr><td class="first center">' doc/index.html \
	| sed 's/ class="[^"]*"//g;s/<a href="[^"]*"[^>]*>//g;s/<\/a>//g;s/<td>Filter<\/td>//;s/<td>[0-9]\/[0-9]\/[0-9]*<\/td>//g;s/<td>http:[^>]*>//' \
	> /tmp/x42-midifilter.inc.html

echo -n "git add+push doc? [y/N] "
read -n1 a
if test "$a" != "y" -a "$a" != "Y"; then
	exit
fi

cd doc && git add *.html *.png && git commit -m "update documentation" && git push
