# kdb-x module for mapping astrophysics fits files to kdb tables
Created using the existing fitsToKdb repo from: https://github.com/jpneill/fitsToKdb <br>
./include/k.h can be found here: https://github.com/KxSystems/kdb/blob/master/c/c/k.h

Both slightly modified to work better with a given example.

# Install using qmamba

!!! note Building artefacts
     More information about how to use qmamba and build conda artefacts can be found on the [qmamba wiki](https://github.com/KxSystems/qmamba/wiki/Build)

Build the artefacts:
```bash
micromamba create --name build
micromamba activate build
cd recipe
rattler-build build
mkdir /tmp/mychannel
rattler-index -f fs /tmp/mychannel/
rattler-build publish  output/linux-64/fd-cfitsiokdb-*.conda  --to file:///tmp/mychannel
```

Install the module:
```q
q)qmamba:use`kx.qmamba
q)qmamba.create "fd"
q)qmamba.activate "fd"
q) / search for the module, notice the dependency on cfitsio
q)qmamba.search `SPECS`CHANNEL`PRETTY!(enlist"fd*";enlist"/tmp/mychannel/";1)
Getting repodata from channels...

file:///tmp/mychannel/linux-64                      ??.?MB @  ??.?MB/s  0.0s
file:///tmp/mychannel/noarch                        ??.?MB @  ??.?MB/s  0.0s
     fd-cfitsiokdb 0.1.0 hb0f4dca_0
────────────────────────────────────────

 Name            fd-cfitsiokdb
 Version         0.1.0
 Build           hb0f4dca_0
 Size            13 kB
 License         LicenseRef-Unspecified
 Subdir          linux-64
 File Name       fd-cfitsiokdb-0.1.0-hb0f4dca_0.conda
 URL             file:///tmp/mychannel/linux-64/fd-cfitsiokdb-0.1.0-hb0f4dca_0.conda
 MD5             2f2201210d9ad0f465f4dc9f3934bdf6
 SHA256          a91a1d0fdfe3e137764b29ea8783c25803c6ead25e8e09cbd38e3ab07a0dc391

 Dependencies:
  - cfitsio
  - libgcc >=16
  - cfitsio >=4.7.0,<4.7.1.0a0

q)/ include conda-forge in the channels to pick up cfitsio
q)qmamba.install `SPECS`CHANNEL!(enlist"fd-cfitsiokdb";("/tmp/mychannel/";"conda-forge"))
q)
q)/ use the module
q)cfitsio:use`fd.cfitsiokdb
q)cfitsio
listHDUs       | code
getFitsRowCount| code
getFitsColCount| code
getFitsColNames| code
getFitsColType | code
readLongCol    | code
readDoubleCol  | code
readIntCol     | code
readSymCol     | code
createTab      | {[f;n;hn]
  // Get number of cols in table
  nc:getFitsColCount[f;hn];
  // If n=0..s
```

# Install from source

## CFITSIO library
fitsToKdb also requires the use of the CFITSIO library found here:
https://heasarc.gsfc.nasa.gov/docs/software/fitsio/ <br>
For installing the library, Copy and run:
<pre>
apt update
apt install build-essential pkg-config libcfitsio-dev
</pre>
Verify the install:
<pre>
pkg-config --modversion cfitsio
pkg-config --cflags --libs cfitsio
</pre>

### Optional - if a new install is needed for clib.li64.so
The module loads its native library with `use`.clib`, so the build output must be named `clib.li64.so` (x86-64 Linux).
<pre>
gcc -O2 -fPIC -shared \
  $(pkg-config --cflags cfitsio) \
  -I./include \
  fitsToKdb.c \
  -o clib.li64.so \
  $(pkg-config --libs cfitsio)
</pre>

### Installing the module
Copy `init.q` and `clib.li64.so` into a `fd/cfitsiokdb` directory on your `QPATH`, for example:
<pre>
mkdir -p ~/.kx/mod/fd/cfitsiokdb
cp init.q clib.li64.so ~/.kx/mod/fd/cfitsiokdb/
</pre>
Then load it in q with ``use`fd.cfitsiokdb``.

# Example
Examples can be found here https://www.sdss4.org/

Model schema description sample can be found here https://data.sdss.org/datamodel/files/SPECTRO_REDUX/specObj.html

To import the module and build the table:
<pre>
//To read in 500 rows from the second chunk(HDU) of data
q)fitsToKdb:use`fd.cfitsiokdb
q)tab:fitsToKdb.createPartialTab[`$"example/example1.fits";500;2]
q)count tab
500

//to get the full tab
q)fitsToKdb:use`fd.cfitsiokdb
q)tab:fitsToKdb.createTab[`$"example/example1.fits";0;2]
q)count tab
1843200
</pre>

### Note on HDUs
Typically the first HDU in these types of files is an image, which is outside the scope of analysis for this module.

As such, the below error will appear 
<pre>
q)fitsToKdb:use`fd.cfitsiokdb
q)tab:fitsToKdb.createTab[`$"example/example1.fits";500;1]
'Unable to open FITS table
</pre>
