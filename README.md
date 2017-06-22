# top processes
Displays Linux' top processes according to their *current* CPU or memory usage during the given sampling time.

## Dependencies
[libprocps](https://gitlab.com/procps-ng/procps) (can be dropped with a little more work)

## Example
```
$ ./top_proc_example
PID        <CPU>      RSS        CMD       
3235       5.91       8876       conky     
3359       5.91       45948      mpd       
4784       3.94       694840     firefox-esr
9317       3.94       2648       top_proc_exampl
3060       1.97       386316     gnome-shell

PID        CPU        <RSS>      CMD       
4784       3.94       694840     firefox-esr
3060       1.97       386316     gnome-shell
4878       0.00       91284      Telegram  
2346       0.00       87108      Xorg      
2437       0.00       85052      spamd child
```

## License
[GNU General Public License v2.0](https://www.gnu.org/licenses/gpl-2.0.txt) (might change to CC0 in the future after dropping libprocps dependency)
(c) [Alexander Heinlein](http://choerbaert.org)
