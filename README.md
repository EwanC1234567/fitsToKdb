# This kdb-x module for mapping astroPhysics fits files to kdb tables
Created using the existing fitsToKdb repo from: https://github.com/jpneill/fitsToKdb <br>
./include/k.h can be found here: https://github.com/KxSystems/kdb/blob/master/c/c/k.h

Both slightly modified to work better with a given example.

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

### Optional - if a new install is needed for fitsToKdb.so
<pre>
gcc -O2 -fPIC -shared \
  $(pkg-config --cflags cfitsio) \
  -I./include \
  fitsToKdb.c \
  -o fitsToKdb.so \
  $(pkg-config --libs cfitsio)
</pre>

# Example
A modern example of data was downloaded from https://www.sdss4.org/

Model schema description can be found here https://data.sdss.org/datamodel/files/SPECTRO_REDUX/specObj.html

To import the module and run the example:
<pre>
//To read in 500 rows from the second chunk(HDU) of data
q)fitsToKdb:use`kx.fitsToKdb
q)tab:fitsToKdb.createPartialTab[`$"example/example1.fits";500;2]
q)count tab
500

//to get the full tab
q)fitsToKdb:use`kx.fitsToKdb
q)tab:fitsToKdb.createTab[`$"example/example1.fits";0;2]
q)count tab
1843200
</pre>

### Note on HDUs
Typically the first HDU in these types of files is an image, which is outside the scope of analysis for this module.

As such, the below error will appear 
<pre>
q)fitsToKdb:use`kx.fitsToKdb
q)tab:fitsToKdb.createTab[`$"example/example1.fits";500;1]
'Unable to open FITS table
</pre>
