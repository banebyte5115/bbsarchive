#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bbsarch.h"

#define _CRT_SECURE_NO_WARNINGS

int string_cmp(char* str1, char* str2) {
	if (strlen(str1) == strlen(str2)) {
		uint i = 0;
		while (i < strlen(str1)) {
			if (str1[i] != str2[i]) {
				return 0;
			}
			++i;
		}
		return 1;
	}
	return 0;
}

uint file_get_size2(FILE* file) {
	fseek(file, 0, SEEK_END);
	uint file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return file_size;
}

ushort bbsarch_hashfunc(char* name, uint bbs_hsize) {
	char* c = name, mov = 0;
	ushort hv = 0;
	do  {
		hv ^= ((*c++) << (mov << 3));
		mov ^= 1;
	} while (*c != 0x00);
	return hv;
}

void bbsarch_create(char* bbs_fn, uint bbs_id, uint bbs_hsize) {
	// creating header
	BBSHeader header;
	header.version = BBS_VERSION;
	header.files = 0;
	header.id = bbs_id;
	header.hashmax = bbs_hsize;
	header.htsize = 0;
	// creating file
	FILE* bbs_f = fopen(bbs_fn, "wb");
	fwrite(&header, sizeof(BBSHeader), 1, bbs_f);
	fclose(bbs_f);
}

void bbsarch_add_file(char* bbs_fn, char* file_fn, char* file_name, uchar file_flag) {
	// creating header
	BBSHeader header;
	// opening files
	FILE* bbs_f = fopen(bbs_fn, "rb+");
	FILE* file_f = fopen(file_fn, "rb");
	// reading header
	fread(&header, sizeof(BBSHeader), 1, bbs_f);
	// allocating memory for hashtable
	BBSHashSlot* ht = (BBSHashSlot*)malloc(sizeof(BBSHashSlot) * header.hashmax);
	memset(ht, 0, sizeof(BBSHashSlot) * header.hashmax);
	// reading hashtable
	uint read_size = 0;
	while (read_size < header.htsize) {
		BBSHashSlot slot;
		read_size += fread(&slot.htpos, 1, 2, bbs_f);
		read_size += fread(&slot.namelen, 1, 1, bbs_f);
		slot.name = new char[slot.namelen];
		read_size += fread(slot.name, 1, slot.namelen, bbs_f);
		read_size += fread(&slot.offset, 1, 4, bbs_f);
		read_size += fread(&slot.size, 1, 4, bbs_f);
		ht[slot.htpos] = slot;
	}
	// copying all data after the hashtable
	uint cur_pos = ftell(bbs_f);
	uint file_size = file_get_size2(bbs_f);
	fseek(bbs_f, cur_pos, SEEK_SET);
	uint to_copy = file_size - cur_pos;
	uchar* cpybuff = (uchar*)malloc(to_copy);
	fread(cpybuff, 1, to_copy, bbs_f);
	// checking whether new entry is passed (to add size of hashtable)
	ushort hv = bbsarch_hashfunc(file_name, header.hashmax);
	if (ht[hv].namelen == 0) {
		// creating entry
		ht[hv].namelen = (uchar)strlen(file_name);
		header.htsize += 2 + 1 + 4 + 4 + ht[hv].namelen;
		ht[hv].htpos = hv;
		ht[hv].name = file_name;
		ht[hv].offset = to_copy;
		ht[hv].size = file_get_size2(file_f);
	}
	// writing header
	header.files += 1;
	fseek(bbs_f, 0, SEEK_SET);
	fwrite(&header, sizeof(BBSHeader), 1, bbs_f);
	// writing hashtable
	for (int i = 0; i < header.hashmax; ++i) {
		if (ht[i].namelen != 0) {
			fwrite(&ht[i].htpos, 2, 1, bbs_f);
			fwrite(&ht[i].namelen , 1, 1, bbs_f);
			fwrite(ht[i].name, ht[i].namelen, 1, bbs_f);
			fwrite(&ht[i].offset, 4, 1, bbs_f);
			fwrite(&ht[i].size, 4, 1, bbs_f);
		}
	}
	// writing all data after the hashtable
	fwrite(cpybuff, 1, to_copy, bbs_f);
	// creating file record
	FileRecord record;
	record.size = file_get_size2(file_f);
	record.namelen = strlen(file_name);
	record.name = file_name;
	record.flag = file_flag;
	// writing file record
	fseek(bbs_f, 0, SEEK_END);
	fwrite(&record.size, 4, 1, bbs_f);
	fwrite(&record.namelen, 1, 1, bbs_f);
	fwrite(record.name, 1, record.namelen, bbs_f);
	fwrite(&record.flag, 1, 1, bbs_f);
	// writing file data
	uchar* buff = new uchar[512 * 1024];
	uint read = 0;
	while ((read = fread(buff, 1, 512 * 1024, file_f)) > 0) {
		fwrite(buff, 1, read, bbs_f);
	}
	fclose(bbs_f);
	fclose(file_f);
}

void bbsarch_get_file(char* bbs_fn, char* file_fn, char* file_name) {
	// creating header
	BBSHeader header;
	// opening files
	FILE* bbs_f = fopen(bbs_fn, "rb");
	FILE* file_f = fopen(file_fn, "wb");
	// reading header
	fread(&header, sizeof(BBSHeader), 1, bbs_f);
	// allocating memory for hashtable
	BBSHashSlot* ht = (BBSHashSlot*)malloc(sizeof(BBSHashSlot) * header.hashmax);
	memset(ht, 0, sizeof(BBSHashSlot) * header.hashmax);
	// reading hashtable
	uint read_size = 0;
	while (read_size < header.htsize) {
		BBSHashSlot slot;
		read_size += fread(&slot.htpos, 1, 2, bbs_f);
		read_size += fread(&slot.namelen, 1, 1, bbs_f);
		slot.name = new char[slot.namelen];
		read_size += fread(slot.name, 1, slot.namelen, bbs_f);
		read_size += fread(&slot.offset, 1, 4, bbs_f);
		read_size += fread(&slot.size, 1, 4, bbs_f);
		ht[slot.htpos] = slot;
	}
	// searching in hashtable
	ushort hv = bbsarch_hashfunc(file_name, header.hashmax);
	char found_flag = 0;
	uint extract_offset = 0;
	uint extract_size = 0;
	if (ht[hv].name == file_name) {
		extract_offset = ht[hv].offset;
		fseek(bbs_f, extract_offset, SEEK_CUR);
		fread(&extract_size, 4, 1, bbs_f);
		uchar extract_namelen = fgetc(bbs_f);
		char extract_name[BBS_NAME_MAX] = { 0 };
		fread(extract_name, 1, extract_namelen, bbs_f);
		uchar extract_flag = fgetc(bbs_f);
		found_flag = 1;
	} else {
		// parsing archive
		uint files = header.files;
		while (files--) {
			fread(&extract_size, 4, 1, bbs_f);
			uchar extract_namelen = fgetc(bbs_f);
			char extract_name[BBS_NAME_MAX] = { 0 };
			fread(extract_name, 1, extract_namelen, bbs_f);
			uchar extract_flag = fgetc(bbs_f);
			if (string_cmp(extract_name, file_name)) {
				found_flag = 1;
				break;
			} else {
				// jump over
				found_flag = 0;
				fseek(bbs_f, extract_size, SEEK_CUR);
			}
		}
	}
	// if found match
	if (found_flag) {
		// extracting data
		uchar* buff = new uchar[BBS_BUFF_MAX];
		uint read = 0;
		while ((read = fread(buff, 1, BBS_BUFF_MAX, bbs_f)) > 0) {
			if (read > extract_size) {
				read = extract_size;
				fwrite(buff, 1, read, file_f);
				break;
			} else {
				fwrite(buff, 1, read, file_f);
			}
			extract_size -= read;
		}
	}
	
	fclose(bbs_f);
	fclose(file_f);
}
