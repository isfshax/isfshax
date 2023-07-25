### DISCLAIMER: You probably SHOULD NOT INSTALL/USE THIS or other isfshax related repositories
isfshax is incomplete: it has not been extensively tested and does not properly initialize hardware after boot1, leading to SD card stability issues and potentially other problems.  
This release is mostly meant for developers or anyone that has some use for the project in its current state.  
**For most users isfshax will be of little use until its issues are fixed!  
Please make sure you properly understand all the risks involved before attempting to install it!**  
Ensure you have an SLC backup at hand and a safe way to restore it before proceeding.  
Thanks to shinyquagsire123's de_Fuse vulnerability it should now be possible to restore SLC without the need for an SLC hardmod, which should make repairs at least somewhat less troublesome.

# isfshax
isfshax is a coldboot boot1 exploit for the Wii U.  
This repository contains the main isfshax exploit and stage2 payload loader.  
It will produce an (unencrypted) ISFS superblock image, meant to be installed through isfshax_installer or other means.  
The stage2 payload will attempt to load (in order):
  - `sd:/isfshax.bin`
  - `slc:/sys/isfshax.bin`

### Status
Right now I unfortunately do not have much time or motivation to work on the project, and it's been more than two years since I last actively contributed to it.  

However, if you have any use for isfshax feel free to pick it up; contributions are welcome!  
While the boot1 vulnerability is stable, right now SDCard initialization/mounting/loading is unreliable and slow, probably something is not being properly initialized (SLC loading works correctly though).  
Some of the minute changes made to de_Fuse's minute_minute might be of help to fix those issues.  
isfshax superblock repairs have also not been tested extensively.  

### Other developer information
isfshax must be the newest ISFS superblock (with the highest superblock generation number) to be loaded by boot1.
This creates a problem: unpatched IOSU will *also* attempt to load the isfshax superblock. IOS-FS contains the same unchecked recursion bug that makes boot1 isfshax possible (though it's probably not as easily exploitable in IOSU, due to stack checks and memory protection), so the loading attempt will normally result in a crash (which is better than the alternative, IOSU attempting to "repair" the patched block and potentially overwriting real superblocks).  
To avoid an uncontrolled crash and ensure IOSU stops as soon as possible in a somewhat controlled manner, we deliberately insert an FST entry pointing to a 0xFFFC cluster, in order to break an assertion only present in IOSU.  
 
To boot IOSU or other CFWs (including minute) with isfshax installed, ISFS superblock loading requires patches to prevent attempts to load the isfshax superblock (this can usually be archieved by reducing the maximum allowed superblock generation number; in IOSU a very small 4 byte patch in IOS-FS is sufficient, see the iosuhax repository... well, *probably* sufficient at least, none of this was significantly tested so, while it appears to work, it could still fail in unexpected ways).

Another thing to take into account is the (admittedly moderately low) possibility of SLC corruption. Normally, SLC repairs are handled by boot1 or IOSU. This is undesirable for isfshax superblocks, due to the usual risk of accidentally overwriting good superblocks, and the potential for generation number overflows (due to the very high generation numbers used).  

To dissuade boot1 or IOSU from repairs, *all* other superblocks are marked as bad NAND clusters inside of each isfshax superblock (isfshax superblocks are also marked as bad inside of all normal superblocks to ensure they are not accidentally overwritten).  

This leaves to stage2 the burden of actually repairing superblocks, should cluster corruption occur. We also store a number of duplicate copies of isfshax to tackle NAND clusters eventually becoming unrepairable.
 
## Credits
Huge thanks to:
  - GaryOderNichts and ashquarky for their direct contributions and help
  - vgmoose for all the support and for the Wii U that replaced the one I
    destroyed during early isfshax testing
  - Maschell for his help and all other contributions to the Wii U scene
  - hexkyz for the warmboot boot1 exploit which made all of this possible
  - shinyquagsire123 for de_Fuse
  - Salt Team for the original minute CFW
  - dimok789, FIX94 and others for the iosuhax CFW
  - fail0verflow for mini
  - and all other contributors to the Wii U scene!
