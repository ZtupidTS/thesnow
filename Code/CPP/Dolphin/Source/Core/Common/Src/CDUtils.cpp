// Most of the code in this file was shamelessly ripped from libcdio With minor adjustments
 
#include "CDUtils.h"
#include "Common.h"
  
#ifdef _WIN32
#include <windows.h>
#define PATH_MAX MAX_PATH

#elif __APPLE__
#include <paths.h>
#include <Carbon/Carbon.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/IOBSD.h>

#elif __linux__
#include <mntent.h>
#include <unistd.h> 
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
 
#include <linux/cdrom.h>
#endif // WIN32

// Follow symlinks until we have the real device file (idea taken from libunieject). 
void cdio_follow_symlink(const char * src, char * dst) {
#ifndef _WIN32
	char tmp_src[PATH_MAX+1];
	char tmp_dst[PATH_MAX+1];
	
	int len;
	
	strcpy(tmp_src, src);
	while(1) {
		len = readlink(tmp_src, tmp_dst, PATH_MAX);
		if(len < 0) {
			strncpy(dst, tmp_src, PATH_MAX);
			return;
		}
		else {
			tmp_dst[len] = '\0';
			strncpy(tmp_src, tmp_dst, PATH_MAX);
		}
	}
#else
	strncpy(dst, src, PATH_MAX);
#endif
}

#ifdef _WIN32
// Returns a string that can be used in a CreateFile call if 
// c_drive letter is a character. If not NULL is returned.
const char *is_cdrom_win32(const char c_drive_letter) {
	
	UINT uDriveType;
	char sz_win32_drive[4];
	
	sz_win32_drive[0]= c_drive_letter;
	sz_win32_drive[1]=':';
	sz_win32_drive[2]='\\';
	sz_win32_drive[3]='\0';
	
	uDriveType = GetDriveType(sz_win32_drive);
	
	switch(uDriveType) {
	  case DRIVE_CDROM: {
		  char sz_win32_drive_full[] = "\\\\.\\X:";
		  sz_win32_drive_full[4] = c_drive_letter;
		  return __strdup(&sz_win32_drive_full[4]);
	  }
	  default:
		  //cdio_debug("Drive %c is not a CD-ROM", c_drive_letter);
		  return NULL;
	}
}

// Returns a pointer to an array of strings with the device names
std::vector<std::string> cdio_get_devices_win32() {
	std::vector<std::string> drives;
	char drive_letter;
	
	// Scan the system for CD-ROM drives.
	// Not always 100% reliable, so use the USE_MNTENT code above first.
	for (drive_letter='A'; drive_letter <= 'Z'; drive_letter++) {
		const char *drive_str=is_cdrom_win32(drive_letter);
		if (drive_str != NULL) {
			drives.push_back(drive_str);
			delete drive_str;
		}
	}
	return drives;
}
#endif // WIN32


#ifdef __APPLE__

/*
  Returns a pointer to an array of strings with the device names
*/
std::vector<std::string> cdio_get_devices_osx(void) {
	io_object_t   next_media;
	mach_port_t   master_port;
	kern_return_t kern_result;
	io_iterator_t media_iterator;
	CFMutableDictionaryRef classes_to_match;
	std::vector<std::string> drives;
	
	kern_result = IOMasterPort( MACH_PORT_NULL, &master_port );
	if( kern_result != KERN_SUCCESS ) {
		return( drives );
	}
	
	classes_to_match = IOServiceMatching( kIOCDMediaClass );
	if( classes_to_match == NULL ) {
		return( drives );
	}
	
	CFDictionarySetValue( classes_to_match, CFSTR(kIOMediaEjectableKey),
						  kCFBooleanTrue );
	
	kern_result = IOServiceGetMatchingServices( master_port, 
												classes_to_match,
												&media_iterator );
	if( kern_result != KERN_SUCCESS) {
		return( drives );
	}
	
	next_media = IOIteratorNext( media_iterator );
	if( next_media != 0 ) {
		char psz_buf[0x32];
		size_t dev_path_length;
		CFTypeRef str_bsd_path;
		
		do {
			str_bsd_path = 
				IORegistryEntryCreateCFProperty( next_media,
												 CFSTR( kIOBSDNameKey ),
												 kCFAllocatorDefault,
												 0 );
			if( str_bsd_path == NULL ) {
				IOObjectRelease( next_media );
				continue;
			}
			
			/* Below, by appending 'r' to the BSD node name, we indicate
			   a raw disk. Raw disks receive I/O requests directly and
			   don't go through a buffer cache. */        
			snprintf( psz_buf, sizeof(psz_buf), "%s%c", _PATH_DEV, 'r' );
			dev_path_length = strlen( psz_buf );
			
			if( CFStringGetCString( (CFStringRef)str_bsd_path,
									(char*)&psz_buf + dev_path_length,
									sizeof(psz_buf) - dev_path_length,
									kCFStringEncodingASCII)) {
				if(psz_buf != NULL)
				{
					std::string str = psz_buf;
					drives.push_back(str);
				}
			}
			CFRelease( str_bsd_path );
			IOObjectRelease( next_media );
			
		} while( ( next_media = IOIteratorNext( media_iterator ) ) != 0 );
	}
	IOObjectRelease( media_iterator );
	return drives;
}
#endif

#ifdef __linux__
/* checklist: /dev/cdrom, /dev/dvd /dev/hd?, /dev/scd? /dev/sr? */
static char checklist1[][40] = {
	{"cdrom"}, {"dvd"}, {""}
};

static struct
{
    const char * format;
    unsigned int num_min;
    unsigned int num_max;
} checklist2[] =
	{
		{ "/dev/hd%c",  'a', 'z' },
		{ "/dev/scd%d", 0,   27 },
		{ "/dev/sr%d",  0,   27 },
		{ /* End of array */ }
	};


/*
  Returns true if a device is a block or char device
 */
bool cdio_is_device_quiet_generic(const char *source_name) {
	struct stat buf;
	if (0 != stat(source_name, &buf)) {
		return false;
	}
	return (S_ISBLK(buf.st_mode) || S_ISCHR(buf.st_mode));
}

/*
  Check a drive to see if it is a CD-ROM 
   Return 1 if a CD-ROM. 0 if it exists but isn't a CD-ROM drive
   and -1 if no device exists .
*/
static bool is_cdrom_linux(const char *drive, char *mnttype) {
	bool is_cd=false;
	int cdfd;
	
	/* If it doesn't exist, return -1 */
	if ( !cdio_is_device_quiet_generic(drive) ) {
		return(false);
	}
	
	/* If it does exist, verify that it's an available CD-ROM */
	cdfd = open(drive, (O_RDONLY|O_NONBLOCK), 0);
	if ( cdfd >= 0 ) {
		if ( ioctl(cdfd, CDROM_GET_CAPABILITY, 0) != -1 ) {
			is_cd = true;
		}
		close(cdfd);
    }
	/* Even if we can't read it, it might be mounted */
	else if ( mnttype && (strcmp(mnttype, "iso9660") == 0) ) {
		is_cd = true;
	}
	return(is_cd);
}

/*
  Recive an mtab formated file and returns path to cdrom device
*/
static char *check_mounts_linux(const char *mtab)
{
  FILE *mntfp;
  struct mntent *mntent;
  
  mntfp = setmntent(mtab, "r");
  if ( mntfp != NULL ) {
	  char *tmp;
	  char *mnt_type;
	  char *mnt_dev;
	  unsigned int i_mnt_type;
	  unsigned int i_mnt_dev;
	  
	  while ( (mntent=getmntent(mntfp)) != NULL ) {
		  i_mnt_type = strlen(mntent->mnt_type) + 1;
		  mnt_type = (char *)calloc(1, i_mnt_type);
		  if (mnt_type == NULL)
			  continue;  /* maybe you'll get lucky next time. */
		  
		  i_mnt_dev = strlen(mntent->mnt_fsname) + 1;
		  mnt_dev = (char *)calloc(1, i_mnt_dev);
		  if (mnt_dev == NULL) {
			  free(mnt_type);
			  continue;
		  }
		  
		  strncpy(mnt_type, mntent->mnt_type, i_mnt_type);
		  strncpy(mnt_dev, mntent->mnt_fsname, i_mnt_dev);
		  
		  /* Handle "supermount" filesystem mounts */
		  if ( strcmp(mnt_type, "supermount") == 0 ) {
			  tmp = strstr(mntent->mnt_opts, "fs=");
			  if ( tmp ) {
				  free(mnt_type);
				  mnt_type = __strdup(tmp + strlen("fs="));
				  if ( mnt_type ) {
					  tmp = strchr(mnt_type, ',');
					  if ( tmp ) {
						  *tmp = '\0';
					  }
				  }
			  }
			  tmp = strstr(mntent->mnt_opts, "dev=");
			  if ( tmp ) {
				  free(mnt_dev);
				  mnt_dev = __strdup(tmp + strlen("dev="));
				  if ( mnt_dev ) {
					  tmp = strchr(mnt_dev, ',');
					  if ( tmp ) {
						  *tmp = '\0';
					  }
				  }
			  }
		  }
		  if ( strcmp(mnt_type, "iso9660") == 0 ) {
			  if (is_cdrom_linux(mnt_dev, mnt_type) > 0) {
				  free(mnt_type);
				  endmntent(mntfp);
				  return mnt_dev;
			  }
		  }
		  free(mnt_dev);
		  free(mnt_type);
	  }
	  endmntent(mntfp);
  }
  return NULL;
}

// Returns a pointer to an array of strings with the device names
std::vector<std::string> cdio_get_devices_linux () {
	
	unsigned int i;
	char drive[40];
	char *ret_drive;
	std::vector<std::string> drives;
	
	// Scan the system for CD-ROM drives.
	for ( i=0; strlen(checklist1[i]) > 0; ++i ) {
		sprintf(drive, "/dev/%s", checklist1[i]);
		if ( is_cdrom_linux(drive, NULL) > 0 ) {
			std::string str = drive;
			drives.push_back(str);
		}
	}
	
	/* Now check the currently mounted CD drives */
	if (NULL != (ret_drive = check_mounts_linux("/etc/mtab"))) {
		std::string str = ret_drive;
		drives.push_back(str);
		free(ret_drive);
	}
	
	/* Finally check possible mountable drives in /etc/fstab */
	if (NULL != (ret_drive = check_mounts_linux("/etc/fstab"))) {
		std::string str = ret_drive;
		drives.push_back(str);
		free(ret_drive);
	}
	
	// Scan the system for CD-ROM drives.
	// Not always 100% reliable, so use the USE_MNTENT code above first.
	for ( i=0; checklist2[i].format; ++i ) {
		unsigned int j;
		for ( j=checklist2[i].num_min; j<=checklist2[i].num_max; ++j ) {
			sprintf(drive, checklist2[i].format, j);
			if ( (is_cdrom_linux(drive, NULL)) > 0 ) {
				std::string str = drive;
				drives.push_back(str);
			}
		}
	}
	return drives;
}
#endif

// Returns a pointer to an array of strings with the device names
std::vector<std::string> cdio_get_devices() {
#ifdef _WIN32
	return cdio_get_devices_win32();
#elif __APPLE__
	return cdio_get_devices_osx();
#elif __linux__
	return cdio_get_devices_linux();
#else
#warning CDIO not supported on your platform!
#endif
}

// Need to be tested, does calling this function twice cause any damage?

// Returns true if device is cdrom/dvd

bool cdio_is_cdrom(std::string device) {
	std::vector<std::string> devices = cdio_get_devices();
	bool res = false;
	for (unsigned int i = 0; i < devices.size(); i++) {
		if (strncmp(devices[i].c_str(), device.c_str(), PATH_MAX) == 0) {
			res = true;
			break;
		}
    }
	
	devices.clear();
	return res;
}


// Can we remove this?
/*
int main() {
    char** res = cdio_get_devices();
    int i = 0;
    for (i = 0; res[i] != NULL; i++) {
        printf("%s\n", res[i]);
    }
	cdio_free_device_list(res);
    return 0;
}
*/
