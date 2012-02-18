-- Variable Protection --

### Author ###
Vlad Jimenez (allejo)
email: allejo.bzflag@gmail.com
irc:   allejo

### License ###
BSD

### Description ###
The plugin will reset any variable that you choose to be locked. This is to
prevent users from changing dangerous settings and risk crashing the server
such as _gravity. This plugin is ideal for servers where anyone can use the
/set command.

### Slash Commands ###
/listupdate

### How To Use ###
To load the plugin you must load it with a conf file listing the variables
you want to be protected. The syntax of loading the plugin is as follows:

-loadplugin /path/to/varProtection.so,/path/to/lockedVars.txt

Each variable needs to be put with an '_' and on a separate line. In order
for the plugin to work correctly, the variable must be spelled correctly
because the plugin IS case sensitive. The format for the conf file is as
follows:

_gravity
_wingsGravity
_jumpVelocity

### More Info ###
For more information, please visit: https://github.com/allejo/varProtection