#include <stdio.h>
#include <stdlib.h>

int write_long(FILE *out, int n)
{
  putc((n&255),out);
  putc(((n>>8)&255),out);
  putc(((n>>16)&255),out);
  putc(((n>>24)&255),out);

  return 0;
}

int write_word(FILE *out, int n)
{
  putc((n&255),out);
  putc(((n>>8)&255),out);

  return 0;
}

int read_long(FILE *in)
{
int c;

  c=getc(in);
  c=c+(getc(in)<<8);
  c=c+(getc(in)<<16);
  c=c+(getc(in)<<24);

  return c;
}

int read_word(FILE *in)
{
int c;

  c=getc(in);
  c=c+(getc(in)<<8);

  return c;
}

int main(int argc, char *argv[])
{
FILE *out,*in;
int c,x,y,k,ch,t,r,bits,num_icons,marker;

  printf("\nbmp2ico Version 1.01 - December 16, 2004\n");
  printf("Copyright 2004 Michael Kohn - http://www.mikekohn.net/\n");
  printf("part of thesnoW's SciTe, has licensed by Michael Kohn\n");
  if (argc<3)
  {
    printf("\nbmp2ico requires 1 output file and unlimited number of input files\n(windows limited to 32,768 characters).\n\n");
    printf("Usage: bmp2ico <output file> <one or more input files>\n\n");
    exit(0);
  }

  out=fopen(argv[1],"wb");
  if (out==0)
  {
    printf("Couldn't open file %s for writing.\n",argv[1]);
    exit(1);
  }

  num_icons=argc-2;

  write_word(out,0);
  write_word(out,1);
  write_word(out,num_icons);

  for (c=0; c<num_icons; c++)
  {
    write_long(out,0);
    write_long(out,0);
    write_long(out,0);
    write_long(out,0);
  }

  for (c=0; c<num_icons; c++)
  {
    printf("Reading %s\n",argv[c+2]);

    in=fopen(argv[c+2],"rb");
    if (in==0)
    {
      printf("Couldn't open %s for reading.\n",argv[c+2]);
    }

    marker=ftell(out);

    fseek(in,18,SEEK_SET);

    x=read_long(in);
    y=read_long(in);
    read_word(in);
    bits=read_word(in);

#ifdef DEBUG
printf("bits=%d\n",bits);
#endif

    if (bits>=8) bits=0;

#ifdef DEBUG
printf("%dx%d\n",x,y);
#endif

    if (x!=y || (x!=16 && x!=32 && x!=64))
    {
      printf("BMP's must be 16x16, 32x32, or 64x64: %s\n",argv[c+2]);
      break;
    }

    /* write Icon Directory */

#ifdef DEBUG
printf("marker=%d\n",marker);
#endif

    fseek(out,6+(c*16),SEEK_SET);
    putc(x,out);
    putc(y,out);
    if (bits==0)
    { putc(0,out); }
      else
    { putc(1<<bits,out); }
    putc(0,out);
    write_word(out,0);
    write_word(out,0);
    write_long(out,0);
    write_long(out,marker);
    fseek(out,marker,SEEK_SET);

#ifdef DEBUG
printf("Wrote Icon Directory\n");
#endif

    /* write BMP */

    fseek(in,14,SEEK_SET);

    t=read_long(in);
    write_long(out,t);     /* biSize */
    t=read_long(in);
    write_long(out,t);     /* biWidth */
    t=read_long(in);
    write_long(out,t*2);     /* biHeight */
    t=read_word(in);
    write_word(out,t);     /* biPlanes */
    bits=read_word(in);
    write_word(out,bits);  /* biPlanes */
    t=read_long(in);
    write_long(out,t);     /* biCompression */
    if (t!=0)
    {
      printf("BMP's cannot be compressed: %s\n",argv[c+2]);
      break;
    }

#ifdef DEBUG
printf("Wrote most of BMP header\n");
#endif

    write_long(out,0);
    write_long(out,0);
    write_long(out,0);
    t=write_long(out,0);
    write_long(out,0);

    read_long(in);
    read_long(in);
    read_long(in);
    t=read_long(in);
    read_long(in);

    if (bits<16)
    {
#ifdef DEBUG
printf("bits is less than 16...  palette is %d\n",t);
#endif
      for (r=0; r<t; r++)
      {
        write_long(out,read_long(in));
      }
#ifdef DEBUG
printf("padding\n");
#endif

      k=1<<bits;
      for (r=t; r<k; r++)
      {
        write_long(out,0);
      }
    }

#ifdef DEBUG
printf("wrote palette\n");
#endif

    while((ch=getc(in))!=EOF)
    {
      putc(ch,out);
    }

#ifdef DEBUG
printf("Adding padding\n");
#endif

    k=(x/4)+(x%4);
    while ((k%4)!=0) k++;

    t=y*k;

    for(r=0; r<t; r++)
    {
      putc(0,out);
    }

    r=ftell(out)-marker;
    marker=ftell(out);

#ifdef DEBUG
printf("writing size to: %d   size=%d\n",6+(c*16)+8,r);
#endif

    fseek(out,6+(c*16)+8,SEEK_SET);
    write_long(out,r);

    fseek(out,marker,SEEK_SET);

    fclose(in);
  }

  fclose(out);

  return 0;
}


