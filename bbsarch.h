#ifndef _BBS_ARCH_
#define _BBS_ARCH_

#define BBS_MAGIC	  (0x58534242)
#define BBS_MAGIC2    (0x42415448)
#define BBS_VERSION   (1001)
#define BBS_NAME_MAX  (255)
#define BBS_BUFF_MAX  (512 * 1024)

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

#pragma pack(push, 1)
typedef struct BBSHashSlot_str {
	ushort htpos;
	uchar namelen;
	char* name;
	uint offset;
	uint size;
} BBSHashSlot;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BBSHeader_str {
	uint signature = BBS_MAGIC;
	ushort version;
	uint files;
	uint id;
	uint hashmax;
	const uint hashsign = BBS_MAGIC2;
	uint htsize;
} BBSHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct FileRecord_str {
	uint size;
	uchar namelen;
	char* name;
	uchar flag;
} FileRecord;
#pragma pack(pop)

ushort bbsarch_hashfunc(char* name, uint bbs_hsize);
void bbsarch_create(char* bbs_fn, uint bbs_id, uint bbs_hsize);
void bbsarch_add_file(char* bbs_fn, char* file_fn, char* file_name, uchar file_flag);
void bbsarch_get_file(char* bbs_fn, char* file_fn, char* file_name);

#endif