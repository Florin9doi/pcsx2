// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#pragma once

#define IOP_ENOENT 2
#define IOP_EIO 5
#define IOP_ENOMEM 12
#define IOP_EACCES 13
#define IOP_ENODEV 19
#define IOP_EISDIR 21
#define IOP_EMFILE 24
#define IOP_EROFS 30

#define IOP_O_RDONLY 0x001
#define IOP_O_WRONLY 0x002
#define IOP_O_RDWR 0x003
#define IOP_O_APPEND 0x100
#define IOP_O_CREAT 0x200
#define IOP_O_TRUNC 0x400
#define IOP_O_EXCL 0x800

#define IOP_SEEK_SET 0
#define IOP_SEEK_CUR 1
#define IOP_SEEK_END 2

typedef struct
{
	u32 mode;
	u32 attr;
	u32 size;
	u8 ctime[8];
	u8 atime[8];
	u8 mtime[8];
	u32 hisize;
} fio_stat_t;
typedef struct
{
	fio_stat_t _fioStat;
	/** Number of subs (main) / subpart number (sub) */
	u32 private_0;
	u32 private_1;
	u32 private_2;
	u32 private_3;
	u32 private_4;
	/** Sector start.  */
	u32 private_5;
} fxio_stat_t;

typedef struct
{
	fio_stat_t stat;
	char name[256];
	u32 unknown;
} fio_dirent_t;

typedef struct
{
	fxio_stat_t stat;
	char name[256];
	u32 unknown;
} fxio_dirent_t;

class IOManFile
{
public:
	static int open(IOManFile** file, const std::string& path, s32 flags, u16 mode)
	{
		return -IOP_ENODEV;
	}

	virtual void close() = 0;

	virtual int lseek(s32 offset, s32 whence) { return -IOP_EIO; }
	virtual int read(void* buf, u32 count) { return -IOP_EIO; } /* Flawfinder: ignore */
	virtual int write(void* buf, u32 count) { return -IOP_EIO; }
};

class IOManDir
{
	// Don't think about it until we know the loaded ioman version.
	// The dirent structure changed between versions.
public:
	static int open(IOManDir** dir, const std::string& full_path)
	{
		return -IOP_ENODEV;
	}

	virtual void close() = 0;

	virtual int read(void* buf, bool iomanX = false, bool realfile = false) { return -IOP_EIO; } /* Flawfinder: ignore */
};

typedef int (*irxHLE)(); // return 1 if handled, otherwise 0
typedef void (*irxDEBUG)();

namespace R3000A
{
	u32 irxFindLoadcore(u32 entrypc);
	u32 irxImportTableAddr(u32 entrypc);
	const char* irxImportFuncname(const std::string& libname, u16 index);
	irxHLE irxImportHLE(const std::string& libnam, u16 index);
	irxDEBUG irxImportDebug(const std::string& libname, u16 index);
	void irxImportLog(const std::string& libnameptr, u16 index, const char* funcname);
	void irxImportLog_rec(u32 import_table, u16 index, const char* funcname);
	int irxImportExec(u32 import_table, u16 index);

	int host_stat(const std::string& path, fxio_stat_t* host_stats, bool realfile);
	int hostdir_open(IOManDir** outDir, const std::string& path);
	int hostfile_open(IOManFile** file, const std::string& path, s32 flags, u16 mode);

	namespace ioman
	{
		void reset();
		bool is_host(const std::string_view path);
		std::string host_path(const std::string_view path, bool allow_open_host_root);
		template <typename T> int allocfd(T* obj);
		template <typename T> T* getfd(int fd);
		void freefd(int fd);
	}
} // namespace R3000A

extern void Hle_SetHostRoot(const char* bootFilename);
extern void Hle_ClearHostRoot();

