/*
 * * =================================================
 * *       Filename:  fitsToKDB.c
 * *
 * *    Description:  Functions for extracting columns from FITS table HDUs.
 * *                  Returns extracted columns to q as lists
 * *    Unsupported:  Does not support image HDUs or unsupported column types
 * * 
 * *        Version:  1.0
 * *        Created:  2016.11.01 20:23:25
 * *       Revision:  none
 * *       Compiler:  gcc
 * *
 * *         Author:  Mr. James Neill , jneill@kx.com
 * *        Company:  First Derivatives plc, Newry
 * *
 * * =================================================
 * */
#define KXVER 3
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<stdint.h>
#include"fitsio.h"
#include"k.h"

static J q_integer(K x)
{
  return x->t==KJ ? x->j : x->i;
}

static int is_table(int hdutype)
{
  return hdutype==BINARY_TBL || hdutype==ASCII_TBL;
}

static void close_file(fitsfile *fptr, int *status)
{
  int close_status=0;
  fits_close_file(fptr,&close_status);
  if(!*status)
    *status=close_status;
}

static int open_table(fitsfile **fptr, const char *path, int hdu, int *hdutype)
{
  int status=0;
  fits_open_file(fptr,path,READONLY,&status);
  if(status)
    return status;
  fits_movabs_hdu(*fptr,hdu,hdutype,&status);
  if(status || !is_table(*hdutype))
    {
      int close_status=0;
      fits_close_file(*fptr,&close_status);
      return status ? status : BAD_HDU_NUM;
    }
  return 0;
}

/********************************************************************
* Name       :  K listHDUs(K x) 
* Description:  Function which prints a list of the number of HDUs and their types
* Parameters :  x - q symbol corresponding to the fits file name `$"example.fits"
*/
K listHDUs(K x)
{
  fitsfile *fptr;
  int i,hdunum,hdutype,status=0;
  ffopen(&fptr,x->s,READONLY,&status);
  ffthdu(fptr,&hdunum,&status);
  printf("Number of HDUs: %d\n",hdunum);
  for(i=1; i<=hdunum; i++)
    {
      ffmahd(fptr,i,&hdutype,&status);
      switch(hdutype)
        {
        case IMAGE_HDU:
          printf("HDU %d: IMAGE_HDU\n",i);
          break;
        case ASCII_TBL:
          printf("HDU %d: ASCII_TBL\n",i);
          break;
        case BINARY_TBL:
          printf("HDU %d: BINARY_TBL\n",i);
          break;
        }
    }
  return ki(0);
}

/********************************************************************
* Name       :  K num_rows(K x,K y)
* Description:  Function which returns number of rows in the first table from a fits file
* Parameters :  x - q symbol corresponding to the fits file name `$"exampleTable.fits"
*               y - int representing the HDU of the table
*/               
K num_rows(K x, K y)
{
  fitsfile *fptr;
  long nrows;
  int status = 0,hdutype;
  status=open_table(&fptr,x->s,q_integer(y),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  fits_get_num_rows(fptr, &nrows, &status);
  close_file(fptr,&status);
  if(status)
    return krr("Unable to read FITS row count");
  return ki(nrows);
}

/********************************************************************
* Name       :  K num_cols(K x, K y)
* Description:  Function which returns number of columns in the first table from a fits file
* Parameters :  x - q symbol corresponding to the fits file name `$"exampleTable.fits"
*               y - int representing the HDU of the table
*/               
K num_cols(K x, K y)
{
  fitsfile *fptr;
  int status = 0,hdutype,ncols;
  status=open_table(&fptr,x->s,q_integer(y),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  fits_get_num_cols(fptr,&ncols,&status);
  close_file(fptr,&status);
  if(status)
    return krr("Unable to read FITS column count");
  return ki(ncols);
}

/********************************************************************
* Name       :  K cols(K x, K y)
* Description:  Function which returns a list of columns in a FITS table
* Parameters :  x - q symbol corresponding to the fits file name `$"example.fits"
*               y - int representing the HDU we're looking for
*/               
K cols(K x, K y)
{
  fitsfile *fptr;
  char colname[FLEN_VALUE],keyname[FLEN_KEYWORD];
  K cols=ktn(KS,0);
  int status=0,colnum,hdutype,ncols;
  status=open_table(&fptr,x->s,q_integer(y),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  fits_get_num_cols(fptr,&ncols,&status);
  for(colnum=1; colnum<=ncols; colnum++)
    {
      fits_make_keyn("TTYPE",colnum,keyname,&status);
      fits_read_key(fptr,TSTRING,keyname,colname,NULL,&status);
      if(status)
        break;
      js(&cols,ss(colname));
    }
  close_file(fptr,&status);
  if(status)
    {
      r0(cols);
      return krr("Unable to read FITS column names");
    }
  return cols;
}

/********************************************************************
* Name       :  K getColType(K x, K y, K z)
* Description:  Function which returns a normalized column type from a FITS table
* Parameters :  x - q symbol corresponding to the name of fits file 
*               y - q symbol corresponding to column name
*               z - q int representing the HDU we're looking at
*/               
K getColType(K x, K y, K z)
{
  fitsfile *fptr;
  int status=0,hdutype,colnum=0,typecode;
  long repeat,width;
  K result;
  status=open_table(&fptr,x->s,q_integer(z),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  fits_get_colnum(fptr,CASEINSEN,y->s,&colnum,&status);
  if(!status)
    fits_get_coltype(fptr,colnum,&typecode,&repeat,&width,&status);
  close_file(fptr,&status);
  if(status)
    return krr("Unable to read FITS column type");
  switch(abs(typecode))
    {
    case TBYTE:
    case TSBYTE:
    case TSHORT:
    case TINT:
    case TLONG:
    case TLONGLONG:
      result=ks("J");
      break;
    case TFLOAT:
    case TDOUBLE:
      result=ks("F");
      break;
    case TSTRING:
      result=ks("S");
      break;
    default:
      return krr("Unsupported FITS column type");
    }
  return result;
}


/********************************************************************
* Name       :  K readLongCol(K x,K y,K z,K h)
*               K readIntCol(K x,K y,K z,K h)
*               K readDoubleCol(K x,K y,K z,K h)
*               K readSymCol(K x,K y,K z,K h)
* Description:  Column extraction functions for different data types
*               Columns types extracted are:  c long->q long,c int->q int,c real and c float->q float
* Parameters:
* 		x - q symbol for fits file name
* 		y - q symbol for column name
* 		z - q long for number of rows to extract
* 		h - q int representing the number of the HDU we're looking at
*/
K readLongCol(K x,K y,K z,K h)
{
  fitsfile *fptr;
  int colnum=0,hdutype,status=0,anynulls=0;
  J nrows=q_integer(z);
  long long nullval=LLONG_MIN;
  K list;
  status=open_table(&fptr,x->s,(int)q_integer(h),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  if(nrows<0)
    {
      close_file(fptr,&status);
      return krr("Row count must not be negative");
    }
  fits_get_colnum(fptr,CASEINSEN,y->s,&colnum,&status);
  list=ktn(KJ,nrows);
  if(!status)
    fits_read_col(fptr,TLONGLONG,colnum,1,1,nrows,&nullval,kJ(list),&anynulls,&status);
  close_file(fptr,&status);
  if(status)
    {
      r0(list);
      return krr("Unable to read FITS integer column");
    }
  return list;
}

K readIntCol(K x,K y,K z,K h)
{
  fitsfile *fptr;
  int colnum=0,hdutype,status=0,anynulls=0,nullval=ni;
  J nrows=q_integer(z);
  K list;
  status=open_table(&fptr,x->s,(int)q_integer(h),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  if(nrows<0)
    {
      close_file(fptr,&status);
      return krr("Row count must not be negative");
    }
  fits_get_colnum(fptr,CASEINSEN,y->s,&colnum,&status);
  list=ktn(KI,nrows);
  if(!status)
    fits_read_col(fptr,TINT,colnum,1,1,nrows,&nullval,kI(list),&anynulls,&status);
  close_file(fptr,&status);
  if(status)
    {
      r0(list);
      return krr("Unable to read FITS integer column");
    }
  return list;
}

K readDoubleCol(K x,K y,K z,K h)
{
  fitsfile *fptr;
  int colnum=0,hdutype,status=0,anynulls=0;
  J nrows=q_integer(z);
  double nullval=nf;
  K list;
  status=open_table(&fptr,x->s,(int)q_integer(h),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  if(nrows<0)
    {
      close_file(fptr,&status);
      return krr("Row count must not be negative");
    }
  fits_get_colnum(fptr,CASEINSEN,y->s,&colnum,&status);
  list=ktn(KF,nrows);
  if(!status)
    fits_read_col(fptr,TDOUBLE,colnum,1,1,nrows,&nullval,kF(list),&anynulls,&status);
  close_file(fptr,&status);
  if(status)
    {
      r0(list);
      return krr("Unable to read FITS floating-point column");
    }
  return list;
}

K readSymCol(K x,K y,K z,K h)
{
  fitsfile *fptr;
  char nullstr[]="";
  char *value;
  int colnum=0,hdutype,status=0,anynul=0,typecode;
  J nrows=q_integer(z),row;
  long repeat,width;
  K list;
  status=open_table(&fptr,x->s,(int)q_integer(h),&hdutype);
  if(status)
    return krr("Unable to open FITS table");
  if(nrows<0)
    {
      close_file(fptr,&status);
      return krr("Row count must not be negative");
    }
  fits_get_colnum(fptr,CASEINSEN,y->s,&colnum,&status);
  if(!status)
    fits_get_coltype(fptr,colnum,&typecode,&repeat,&width,&status);
  if(status || width<0 || (unsigned long long)width>=SIZE_MAX)
    {
      close_file(fptr,&status);
      return krr("Unable to determine FITS text column width");
    }
  value=calloc((size_t)width+1,sizeof(char));
  if(!value)
    {
      close_file(fptr,&status);
      return krr("Unable to allocate FITS text buffer");
    }
  list=ktn(KS,nrows);
  for(row=0;row<nrows && !status;row++)
    {
      value[0]='\0';
      fits_read_col_str(fptr,colnum,row+1,1,1,nullstr,&value,&anynul,&status);
      if(!status)
        kS(list)[row]=ss(value);
    }
  free(value);
  close_file(fptr,&status);
  if(status)
    {
      r0(list);
      return krr("Unable to read FITS text column");
    }
  return list;
}
