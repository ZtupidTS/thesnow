/*
buildtorrent -- torrent file creation program
Copyright (C) 2007,2008,2009,2010 Claude Heiland-Allen


This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "config.h"

//#include <getopt.h>
#include "getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <inttypes.h>
//#include <dirent.h>
#include "dirent.h"
#include <time.h>
/* for correct behaviour ensure this is the POSIX and not the GNU version */
//#include <libgen.h>
#include <malloc.h>

#include "libgen.h"

#include "sha1.h"
#include "md5.h"
//#pragma comment (linker,"/NODEFAULTLIB:libCMT.lib")
#pragma comment (lib,"libmingwex.a")
#pragma comment (lib,"libgcc.a")
#pragma comment (lib,"libmingw32.a")
#pragma comment (lib,"libmsvcrt.a")
#pragma comment (lib,"libmsvcrtd.a")
#pragma comment (lib,"libstdc++.a")

#ifdef _MSC_VER    /* however you determine using VC++ compiler ?: no clue! */
typedef  __int8 int8;
typedef  __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
#endif
#ifndef PRId64
#define PRId64 (sizeof (long) == 8 ? "ld" : "lld")
/*
#define PRId8     "hhd"
#define PRId16    "hd"
#define PRId32    "ld"
#define PRId64    "lld"
*/
#endif

/******************************************************************************
program version string
******************************************************************************/
#define bt_version "0.8"
const char* __version__ = bt_version;

/******************************************************************************
torrent data structure declarations
******************************************************************************/

/* structure for torrent file data */
struct _bt_data;
typedef struct _bt_data *bt_data;

/* error flags */
enum _bt_error {
  BT_ERROR_NONE = 0,
  BT_ERROR_NULL,
  BT_ERROR_TYPE,
  BT_ERROR_MEMORY,
  BT_ERROR_IO,
  BT_ERROR_OVERFLOW,
  BT_ERROR_PARSE
};
typedef enum _bt_error bt_error;

/* attempt to support large files */
typedef int64_t integer;

/* constructors */
bt_data bt_string(const unsigned int length, const char *data);
bt_data bt_integer(const integer number);
bt_data bt_list(void);
bt_data bt_dictionary(void);

/* append to a list */
bt_error bt_list_append(bt_data list, const bt_data data);

/* insert into a dictionary, maintaining sorted keys */
bt_error bt_dictionary_insert(
  bt_data dictionary,
  const unsigned int keylength, const char *keyname,
  const bt_data data
);

/* deep-copy a structure */
bt_data bt_copy(const bt_data data);

/* destructor */
void bt_free(bt_data bd);

/* write as a torrent file */
bt_error bt_write(FILE *f, const bt_data bd);

/* types of structures */
enum _bt_type {
  BT_TYPE_STRING = 1000,
  BT_TYPE_INTEGER,
  BT_TYPE_LIST,
  BT_TYPE_DICTIONARY
};
typedef enum _bt_type bt_type;

/* string structure */
struct _bt_string {
  unsigned int length;
  char *data;
};

/* integer structure */
struct _bt_integer {
  integer number;
};

/* list structure */
struct _bt_list {
  struct _bt_data *item;
  struct _bt_list *next;
};

/* dictionary structure */
struct _bt_dictionary {
  struct _bt_string *key;
  struct _bt_data *value;
  struct _bt_dictionary *next;
};

/* general structure */
struct _bt_data {
  bt_type type;
  union {
    struct _bt_string *string;
    struct _bt_integer *integer;
    struct _bt_list *list;
    struct _bt_dictionary *dictionary;
  } b;
};

/******************************************************************************
allocate a new string structure
******************************************************************************/
bt_data bt_string(const unsigned int length, const char *data) {
//  bt_data bd = malloc(sizeof(struct _bt_data));
  bt_data bd = new _bt_data;
  if (!bd) {
    return NULL;
  }
  {
//  struct _bt_string *s = malloc(sizeof(struct _bt_string));
    struct _bt_string *s = new struct _bt_string;
  if (!s) {
    free(bd);
    return NULL;
  }
//  s->data = malloc(length);
  s->data = new char[length];
  if (!s->data) {
    free(s);
    free(bd);
    return NULL;
  }
  s->length = length;
  memcpy(s->data, data, length);
  bd->type = BT_TYPE_STRING;
  bd->b.string = s;
  }
  return bd;
}

/******************************************************************************
allocate a new integer structure
******************************************************************************/
bt_data bt_integer(const integer number) {
//  bt_data bd = malloc(sizeof(struct _bt_data));
  bt_data bd = new struct _bt_data;
  if (!bd) {
    return NULL;
  }
  {
//  struct _bt_integer *n = malloc(sizeof(struct _bt_integer));
  struct _bt_integer *n = new struct _bt_integer;
  if (!n) {
    free(bd);
    return NULL;
  }
  n->number = number;
  bd->type = BT_TYPE_INTEGER;
  bd->b.integer = n;
  }
  return bd;
}

/******************************************************************************
allocate a new empty list structure
invariant: bd->b.list != NULL && last(bd->b.list).{item, next} == NULL
******************************************************************************/
bt_data bt_list(void) {
//  bt_data bd = malloc(sizeof(struct _bt_data));
  bt_data bd = new struct _bt_data;
  if (!bd) {
    return NULL;
  }
  {
//  struct _bt_list *l = malloc(sizeof(struct _bt_list));
  struct _bt_list *l = new struct _bt_list;
  if (!l) {
    free(bd);
    return NULL;
  }
  l->item = NULL;
  l->next = NULL;
  bd->type = BT_TYPE_LIST;
  bd->b.list = l;
  }
  return bd;
}

/******************************************************************************
allocate a new empty dictionary structure
invariant: bd->b.dictionary != NULL &&
           last(bd->b.list).{key, value, next} == NULL &&
	   ordered ascending by key
******************************************************************************/
bt_data bt_dictionary(void) {
//  bt_data bd = malloc(sizeof(struct _bt_data));
  bt_data bd = new struct _bt_data;
  if (!bd) {
    return NULL;
  }
  {
//  struct _bt_dictionary *d = malloc(sizeof(struct _bt_dictionary));
  struct _bt_dictionary *d = new struct _bt_dictionary;
  if (!d) {
    free(bd);
    return NULL;
  }
  d->key = NULL;
  d->value = NULL;
  d->next = NULL;
  bd->type = BT_TYPE_DICTIONARY;
  bd->b.dictionary = d;
  }
  return bd;
}

/******************************************************************************
append an item to a list
the item is not copied
returns an error flag
******************************************************************************/
bt_error bt_list_append(bt_data list, const bt_data data) {
  if (!list) {
    return BT_ERROR_NULL;
  }
  if (BT_TYPE_LIST != list->type) {
    return BT_ERROR_TYPE;
  }
  if (!list->b.list) {
    return BT_ERROR_NULL;
  }
  {
  struct _bt_list *current;
//  struct _bt_list *end = malloc(sizeof(struct _bt_list));
  struct _bt_list *end = new struct _bt_list;
  if (!end) {
    return BT_ERROR_MEMORY;
  }
  end->item = NULL;
  end->next = NULL;
  current = list->b.list;
  while (current->next) {
    current = current->next;
  }
  current->item = data;
  current->next = end;
  }
  return BT_ERROR_NONE;
}

/******************************************************************************
insert an item into a dictionary
the value is not copied, the key is copied
returns an error flag
maintains an ascending ordering of key names
FIXME: assumes key names are null terminated
******************************************************************************/
bt_error bt_dictionary_insert(
  bt_data dictionary,
  const unsigned int keylength, const char *keyname,
  const bt_data data
) {
  if (!dictionary) {
    return BT_ERROR_NULL;
  }
  if (BT_TYPE_DICTIONARY != dictionary->type) {
    return BT_ERROR_TYPE;
  }
  if (!dictionary->b.dictionary) {
    return BT_ERROR_NULL;
  }
  {
  struct _bt_dictionary *current;
  struct _bt_dictionary *lastcurrent = NULL;
  struct _bt_dictionary *insertee;
//  struct _bt_string *key = malloc(sizeof(struct _bt_string));
  struct _bt_string *key = new struct _bt_string;
  if (!key) {
    return BT_ERROR_MEMORY;
  }
//  key->data = malloc(keylength);
  key->data = new char[keylength];
  if (!key->data) {
    free(key);
    return BT_ERROR_MEMORY;
  }
//  if (!(insertee = malloc(sizeof(struct _bt_dictionary)))) {
  if (!(insertee = new struct _bt_dictionary)) {
    free(key->data);
    free(key);
    return BT_ERROR_MEMORY;
  }
  memcpy(key->data, keyname, keylength);
  key->length = keylength;
  insertee->key = key;
  insertee->value = data;
  insertee->next = NULL;
  current = dictionary->b.dictionary;
  while (
    current->next && current->key && (0 < strcmp(keyname, current->key->data))
  ) {
    lastcurrent = current;
    current = current->next;
  }
  if (lastcurrent) {
    insertee->next = current;
    lastcurrent->next = insertee;
  } else {
    insertee->next = dictionary->b.dictionary;
    dictionary->b.dictionary = insertee;
  }
  }
  return BT_ERROR_NONE;
}

/******************************************************************************
deep copy a data structure
FIXME: doesn't handle dictionaries yet
******************************************************************************/
bt_data bt_copy(const bt_data data) {
  if (!data) {
    return NULL;
  }
  switch (data->type) {
  case (BT_TYPE_STRING): {
    return bt_string(data->b.string->length, data->b.string->data);
  }
  case (BT_TYPE_INTEGER): {
    return bt_integer(data->b.integer->number);
  }
  case (BT_TYPE_LIST): {
    struct _bt_list *current;
    bt_error err;
    bt_data list = bt_list();
    if (!list) {
      return NULL;
    }
    current = data->b.list;
    while (current && current->item) {
      if ((err = bt_list_append(list, bt_copy(current->item)))) {
        bt_free(list);
        return NULL;
      }
      current = current->next;
    }
    return list;
  }
  default: {
    return NULL;
  }
  }
}

/******************************************************************************
free a string
******************************************************************************/
void bt_free_string(struct _bt_string *s) {
  if (!s) {
    return;
  }
  if (s->data) {
    free(s->data);
  }
  free(s);
}

/******************************************************************************
deep free a data structure
******************************************************************************/
void bt_free(bt_data bd) {
  if (!bd) {
    return;
  }
  switch (bd->type) {
  case (BT_TYPE_STRING): {
    if (bd->b.string) {
      bt_free_string(bd->b.string);
    }
    break;
  }
  case (BT_TYPE_INTEGER): {
    if (bd->b.integer) {
      free(bd->b.integer);
    }
    break;
  }
  case (BT_TYPE_LIST): {
    struct _bt_list *current;
    struct _bt_list *next;
    current = bd->b.list;
    while (current) {
      next = current->next;
      if (current->item) {
        bt_free(current->item);
      }
      free(current);
      current = next;
    }
    break;
  }
  case (BT_TYPE_DICTIONARY): {
    struct _bt_dictionary *current;
    struct _bt_dictionary *next;
    current = bd->b.dictionary;
    while (current) {
      next = current->next;
      if (current->key) {
        bt_free_string(current->key);
      }
      if (current->value) {
        bt_free(current->value);
      }
      free(current);
      current = next;
    }
    break;
  }
  default:
    break;
  }
  free(bd);
}

/******************************************************************************
write a string in torrent encoding
******************************************************************************/
bt_error bt_write_string(FILE *f, const struct _bt_string *s) {
  if (!s) {
    return BT_ERROR_NULL;
  }
  if (!s->data) {
    return BT_ERROR_NULL;
  }
  if (0 > fprintf(f, "%d:", s->length)) {
    return BT_ERROR_IO;
  }
  if (1 != fwrite(s->data, s->length, 1, f)) {
    return BT_ERROR_IO;
  }
  return BT_ERROR_NONE;
}

/******************************************************************************
write a data structure in torrent encoding
******************************************************************************/
bt_error bt_write(FILE *f, const bt_data bd) {
  if (!bd) {
    return BT_ERROR_NULL;
  }
  switch (bd->type) {
  case (BT_TYPE_STRING): {
    return bt_write_string(f, bd->b.string);
    break;
  }
  case (BT_TYPE_INTEGER): {
    if (!bd->b.integer) {
      return BT_ERROR_NULL;
    }
//    if (0 > fprintf(f, "i%" PRId64 "e", bd->b.integer->number)) {
    if (0 > fprintf(f, "i%llde", 1)) {
      return BT_ERROR_IO;
    }
    break;
  }
  case (BT_TYPE_LIST): {
    struct _bt_list *current;
    bt_error err;
    current = bd->b.list;
    if (0 > fprintf(f, "l")) {
      return BT_ERROR_IO;
    }
    while (current->next) {
      if (current->item) {
        if ((err = bt_write(f, current->item))) {
          return err;
        }
      }
      current = current->next;
    }
    if (0 > fprintf(f, "e")) {
      return BT_ERROR_IO;
    }
    break;
  }
  case (BT_TYPE_DICTIONARY): {
    struct _bt_dictionary *current;
    bt_error err;
    current = bd->b.dictionary;
    if (0 > fprintf(f, "d")) {
      return BT_ERROR_IO;
    }
    while (current->next) {
      if (current->key) {
        if ((err = bt_write_string(f, current->key))) {
          return err;
        }
        if (!current->value) {
          return BT_ERROR_NULL;
        }
        if ((err = bt_write(f, current->value))) {
          return err;
        }
      }
      current = current->next;
    }
    if (0 > fprintf(f, "e")) {
      return BT_ERROR_IO;
    }
    break;
  }
  default: {
    return BT_ERROR_TYPE;
    break;
  }
  }
  return BT_ERROR_NONE;
}

/******************************************************************************
pretty print torrent data
******************************************************************************/
void bt_show(bt_data bd, int piecestoo, int indent, int indentstep, int comma) {
  int i;
  if (!bd) {
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("NULL%s\n", comma ? "," : "");
    return;
  }
  switch (bd->type) {
  case (BT_TYPE_STRING): {
    char strbuf[512];
    int len = bd->b.string->length > 500 ? 500 : bd->b.string->length;
    memcpy(strbuf, bd->b.string->data, len);
    strbuf[len] = '\0';
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf(
      "\"%s\"%s%s\n", strbuf, comma ? "," : "", len == 500 ? " // truncated!" : ""
    );
    break;
  }
  case (BT_TYPE_INTEGER): {
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("%lld%s\n", bd->b.integer->number, comma ? "," : "");
    break;
  }
  case (BT_TYPE_LIST): {
    struct _bt_list *current;
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("[\n");
    current = bd->b.list;
    while (current->next) {
      bt_show(
        current->item, piecestoo,
        indent + indentstep, indentstep,
        current->next->next != NULL
      );
      current = current->next;
    }
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("]%s\n", comma ? "," : "");
    break;
  }
  case (BT_TYPE_DICTIONARY): {
    struct _bt_dictionary *current;
    char strbuf[512];
    int len;
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("{\n");
    current = bd->b.dictionary;
    while (current->next) {
      len = current->key->length > 500 ? 500 : current->key->length;
      memcpy(strbuf, current->key->data, len);
      strbuf[len] = '\0';
      for(i = 0; i < indent + indentstep; i++) {
        printf(" ");
      }
      printf("\"%s\" =>%s\n", strbuf, len == 500 ? " // truncated!" : "");
      if (strcmp("pieces", strbuf) == 0) {
        if (piecestoo) {
          printf("----------------------------------------");
          char *hexdigits = "0123456789abcdef";
          for (i = 0; i < current->value->b.string->length; i++) {
            if ((i % 20) == 0) {
              printf("\n");
            }
            printf("%c%c",
              hexdigits[(current->value->b.string->data[i] & 0xF0) >> 4],
              hexdigits[(current->value->b.string->data[i] & 0x0F)]
            );
          }
          printf("\n----------------------------------------\n");
        } else {
          for(i = 0; i < indent + indentstep + indentstep; i++) {
            printf(" ");
          }
          printf("\"...\"%s // pieces not shown\n", current->next->next ? "," : "");
        }
      } else {
        bt_show(
          current->value, piecestoo, indent + indentstep + indentstep,
          indentstep, current->next->next != NULL
        );
      }
      current = current->next;
    }
    for(i = 0; i < indent; i++) {
      printf(" ");
    }
    printf("}%s\n", comma ? "," : "");
    break;
  }
  }
}


/******************************************************************************
file list data structure
the first in the list is a dummy
******************************************************************************/
struct _bt_file_list {
  char *file;
  bt_data path;
  struct _bt_file_list *next;
};
typedef struct _bt_file_list *bt_file_list;

/******************************************************************************
create the dummy first list element
******************************************************************************/
bt_file_list bt_create_file_list() {
//  bt_file_list flist = malloc(sizeof(struct _bt_file_list));
  bt_file_list flist = new struct _bt_file_list;
  if (!flist) {
    return NULL;
  }
  flist->file = NULL;
  flist->path = NULL;
  flist->next = NULL;
  return flist;
}

/******************************************************************************
prepend to the file list
copies the arguments
path may be NULL, but only in single file mode
******************************************************************************/
bt_error bt_file_list_prepend(
  bt_file_list flist, const char *file, bt_data path
) {
  if (!flist) {
    return BT_ERROR_NULL;
  }
  if (!file) {
    return BT_ERROR_NULL;
  }
  {
//  bt_file_list node = malloc(sizeof(struct _bt_file_list));
  bt_file_list node = new struct _bt_file_list;
  if (!node) {
    return BT_ERROR_MEMORY;
  }
//  if (!(node->file = malloc(strlen(file) + 1))) {
  if (!(node->file = new char[strlen(file) + 1])) {
    free(node);
    return BT_ERROR_MEMORY;
  }
  memcpy(node->file, file, strlen(file) + 1);
  if (path) {
    if (!(node->path = bt_copy(path))) {
      free(node->file);
      free(node);
      return BT_ERROR_MEMORY;
    }
  } else {
    node->path = NULL;
  }
  node->next = flist->next;
  flist->next = node;
  return BT_ERROR_NONE;
  }
}


/******************************************************************************
annotated file list data structure
the first in the list is a dummy
******************************************************************************/
struct _bt_afile_list {
  char *file;
  bt_data path;
  integer length;
  bt_data md5sum;
  struct _bt_afile_list *next;
};
typedef struct _bt_afile_list *bt_afile_list;

/******************************************************************************
create the dummy first list element
******************************************************************************/
bt_afile_list bt_create_afile_list() {
//  bt_afile_list aflist = malloc(sizeof(struct _bt_afile_list));
  bt_afile_list aflist = new struct _bt_afile_list;
  if (!aflist) {
    return NULL;
  }
  aflist->file = NULL;
  aflist->path = NULL;
  aflist->length = 0;
  aflist->md5sum = NULL;
  aflist->next = NULL;
  return aflist;
}

/******************************************************************************
prepend to the annotated file list
aflist is an annotated file list to prepend to
fnode is a file list node to annotate
length is the length to annotate with
md5sum is the md5sum to annotate with (may be NULL)
NOTE does NOT create a new copy of the existing data
******************************************************************************/
bt_error bt_afile_list_prepend(
  bt_afile_list aflist, bt_file_list fnode, integer length, bt_data md5sum
) {
  if (!aflist) {
    return BT_ERROR_NULL;
  }
  if (!fnode) {
    return BT_ERROR_NULL;
  }
  {
//  bt_afile_list node = malloc(sizeof(struct _bt_afile_list));
  bt_afile_list node = new struct _bt_afile_list;
  if (!node) {
    return BT_ERROR_MEMORY;
  }
  node->file = fnode->file;
  node->path = fnode->path;
  node->length = length;
  node->md5sum = md5sum;
  node->next = aflist->next;
  aflist->next = node;
  return BT_ERROR_NONE;
  }
}


/******************************************************************************
append a file to a path string
path must be at least length maxlength bytes
returns an error flag
******************************************************************************/
bt_error bt_joinpath(size_t maxlength, char *path, const char *file) {
  if (strlen(path) + strlen(file) + 1 + 1 > maxlength) {
    return BT_ERROR_OVERFLOW;
  } else {
    size_t oldlength = strlen(path);
    if (oldlength > 0) {
      path[oldlength] = '/';
      memcpy(path + oldlength + 1, file, strlen(file) + 1);
    } else {
      memcpy(path, file, strlen(file) + 1);
    }
    return BT_ERROR_NONE;
  }
}


/******************************************************************************
md5sum a file
******************************************************************************/
bt_data bt_md5sum_file(const char *filename, integer length) {
  struct MD5Context context;
  char *hexdigits = "0123456789abcdef";
  unsigned char *buffer = NULL;
  unsigned char digest[16];
  char hexdump[33];
  integer buflen = 262144;
  integer size = 0;
  integer left = length;
  integer i;
  FILE *file = NULL;
  if (!filename) {
    return NULL;
  }
//  if (!(buffer = malloc(buflen))) {
  if (!(buffer = new BYTE[buflen])) {
    return NULL;
  }
  if (!(file = fopen(filename, "rb"))) {
    free(buffer);
    return NULL;
  }
  MD5Init(&context);
  while (left > 0) {
    if (left > buflen) {
       size = buflen;
    } else {
       size = left;
    }
    if (1 != fread(buffer, size, 1, file)) {
      free(buffer);
      fclose(file);
      return NULL;
    }
    MD5Update(&context, buffer, size);
    left -= size;
  }
  MD5Final(digest, &context);
  for (i = 0; i < 16; ++i) {
    hexdump[i+i+0] = hexdigits[(digest[i] & 0xF0) >> 4];
    hexdump[i+i+1] = hexdigits[(digest[i] & 0x0F)];
  }
  free(buffer);
  fclose(file);
  return bt_string(32, hexdump);
}


/******************************************************************************
find files recursively
directory is the initial directory
path is a buffer of maxlength bytes to store the accumulated path
pathlist is the accumulated path exploded as a list
files is a file list to add the found files to
returns an error flag
******************************************************************************/
bt_error bt_find_files(
  DIR *directory, size_t maxlength, char *path, bt_data pathlist, bt_file_list files
) {
  struct dirent *entry;
  bt_data filename = NULL;
  bt_data filepath = NULL;
  bt_error err = BT_ERROR_NONE;
  while ((entry = readdir(directory))) {
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
      struct stat s;
      size_t oldlength = strlen(path);
      if (!(err = bt_joinpath(maxlength, path, entry->d_name))) {
        if (!stat(path, &s)) {
          if (S_ISREG(s.st_mode)) {
            if ((filename = bt_string(strlen(entry->d_name), entry->d_name))) {
              if ((filepath = bt_copy(pathlist))) {
                if (!(err = bt_list_append(filepath, filename))) {
                  err = bt_file_list_prepend(files, path, filepath);
                }
              } else {
                err = BT_ERROR_MEMORY;
              }
            } else {
              err = BT_ERROR_MEMORY;
            }
          } else if (S_ISDIR(s.st_mode)) {
            DIR* dir;
            if ((dir = opendir(path))) {
              if ((filename = bt_string(strlen(entry->d_name), entry->d_name))) {
                if ((filepath = bt_copy(pathlist))) {
                  if (!(err = bt_list_append(filepath, filename))) {
                    err = bt_find_files(dir, maxlength, path, filepath, files);
                  }
                } else {
                  err = BT_ERROR_MEMORY;
                }
              } else {
                err = BT_ERROR_MEMORY;
              }
              closedir(dir);
            } else {
              err = BT_ERROR_IO;
            }
          } else {
            /* FIXME neither regular file nor directory, what to do? */
          }
        } else {
          err = BT_ERROR_IO;
        }
      }
      path[oldlength] = '\0';
    }
    if (err) {
      break;
    }
  }
  return err;
}


/******************************************************************************
annotate a file list
files is the file list to annotate
afiles is the file list to add to
returns an error flag
******************************************************************************/
bt_error bt_annotate_files(
  bt_file_list files, bt_afile_list afiles
) {
  bt_error err = BT_ERROR_NONE;
  bt_file_list fnode = files;
  while ((fnode = fnode->next)) {
    struct stat s;
    if (!stat(fnode->file, &s)) {
      if (S_ISREG(s.st_mode)) {
        integer length = s.st_size;
        err = bt_afile_list_prepend(afiles, fnode, length, NULL);
      } else {
        err = BT_ERROR_IO;
      }
    } else {
      err = BT_ERROR_IO;
    }
    if (err) {
      break;
    }
  }
  return err;
}


/******************************************************************************
convert an annotated file list into torrent format
aflist is the list to convert
returns the torrent format file list
******************************************************************************/
bt_data bt_afile_list_info(bt_afile_list aflist) {
  bt_afile_list node;
  bt_data files;
  if (!(files = bt_list())) {
    return NULL;
  }
  node = aflist;
  while ((node = node->next)) {
    bt_data file;
    bt_data filesize;
    bt_data filepath;
    bt_data md5sum;
    if (!(file = bt_dictionary())) {
      return NULL;
    }
    if (!(filesize = bt_integer(node->length))) {
      return NULL;
    }
    if (!(filepath = node->path)) {
      return NULL;
    }
    if (bt_dictionary_insert(file, strlen("length"), "length", filesize)) {
      return NULL;
    }
    if (bt_dictionary_insert(file, strlen("path"), "path", filepath)) {
      return NULL;
    }
    if ((md5sum = node->md5sum)) {
      if (bt_dictionary_insert(file, strlen("md5sum"), "md5sum", md5sum)) {
        return NULL;
      }
    }
    if (bt_list_append(files, file)) {
      return NULL;
    }
  }
  return files;
}


/******************************************************************************
draw progressbar
******************************************************************************/
void bt_progressbar(integer piece, integer count) {
  integer oldblocks = ((piece - 1) * 50) / count;
  integer blocks = (piece * 50) / count;
  integer i;
  char s[53];
  if (blocks != oldblocks) {
    s[0] = '[';
    for (i = 0; i < 50; i++) {
      s[i+1] = (i < blocks) ? '=' : ((i == blocks) ? '>' : ' ');
    }
    s[51] = ']';
    s[52] = '\0';
    fprintf(
      stderr,
      "\b\b\b\b\b\b\b\b\b\b"
      "\b\b\b\b\b\b\b\b\b\b"
      "\b\b\b\b\b\b\b\b\b\b"
      "\b\b\b\b\b\b\b\b\b\b"
      "\b\b\b\b\b\b\b\b\b\b"
      "\b\b%s",
      s
    );
  }
}


/******************************************************************************
hash files
return piece hash as a string
data side effect: if domd5sum then add md5sums to aflist
output side effect: if verbose then show file summary
******************************************************************************/
bt_data bt_hash_pieces(bt_afile_list aflist, integer size, int domd5sum, int verbose) {
  bt_afile_list node = NULL;
  char *hashdata = NULL;
  integer total = 0;
  unsigned char *buffer = NULL;
  unsigned char *bufptr = NULL;
  integer remain = size;
  integer left;
  FILE *file = NULL;
  integer piececount;
  integer i;
  sha1_byte digest[SHA1_DIGEST_LENGTH];
  if (!aflist) {
    return NULL;
  }
//  if (!(buffer = malloc(size))) {
  if (!(buffer = new unsigned char[size])) {
    return NULL;
  }
  node = aflist;
  fprintf(stderr, "数量 : 文件名-------------------------------↓\n");
  while ((node = node->next)) {
    if (verbose) {
      fprintf(stderr, "%10lld BYTE : %s\n", node->length, node->file);
    }
	if (node->length < 0){
		fprintf(stderr, "buildtorrent: error computing length for \"%s\"\n", node->file);
	}
    if (domd5sum) {
      if (!(node->md5sum = bt_md5sum_file(node->file, node->length))) {
        fprintf(stderr, "buildtorrent: error computing md5sum for \"%s\"\n", node->file);
      }
    }
    total += node->length;
  }
  piececount = (total + size - 1) / size; /* ceil(total/size) */
  if (piececount <= 0) { /* FIXME: no idea what to do if there's no data */
    free(buffer);
    fprintf(stderr, "torrent has no data, aborting!\n");
    return NULL;
  }
  fprintf(stderr, "-----------------区块列表-------------------↑\n");
  if (verbose) {
    fprintf(stderr, "hashing %lld pieces\n", piececount);
    fprintf(stderr, "["
                    "          "
                    "          "
                    "          "
                    "          "
                    "          "
                    "]");
  }
  if (!(hashdata = new char[piececount * SHA1_DIGEST_LENGTH])) {
    free(buffer);
    return NULL;
  }
  node = aflist->next;
  file = fopen(node->file, "rb");
  if (!file) {
    free(buffer);
    return NULL;
  }
  left = node->length;
  bufptr = buffer;
  for (i = 0; i < piececount; ++i) {
    do {
      if (left <= remain) {
        /* take all */
        if (left != 0) { /* don't fail on empty files */
          if (1 != fread(bufptr, left, 1, file)) {
            fclose(file);
            free(buffer);
            return NULL;
          }
          bufptr += left;
          remain -= left;
          fclose(file);
          file = NULL;
        }
        node = node->next;
        if (node) {
          file = fopen(node->file, "rb");
          if (!file) {
            free(buffer);
            return NULL;
          }
          left = node->length;
        }
      } else { /* left > remain */
        /* take as much as we can */
        if (remain != 0) { /* don't fail on empty files */
          if (1 != fread(bufptr, remain, 1, file)) {
            free(buffer);
            return NULL;
          }
          bufptr += remain;
          left -= remain;
          remain = 0;
        }
      }
    } while (remain != 0 && node);
    if (!node && i != piececount - 1) {
      /* somehow the pieces don't add up */
      if (file) {
        fclose(file);
      }
      free(buffer);
      return NULL;
    }
    /* remain == 0 || i == piececount - 1 */
    if (verbose) {
      bt_progressbar(i, piececount);
    }
    SHA1(buffer, size - remain, digest);
    memcpy(hashdata + i * SHA1_DIGEST_LENGTH, digest, SHA1_DIGEST_LENGTH);
    bufptr = buffer;
    remain = size;
  }
  if (verbose) {
    bt_progressbar(piececount, piececount);
    fprintf(stderr, "\n");
  }
  return bt_string(SHA1_DIGEST_LENGTH * piececount, hashdata);
}


/******************************************************************************
parse an announce list
format = "url|url|url,url|url|url,url|url|url"
******************************************************************************/
bt_data bt_parse_announcelist(const char *urls) {
  bt_data announcelist;
  bt_data tier;
  const char *s;
  const char *t;
  const char *t1;
  const char *t2;
  if (!urls) {
    return NULL;
  }
  if (strcmp("", urls) == 0) {
    return NULL;
  }
  announcelist = bt_list();
  if (!announcelist) {
    return NULL;
  }
  s = urls;
  tier = bt_list();
  do {
    t = NULL;
    t1 = strchr(s, '|');
    t2 = strchr(s, ',');
    if (!t1 && !t2) {
      t = s + strlen(s);
    } else if (!t1) {
      t = t2;
    } else if (!t2) {
      t = t1;
    } else {
      t = (t1 < t2) ? t1 : t2;
    }
    if (t <= s) {
      return NULL;
    }
    if (bt_list_append(tier, bt_string(t - s, s))) {
      return NULL;
    }
    if (t[0] == ',' || t[0] == '\0') {
      if (bt_list_append(announcelist, tier)) {
        return NULL;
      };
      if (t[0] != '\0') {
        tier = bt_list();
      } else {
        tier = NULL;
      }
    }
    s = t + 1;
  } while (t[0] != '\0');
  return announcelist;
}

/******************************************************************************
parse a webseed list
format = "url,url,url"
******************************************************************************/
bt_data bt_parse_webseedlist(const char *urls) {
  bt_data webseedlist;
  const char *s;
  const char *t;
  const char *t2;
  if (!urls) {
    return NULL;
  }
  if (strcmp("", urls) == 0) {
    return NULL;
  }
  webseedlist = bt_list();
  if (!webseedlist) {
    return NULL;
  }
  s = urls;
  do {
    t = NULL;
    t2 = strchr(s, ',');
    if (!t2) {
      t = s + strlen(s);
    } else {
      t = t2;
    }
    if (t <= s) {
      return NULL;
    }
    if (bt_list_append(webseedlist, bt_string(t - s, s))) {
      return NULL;
    }
    s = t + 1;
  } while (t[0] != '\0');
  return webseedlist;
}

/******************************************************************************
read and parse a filelist file
line format = "real/file/system/path|tor/rent/path/file"
delimiters: '|', '\n'
escape character: '\\'
******************************************************************************/
enum _bt_parse_state {
  BT_PARSE_LEFT = 0,
  BT_PARSE_LEFT_ESCAPED,
  BT_PARSE_RIGHT,
  BT_PARSE_RIGHT_ESCAPED,
  BT_PARSE_OK,
  BT_PARSE_ERROR
};
typedef enum _bt_parse_state bt_parse_state;
bt_error bt_parse_filelist(bt_file_list flist, FILE *infile) {
  char filename[8192];
  char torrname[8192];
  bt_data torrpath = NULL;
  int c;
  int i = 0;
  int state = BT_PARSE_LEFT;
  while(state != BT_PARSE_OK && state != BT_PARSE_ERROR) {
    c = getc(infile);
    switch (state) {
    case (BT_PARSE_LEFT):
      if (c < 0) {
        state = BT_PARSE_OK;
      } else if (c == '\\') {
        state = BT_PARSE_LEFT_ESCAPED;
      } else if (c == '|') {
        filename[i] = '\0';
        i = 0;
        torrpath = bt_list();
        if (!torrpath) {
          state = BT_PARSE_ERROR;
        } else {
          state = BT_PARSE_RIGHT;
        }
      } else {
        filename[i++] = c;
        if (i > 8190) {
          state = BT_PARSE_ERROR;
        } else {
          state = BT_PARSE_LEFT;
        }
      }
      break;
    case (BT_PARSE_LEFT_ESCAPED):
      if (c < 0) {
        state = BT_PARSE_ERROR;
      } else {
        filename[i++] = c;
        if (i > 8190) {
          state = BT_PARSE_ERROR;
        } else {
          state = BT_PARSE_LEFT;
        }
      }
      break;
    case (BT_PARSE_RIGHT):
      if (c < 0) {
        state = BT_PARSE_ERROR;
      } else if (c == '\\') {
        state = BT_PARSE_RIGHT_ESCAPED;
      } else if (c == '/' || c == '\n') {
        bt_data torrnamestr;
        torrname[i] = '\0';
        i = 0;
        if (0 == strcmp("", torrname)) {
          state = BT_PARSE_ERROR;
        } else {
          torrnamestr = bt_string(strlen(torrname), torrname);
          if (!torrnamestr) {
            state = BT_PARSE_ERROR;
          } else {
            bt_list_append(torrpath, torrnamestr);
            if (c == '\n') {
              bt_file_list_prepend(flist, filename, torrpath);
              state = BT_PARSE_LEFT;
            } else {
              state = BT_PARSE_RIGHT;
            }
          }
        }
      } else {
        torrname[i++] = c;
        if (i > 8190) {
          state = BT_PARSE_ERROR;
        } else {
          state = BT_PARSE_RIGHT;
        }
      }
      break;
    case (BT_PARSE_RIGHT_ESCAPED):
      if (c < 0) {
        state = BT_PARSE_ERROR;
      } else {
        torrname[i++] = c;
        if (i > 8190) {
          state = BT_PARSE_ERROR;
        } else {
          state = BT_PARSE_RIGHT;
        }
      }
      break;
    default:
      fprintf(stderr, "buildtorrent: internal error parsing file list (%d)\n", state);
      break;
    }
  }
  if (state == BT_PARSE_OK) {
    return BT_ERROR_NONE;
  } else {
    return BT_ERROR_PARSE;
  }
}

/******************************************************************************
show usage message
******************************************************************************/
void bt_usage(void) {
  printf(
    "Usage:\n"
    "  buildtorrent [OPTIONS] -a announceurl input output\n"
    "  buildtorrent [OPTIONS] -a announceurl -f filelist -n name output\n"
    "\n"
    "options:\n"
    "--announce        -a  <announce>   : announce url (required)\n"
    "--filelist        -f  <filelist>   : external file list (requires '-n')\n"
    "--name            -n  <name>       : torrent name, default based on input\n"
    "--announcelist    -A  <announces>  : announce url list (format: a,b1|b2,c)\n"
    "--webseeds        -w  <webseeds>   : webseed url list (format: a,b,c)\n"
    "--piecelength     -l  <length>     : piece length in bytes, default 262144\n"
    "--piecesize       -L  <size>       : use 2^size as piece length, default 18\n"
    "--comment         -c  <comment>    : user comment, omitted by default\n"
    "--private         -p  <private>    : private flag, either 0 or 1\n"
    "--nodate          -D               : omit 'creation date' field\n"
    "--nocreator       -C               : omit 'created by' field\n"
    "--md5sum          -m               : add an 'md5sum' field for each file\n"
    "--show            -s               : show generated torrent structure\n"
    "--showall         -S               : show pieces too (implies '-s')\n"
    "--quiet           -q               : quiet operation\n"
    "--version         -V               : show version of buildtorrent\n"
    "--help            -h               : show this help screen\n"
  );
}

/******************************************************************************
show version message
******************************************************************************/
void bt_showversion(void) {
	printf(
		"buildtorrent " bt_version "\n"
		"Copyright (C) 2007-2010 Claude Heiland-Allen <claudiusmaximus@goto10.org>\n"
		"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n"
		"This is a modified version for thegfw\n\n"
    );
}

/******************************************************************************
main program
******************************************************************************/
int main(int argc, char **argv) {

  char *url = NULL;
  char *urls = NULL;
  char *wurls = NULL;
  char *inname = NULL;
  char *nameflag = NULL;
  char *namebase = NULL;
  char *outfile = NULL;
  char *commentstr = NULL;
  char *filelistfilename = NULL;
  int lplen = -1;
  unsigned int plen = 262144;
  int verbose = 1;
  int nodate = 0;
  int nocreator = 0;
  int privated = 0;
  int privateopt = 0;
  int domd5sum = 0;
  int show = 0;
  int slen;
  int i;

  DIR *dir = NULL;
  FILE *output = NULL;
  bt_data torrent = NULL;
  bt_data announce = NULL;
  bt_data announcelist = NULL;
  bt_data webseedlist = NULL;
  bt_data info = NULL;
  bt_data piecelength = NULL;
  bt_data pieces = NULL;
  bt_data files = NULL;
  bt_data name = NULL;
  bt_data length = NULL;
  bt_data pathlist = NULL;
  bt_data creator = NULL;
  bt_data creationdate = NULL;
  bt_data comment = NULL;
  bt_data isprivate = NULL;
  bt_data md5sum = NULL;

  int multifile = 0;
  bt_file_list flist = NULL;
  bt_afile_list aflist = NULL;

  struct stat s;
  char path[8192];
  char nametemp[8192];

  while (1) {
    int optidx = 0;
    static struct option options[] = {
      { "announce", 1, 0, 'a' },
      { "filelist", 1, 0, 'f' },
      { "name", 1, 0, 'n' },
      { "announcelist", 1, 0, 'A' },
      { "webseeds", 1, 0, 'w' },
      { "piecelength", 1, 0, 'l' },
      { "piecesize", 1, 0, 'L' },
      { "comment", 1, 0, 'c' },
      { "private", 1, 0, 'p' },
      { "nodate", 0, 0, 'D' },
      { "nocreator", 0, 0, 'C' },
      { "md5sum", 0, 0, 'm' },
      { "show", 0, 0, 's' },
      { "showpieces", 0, 0, 'S' },
      { "quiet", 0, 0, 'q' },
      { "version", 0, 0, 'V' },
      { "help", 0, 0, 'h' },
      { 0, 0, 0, 0 }
    };
    char c = getopt_long(argc, argv, "hVqSsmCDa:f:n:A:w:l:L:c:p:", options, &optidx );
    if (c == -1) {
      break;
    }
    switch (c) {
    case ('?'):
      return 1;
    case ('a'):
      url = optarg;
      break;
    case ('f'):
      filelistfilename = optarg;
      break;
    case ('n'):
      nameflag = optarg;
      break;
    case ('A'):
      urls = optarg;
      break;
    case ('w'):
      wurls = optarg;
      break;
    case ('l'):
      plen = atoi(optarg);
      break;
    case ('L'):
      lplen = atoi(optarg);
      break;
    case ('c'):
      commentstr = optarg;
      break;
    case ('p'):
      privated = 1;
      privateopt = (strcmp(optarg, "0") == 0) ? 0 : 1;
      break;
    case ('D'):
      nodate = 1;
      break;
    case ('C'):
      nocreator = 1;
      break;
    case ('m'):
      domd5sum = 1;
      break;
    case ('s'):
      show = 1;
      break;
    case ('S'):
      show = 2;
      break;
    case ('q'):
      verbose = 0;
      break;
    case ('V'):
		bt_showversion();
/*		
      printf(
        "buildtorrent " bt_version "\n"
        "Copyright (C) 2007-2010 Claude Heiland-Allen <claudiusmaximus@goto10.org>\n"
        "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n"
		"This is a modified version for thegfw\n\n"
      );
*/
      return 0;
    case ('h'):
      bt_usage();
      return 0;
    }
  }
  if (!url) {
	bt_showversion();
    fprintf(stderr, "buildtorrent: announce url required\n");
	bt_usage();
    return 1;
  }
  if (0 <= lplen && lplen < 31) {
    plen = 1 << lplen;
  }
  if (plen <= 0) { /* avoid division by zero */
    fprintf(stderr, "buildtorrent: piece length must be greater than 0\n");
    return 1;
  }

  if (filelistfilename) {
    if (optind + 1 < argc) {
      fprintf(stderr, "buildtorrent: too many arguments\n");
      return 1;
    }
    if (optind + 1 > argc) {
      fprintf(stderr, "buildtorrent: too few arguments\n");
      return 1;
    }
    if (!nameflag) {
      fprintf(stderr, "buildtorrent: missing '-n', required when using '-f'\n");
      return 1;
    }
    inname = NULL;
    outfile = argv[optind];
  } else {
    if (optind + 2 < argc) {
      fprintf(stderr, "buildtorrent: too many arguments\n");
      return 1;
    }
    if (optind + 2 > argc) {
      fprintf(stderr, "buildtorrent: too few arguments\n");
      return 1;
    }
    inname  = argv[optind];
    outfile = argv[optind + 1];
  }

  /* handle paths correctly (note: requires POSIX basename(), not GNU) */
  if (inname) {
    if (strlen(inname) > 8190) {
      fprintf(stderr, "buildtorrent: 'input' argument too long\n");
      return 1;
    }
    strncpy(nametemp, inname, 8191);
    nametemp[8191] = '\0';
    namebase = basename(nametemp);
    slen = strlen(namebase);
    for (i = 0; i < slen; ++i) {
      if (namebase[i] == '/') {
        fprintf(
          stderr,
          "buildtorrent: BUG! input (\"%s\") munged (\"%s\") contains '/'.\n",
          inname,
          namebase
        );
        return 1;
      }
    }
  }

  if (inname) {
    if (stat(inname, &s)) {
      fprintf(stderr, "buildtorrent: could not stat \"%s\"\n", inname);
      return 1;
    }
  }

  if (!(torrent = bt_dictionary())) {
    fprintf(stderr, "buildtorrent: couldn't allocate torrent dictionary\n");
    return 1;
  }
  if (!(info = bt_dictionary())) {
    fprintf(stderr, "buildtorrent: couldn't allocate info dictionary\n");
    return 1;
  }
  if (!(announce = bt_string(strlen(url), url))) {
    fprintf(stderr, "buildtorrent: couldn't allocate announce string\n");
    return 1;
  }
  if (!(piecelength = bt_integer(plen))) {
    fprintf(stderr, "buildtorrent: couldn't allocate piece length integer\n");
    return 1;
  }
  if (nameflag) {
    name = bt_string(strlen(nameflag), nameflag);
  } else {
    name = bt_string(strlen(namebase), namebase);
  }
  if (!name) {
    fprintf(stderr, "buildtorrent: couldn't allocate name string\n");
    return 1;
  }
  if (bt_dictionary_insert(info, strlen("name"), "name", name)) {
    fprintf(stderr, "buildtorrent: couldn't insert name into info\n");
    return 1;
  }
  if (bt_dictionary_insert(
    info, strlen("piece length"), "piece length", piecelength
  )) {
    fprintf(stderr, "buildtorrent: couldn't insert piece length into info\n");
    return 1;
  }
  if (urls) {
    if (!(announcelist = bt_parse_announcelist(urls))) {
      fprintf(stderr, "buildtorrent: error parsing announce-list argument\n");
      return 1;
    }
  }
  if (wurls) {
    if (!(webseedlist = bt_parse_webseedlist(wurls))) {
      fprintf(stderr, "buildtorrent: error parsing webseed list argument\n");
      return 1;
    }
  }
  if (!(flist = bt_create_file_list())) {
    fprintf(stderr, "buildtorrent: couldn't allocate file list\n");
    return 1;
  }
  if (!(aflist = bt_create_afile_list())) {
    fprintf(stderr, "buildtorrent: couldn't allocate annotated file list\n");
    return 1;
  }

  if (inname && S_ISDIR(s.st_mode)) {

    multifile = 1;
    if (!(dir = opendir(inname))) {
      fprintf(stderr, "buildtorrent: couldn't open directory\n");
      return 1;
    }
    if (!(pathlist = bt_list())) {
      fprintf(stderr, "buildtorrent: couldn't path list\n");
      return 1;
    }
    memcpy(path, inname, strlen(inname) + 1);
    if (bt_find_files(dir, 8192, path, pathlist, flist)) {
      fprintf(stderr, "buildtorrent: error finding files\n");
      return 1;
    }
    closedir(dir);

  } else if (inname && S_ISREG(s.st_mode)) {

    multifile = 0;
    if ((bt_file_list_prepend(flist, inname, NULL))) {
      fprintf(stderr, "buildtorrent: error building single file list\n");
      return 1;
    }

  } else if (!inname) {
 
    multifile = 1;
    bt_error err = BT_ERROR_NONE;
    if (0 == strcmp("-", filelistfilename)) {
      err = bt_parse_filelist(flist, stdin);
    } else {
      FILE *filelistfile;
      if ((filelistfile = fopen(filelistfilename, "rb"))) {
        err = bt_parse_filelist(flist, filelistfile);
        fclose(filelistfile);
      } else {
        fprintf(stderr, "buildtorrent: couldn't open file list \"%s\"\n", filelistfilename);
        return 1;        
      }
    }
    if (err) {
      fprintf(stderr, "buildtorrent: error processing file list\n");
      return 1;
    }

  } else {

    fprintf(
      stderr, "buildtorrent: \"%s\" is neither file nor directory\n", inname
    );
    return 1;

  }

  if ((bt_annotate_files(flist, aflist))) {
    fprintf(stderr, "buildtorrent: error annotating file list\n");
    return 1;
  }

  if (privated) {
    if (!(isprivate = bt_integer(privateopt))) {
      fprintf(stderr, "buildtorrent: couldn't allocate private integer\n");
      return 1;
    }
    if (bt_dictionary_insert(info, strlen("private"), "private", isprivate)) {
      fprintf(stderr, "buildtorrent: couldn't insert private into info\n");
      return 1;
    }
  }  
  if (bt_dictionary_insert(torrent, strlen("announce"), "announce", announce)) {
    fprintf(stderr, "buildtorrent: couldn't insert announce into torrent\n");
    return 1;
  }
  if (urls) {
    if (bt_dictionary_insert(torrent, strlen("announce-list"), "announce-list", announcelist)) {
      fprintf(stderr, "buildtorrent: couldn't insert announce-list into torrent\n");
      return 1;
    }
  }
  if (wurls) {
    if (bt_dictionary_insert(torrent, strlen("url-list"), "url-list", webseedlist)) {
      fprintf(stderr, "buildtorrent: couldn't insert webseed url-list into torrent\n");
      return 1;
    }
  }
  if (!nodate) {
    if (!(creationdate = bt_integer(time(NULL)))) {
      fprintf(stderr, "buildtorrent: couldn't allocate creation date integer\n");
      return 1;
    }
    if (bt_dictionary_insert(
      torrent, strlen("creation date"), "creation date", creationdate
    )) {
      fprintf(stderr, "buildtorrent: couldn't insert creation date into torrent\n");
      return 1;
    }
  }
  if (!nocreator) {
    if (!(creator = bt_string(strlen("buildtorrent/" bt_version), "buildtorrent/" bt_version))) {
      fprintf(stderr, "buildtorrent: couldn't allocate created by string\n");
      return 1;
    }
    if (bt_dictionary_insert(
      torrent, strlen("created by"), "created by", creator
    )) {
      fprintf(stderr, "buildtorrent: couldn't insert created by into torrent\n");
      return 1;
    }
  }
  if (commentstr) {
    if (!(comment = bt_string(strlen(commentstr), commentstr))) {
      fprintf(stderr, "buildtorrent: couldn't allocate comment string\n");
      return 1;
    }
    if (bt_dictionary_insert(
      torrent, strlen("comment"), "comment", comment
    )) {
      fprintf(stderr, "buildtorrent: couldn't insert comment into torrent\n");
      return 1;
    }
  }
  if (!(output = fopen(outfile, "wb"))) {
    fprintf(stderr, "buildtorrent: couldn't open \"%s\" for writing\n", outfile);
    return 1;
  }

  if (!(pieces = bt_hash_pieces(aflist, plen, domd5sum, verbose))) {
    fprintf(stderr, "buildtorrent: error hashing files\n");
    return 1;
  }

  if (multifile) {
    if (!(files = bt_afile_list_info(aflist))) {
      fprintf(stderr, "buildtorrent: error getting file list info\n");
      return 1;
    }
    if (bt_dictionary_insert(info, strlen("files"), "files", files)) {
      fprintf(stderr, "buildtorrent: couldn't insert files into info\n");
      return 1;
    }
  } else {
    bt_afile_list node = aflist->next;
    if (!(length = bt_integer(node->length))) {
      fprintf(stderr, "buildtorrent: couldn't allocate length integer\n");
      return 1;
    }
    if (bt_dictionary_insert(info, strlen("length"), "length", length)) {
      fprintf(stderr, "buildtorrent: couldn't insert length into info\n");
      return 1;
    }
    if (node->md5sum) {
      if (bt_dictionary_insert(info, strlen("md5sum"), "md5sum", node->md5sum)) {
        fprintf(stderr, "buildtorrent: couldn't insert md5sum into info\n");
        return 1;
      }
    }
  }

  if (bt_dictionary_insert(info, strlen("pieces"), "pieces", pieces)) {
    fprintf(stderr, "buildtorrent: couldn't insert pieces into info\n");
    return 1;
  }
  if (bt_dictionary_insert(torrent, strlen("info"), "info", info)) {
    fprintf(stderr, "buildtorrent: couldn't insert info into torrent\n");
    return 1;
  }

  if (bt_write(output, torrent)) {
    fprintf(stderr, "buildtorrent: error writing \"%s\"\n", outfile);
    return 1;
  }
  if (show) {
    printf("torrent =>\n");
    bt_show(torrent, show == 2, 2, 2, 0);
  }
  bt_free(torrent);
  fclose(output);
  return 0;
}

/* EOF */
