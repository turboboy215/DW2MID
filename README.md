# DW2MID
## David Whittaker (GB/GBC) to MIDI converter

This tool converts music from Game Boy and Game Boy Color games using David Whittaker's sound engine to MIDI format.
It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).
For R-Type DX, which contains separate music banks for R-Type I and II, you must run the program multiple times specifying where each different bank is located. However, in order to prevent files from being overwritten, the MIDI files from the previous bank must either be moved to a separate folder or renamed.

Examples:
* DW2MID "R-Type II (U).gb" 7
* DW2MID "Loopz (U).gb" 1
* DW2MID "Alfred Chicken (E) [!].gb" 8

This tool was based on my own reverse-engineering of the sound engine, based on R-Type II.

As usual, there is another program, DW2TXT included, which prints out information about the song data from each game. This is essentially a prototype of DW2MID.

Supported games:
  * Alfred Chicken/Alfred's Adventure
  * Alien 3
  * Alien Olympics 2044 AD
  * The Amazing Spider-Man 2
  * Castelian
  * Chase H.Q.
  * Days of Thunder
  * Faceball 2000
  * The Flintstones
  * Krusty's Fun House
  * The Lion King
  * Loopz
  * Populous
  * Race Drivin'
  * Robin Hood: Prince of Thieves
  * R-Type
  * R-Type II
  * R-Type DX
  * Splitz
  * Terminator 2: Judgment Day
  * Tip Off
  * Xenon 2: Megablast

## To do:
  * Support for other versions of the sound engine (there are many others, including various computers)
  * GBS file support
