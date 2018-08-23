	Directions for making changes to qbsolv man page

The qbsolv man page is maintained as an OpenOffice .odt file formatted to the
classical *nix 'man' page format.  Any addition to the qbsolv command line
arguments need to be documented both in the qbsolv/src/main.c:print_help
function and in the qbsolv.odt file.  (Perhaps a clever person can find a way
to do this without needing to make changes in multiple places.)

Steps:
1) Modify the qbsolv.odt file as needed.
2) While still in OpenOffice, Export the contents as both PDF and HTML.
3) Edit the qbsolv.html file, and in the section of "<META NAME=...>" lines 
at the top, add the line
  <meta name="keywords" content="man qbsolv,qbsolv man page" />
We believe this will make the instance of this file on github.com found by
doing a web search for "man qbsolv".
4) Commit the changes to qbsolv.{odt,pdf,html}.
