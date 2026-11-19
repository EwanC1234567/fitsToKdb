/ Usage: q test.q <fits file> <hdu number>
/ Needs fd.cfitsiokdb on QPATH (e.g. QPATH=$HOME/.kx/mod q test.q ...).
/ Requires a FITS file with a binary table in the given HDU; none is shipped in the repo.
args:.z.x;
if[2>count args;-2 "usage: q test.q <fits file> <hdu>";exit 2];
f:`$first args;
hn:"J"$last args;
m:use`fd.cfitsiokdb;

fail:{[msg] -2 "FAIL: ",msg; exit 1};

/ partial load returns the requested number of rows
t5:m.createTab[f;2;hn];
if[not 2=count t5;fail"partial load returned ",string count t5];
if[not 98h=type t5;fail"createTab did not return a table"];

/ full load returns the row count reported by the file
n:m.getFitsRowCount[f;hn];
tAll:m.createTab[f;0;hn];
if[not n=count tAll;fail"full load returned ",string[count tAll]," rows, expected ",string n];

/ export dictionary lists the public functions
if[not all `createTab`listHDUs in key m;fail"createTab/listHDUs missing from export"];

-1 "PASS";
exit 0;
