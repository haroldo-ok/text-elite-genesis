# Text Elite

    Text Elite is a C implementation of the classic Elite trading system in a text adventure style shell. I originally coded this to formalise and archive the definition of the Classic Elite Universe, but have released it, with sources, now that [Christian Pinder](http://home.clara.net/cjpinder/elite.html) has publicly reverse engineered BBC Disk Elite trading into C.

Text Elite is currently trading only. There is no risk of misjump, police, or pirate attack.

Note that Text Elite Galactic Hyperspace jumps to the system in the target Galaxy having the same position in the Gaslactic Generation sequence as the planet jumped from. This is a deviation from Classic 6502 Elite, which alwsys jumped to a common "central" system (nearest (#0x60,#x60)).

[Click here to download Text Elite 1.5](http://www.elitehomepage.org/archive/a/b9101315.zip) (35 Kbytes) Includes C sources.\
    Ver 1.5 further fixes to Goat Soup string.\
    Ver 1.4 introduces some minor bugfixes relating to the Goat Soup string.\
    Ver 1.3 introduces rand command to instant a platform independent random number generator inplace of ANSI C's rand().\
    Ver 1.2 introduces Goat Soup string.

An [Amiga version](http://wuarchive.wustl.edu/aminet/dirs/aminet/game/text/TextElite.lha) is available on aminet.

    You can send scripts to txtelite.exe with a command such as txtelite < spears.txt but take care to ensure that a final "quit" is present in the file. The drug- and fur-running Commander [Spears](http://www.iancgbell.clara.net/elite/text/spears.txt) submitted by Christian Pinder, achieves 5016 CR in just 23 commands. Another wicked peddlar of alien narctoics, Commander [Sinclair](http://www.iancgbell.clara.net/elite/text/sinclair.txt) , submitted by Duane McDonnell, achieves 1001201 CR with just 632 hyperspace jumps.

    McDonnell has made the treesearch program he used to generate this (suboptimal) script available; it might proove useful for nonplayer trader AI and player help.

[Click here to download Mkscript 1.1](http://www.elitehomepage.org/archive/a/c1000011.zip) (71 Kbytes) Includes C sources.

Original version: http://www.iancgbell.clara.net/elite/text/index.htm