//fd.cfitsiokdb module: native functions are loaded from clib (clib.li64.so) via kexport
clib:use`.clib;
listHDUs:clib.listHDUs;
getFitsRowCount:clib.num_rows;
getFitsColCount:clib.num_cols;
getFitsColNames:clib.cols;
getFitsColType:clib.getColType;
readLongCol:clib.readLongCol;
readDoubleCol:clib.readDoubleCol;
readIntCol:clib.readIntCol;
readSymCol:clib.readSymCol;

///
// createTab creates a table of astronomy data from a given fits file, the hdu to reference and the number of lines to load
// @param f fits file - symbol
// @param n number of rows to load into kdb - long/int
// @param hn number of the hdu in the fits file f that the data to be retrieved is stored under - long/int
// example load 500 rows from table in hdu 2 in file example.fits
// q)createTab[`:example.fits;500;2]
createTab:{[f;n;hn]
  // Get number of cols in table
  nc:getFitsColCount[f;hn];
  // If n=0 Get number of rows in table.
  if[n=0;n:getFitsRowCount[f;hn]];
  // Get list of column names in table
  c:getFitsColNames[f;hn];
  // Get list of column types in table
  t:getFitsColType[f;;hn]each c;
  if[any not t in `J`F`S;' "Unsupported FITS column type"];
  iCols:()!();
  jCols:()!();
  fCols:()!();
  sCols:()!();
  // Get int cols
  iCols:iCols!readIntCol[f;;n;hn]each iCols:c where t=`I;
  // Get long cols
  jCols:jCols!readLongCol[f;;n;hn]each jCols:c where t=`J;
  // Get float cols
  fCols:fCols!readDoubleCol[f;;n;hn]each fCols:c where t=`F;
  // Get text cols
  sCols:sCols!readSymCol[f;;n;hn]each sCols:c where t=`S;
  // Combine all into table
  flip raze (iCols;jCols;fCols;sCols)
 }

export:([listHDUs;getFitsRowCount;getFitsColCount;getFitsColNames;getFitsColType;readLongCol;readDoubleCol;readIntCol;readSymCol;createTab]);