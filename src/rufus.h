/*
 * Rufus: The Reliable USB Formatting Utility
 * Copyright © 2011-2014 Pete Batard <pete@akeo.ie>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <windows.h>
#include <winioctl.h>               // for DISK_GEOMETRY
#include <malloc.h>
#include <stdint.h>

#if defined(_MSC_VER)
// Disable some VS2012 Code Analysis warnings
#pragma warning(disable: 28159)		// VS2012 wants us to use GetTickCount64(), but it's not available on XP
#endif

#pragma once

/* Program options */
#define RUFUS_DEBUG                 // print debug info to Debug facility
/* Features not ready for prime time and that may *DESTROY* your data - USE AT YOUR OWN RISKS! */
// #define RUFUS_TEST

#define APPLICATION_NAME            "Rufus"
#define COMPANY_NAME                "Akeo Consulting"
#define STR_NO_LABEL                "NO_LABEL"
#define LEFT_TO_RIGHT_MARK          "‎"			// Yes, there is a character between the quotes!
#define RIGHT_TO_LEFT_MARK          "‏"
#define DRIVE_ACCESS_TIMEOUT        15000		// How long we should retry drive access (in ms)
#define DRIVE_ACCESS_RETRIES        60			// How many times we should retry
#define DRIVE_INDEX_MIN             0x00000080
#define DRIVE_INDEX_MAX             0x000000C0
#define MAX_DRIVES                  (DRIVE_INDEX_MAX - DRIVE_INDEX_MIN)
#define MAX_TOOLTIPS                128
#define MAX_SIZE_SUFFIXES           6			// bytes, KB, MB, GB, TB, PB
#define MAX_CLUSTER_SIZES           18
#define MAX_PROGRESS                (0xFFFF-1)	// leave room for 1 more for insta-progress workaround
#define MAX_LOG_SIZE                0x7FFFFFFE
#define MAX_GUID_STRING_LENGTH      40
#define MAX_GPT_PARTITIONS          128
#define MAX_SECTORS_TO_CLEAR        128			// nb sectors to zap when clearing the MBR/GPT (must be >34)
#define MBR_UEFI_MARKER             0x49464555	// 'U', 'E', 'F', 'I', as a 32 bit little endian longword
#define WRITE_RETRIES               3
#define FS_DEFAULT                  FS_FAT32
#define BADBLOCK_PATTERNS           {0xaa, 0x55, 0xff, 0x00}
#define LARGE_FAT32_SIZE            (32*1073741824LL)	// Size at which we need to use fat32format
#define UDF_FORMAT_SPEED            3.1f		// Speed estimate at which we expect UDF drives to be formatted (GB/s)
#define UDF_FORMAT_WARN             20			// Duration (in seconds) above which we warn about long UDF formatting times
#define MAX_FAT32_SIZE              2.0f		// Threshold above which we disable FAT32 formatting (in TB)
#define DD_BUFFER_SIZE              65536		// Size of the buffer we use for DD operations
#define WHITE                       RGB(255,255,255)
#define SEPARATOR_GREY              RGB(223,223,223)
#define RUFUS_URL                   "http://rufus.akeo.ie"
#define DOWNLOAD_URL                RUFUS_URL "/downloads"
#define FILES_URL                   RUFUS_URL "/files"
#define SEVENZIP_URL                "http://sourceforge.net/projects/sevenzip/files/7-Zip/"
#define FILES_DIR                   "rufus_files"
#define IGNORE_RETVAL(expr)         do { (void)(expr); } while(0)
#ifndef ARRAYSIZE
#define ARRAYSIZE(A)                (sizeof(A)/sizeof((A)[0]))
#endif
#define IsChecked(CheckBox_ID)      (IsDlgButtonChecked(hMainDialog, CheckBox_ID) == BST_CHECKED)
#define MB_IS_RTL                   (right_to_left_mode?MB_RTLREADING:0)
#define IDD_IS_RTL                  (right_to_left_mode?100:0)

#define safe_free(p) do {free((void*)p); p = NULL;} while(0)
#define safe_min(a, b) min((size_t)(a), (size_t)(b))
#define safe_strcp(dst, dst_max, src, count) do {memcpy(dst, src, safe_min(count, dst_max)); \
	((char*)dst)[safe_min(count, dst_max)-1] = 0;} while(0)
#define safe_strcpy(dst, dst_max, src) safe_strcp(dst, dst_max, src, safe_strlen(src)+1)
#define safe_strncat(dst, dst_max, src, count) strncat(dst, src, safe_min(count, dst_max - safe_strlen(dst) - 1))
#define safe_strcat(dst, dst_max, src) safe_strncat(dst, dst_max, src, safe_strlen(src)+1)
#define safe_strcmp(str1, str2) strcmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_strstr(str1, str2) strstr(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_stricmp(str1, str2) _stricmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_strncmp(str1, str2, count) strncmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2), count)
#define safe_strnicmp(str1, str2, count) _strnicmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2), count)
#define safe_closehandle(h) do {if ((h != INVALID_HANDLE_VALUE) && (h != NULL)) {CloseHandle(h); h = INVALID_HANDLE_VALUE;}} while(0)
#define safe_unlockclose(h) do {if ((h != INVALID_HANDLE_VALUE) && (h != NULL)) {UnlockDrive(h); CloseHandle(h); h = INVALID_HANDLE_VALUE;}} while(0)
#define safe_sprintf(dst, count, ...) do {_snprintf(dst, count, __VA_ARGS__); (dst)[(count)-1] = 0; } while(0)
#define static_sprintf(dst, ...) safe_sprintf(dst, sizeof(dst), __VA_ARGS__)
#define safe_strlen(str) ((((char*)str)==NULL)?0:strlen(str))
#define safe_strdup _strdup
#if defined(_MSC_VER)
#define safe_vsnprintf(buf, size, format, arg) _vsnprintf_s(buf, size, _TRUNCATE, format, arg)
#else
#define safe_vsnprintf vsnprintf
#endif

#ifdef RUFUS_DEBUG
extern void _uprintf(const char *format, ...);
#define uprintf(...) _uprintf(__VA_ARGS__)
#define vuprintf(...) if (verbose) _uprintf(__VA_ARGS__)
#define vvuprintf(...) if (verbose > 1) _uprintf(__VA_ARGS__)
#ifdef _CRTDBG_MAP_ALLOC
// Use the _CRTDBG as our general debug flag
#define duprintf(...) _uprintf(__VA_ARGS__)
#else
#define duprintf(...)
#endif
#else
#define uprintf(...)
#define vuprintf(...)
#define vvuprintf(...)
#define duprintf(...)
#endif

/* Custom Windows messages */
enum user_message_type {
	UM_FORMAT_COMPLETED = WM_APP,
	UM_MEDIA_CHANGE,
	// TODO: relabel "ISO" to a more generic "progress"
	UM_ISO_CREATE,
	UM_ISO_INIT,
	UM_ISO_EXIT,
	// Start of the WM IDs for the language menu items
	UM_LANGUAGE_MENU = WM_APP + 0x100
};

/* Custom notifications */
enum notification_type {
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR,
	MSG_QUESTION,
};
typedef INT_PTR (CALLBACK *Callback_t)(HWND, UINT, WPARAM, LPARAM);
typedef struct {
	WORD id;
	Callback_t callback;
} notification_info;	// To provide a "More info..." on notifications

/* Timers used throughout the program */
enum timer_type {
	TID_MESSAGE = 0x1000,
	TID_BADBLOCKS_UPDATE,
	TID_APP_TIMER,
	TID_BLOCKING_TIMER,
	TID_REFRESH_TIMER
};

/* Action type, for progress bar breakdown */
enum action_type {
	OP_ANALYZE_MBR,
	OP_BADBLOCKS,
	OP_ZERO_MBR,
	OP_PARTITION,
	OP_FORMAT,
	OP_CREATE_FS,
	OP_FIX_MBR,
	OP_DOS,
	OP_FINALIZE,
	OP_MAX
};

/* File system indexes in our FS combobox */
enum {
	FS_UNKNOWN = -1,
	FS_FAT16 = 0,
	FS_FAT32,
	FS_NTFS,
	FS_UDF,
	FS_EXFAT,
	FS_REFS,
	FS_MAX
};

enum dos_type {
	DT_WINME = 0,
	DT_FREEDOS,
	DT_ISO,
	DT_IMG,
	DT_SYSLINUX_V4,		// Start of indexes that only display in advanced mode
	DT_SYSLINUX_V5,
	DT_REACTOS,
	DT_MAX
};

enum bios_type {
	BT_BIOS = 0,
	BT_UEFI,
	BT_MAX
};
// For the partition types we'll use Microsoft's PARTITION_STYLE_### constants
#define GETBIOSTYPE(x) (((x)>0)?(((x) >> 16) & 0xFFFF):0)
#define GETPARTTYPE(x) (((x)>0)?((x) & 0xFFFF):0);

/* Current drive info */
typedef struct {
	DWORD DeviceNumber;
	LONGLONG DiskSize;
	DISK_GEOMETRY Geometry;
	DWORD FirstSector;
	char proposed_label[16];
	int PartitionType;
	int nPartitions;
	int FSType;
	BOOL has_protective_mbr;
	BOOL has_mbr_uefi_marker;
	struct {
		ULONG Allowed;
		ULONG Default;
	} ClusterSize[FS_MAX];
} RUFUS_DRIVE_INFO;

/* Special handling for old .c32 files we need to replace */
#define NB_OLD_C32          2
#define OLD_C32_NAMES       { "menu.c32", "vesamenu.c32" }
#define OLD_C32_THRESHOLD   { 53500, 148000 }

/* ISO details that the application may want */
#define WINPE_MININT    0x2A
#define WINPE_I386      0x15
#define HAS_SYSLINUX(r) (r.sl_version != 0)
#define IS_WINPE(r)     (((r&WINPE_MININT) == WINPE_MININT)||((r&WINPE_I386) == WINPE_I386))
#define IS_EFI(r)       ((r.has_efi) || (r.has_win7_efi))
#define IS_REACTOS(r)   (r.reactos_path[0] != 0)

typedef struct {
	char label[192];		/* 3*64 to account for UTF-8 */
	char usb_label[192];	/* converted USB label for workaround */
	char cfg_path[128];		/* path to the ISO's isolinux.cfg */
	char reactos_path[128];	/* path to the ISO's freeldr.sys or setupldr.sys */
	uint64_t projected_size;
	// TODO: use a bitmask and #define tests for the following
	uint8_t winpe;
	BOOL has_4GB_file;
	BOOL has_long_filename;
	BOOL has_symlinks;
	BOOL has_bootmgr;
	BOOL has_efi;
	BOOL has_win7_efi;
	BOOL has_autorun;
	BOOL has_old_c32[NB_OLD_C32];
	BOOL has_old_vesamenu;
	BOOL uses_minint;
	BOOL is_bootable_img;
	uint16_t sl_version;	// Syslinux/Isolinux version
	char sl_version_str[12];
} RUFUS_ISO_REPORT;

/* Isolate the Syslinux version numbers */
#define SL_MAJOR(x) ((uint8_t)((x)>>8))
#define SL_MINOR(x) ((uint8_t)(x))

typedef struct {
	uint16_t version[4];
	uint32_t platform_min[2];		// minimum platform version required
	char* download_url;
	char* release_notes;
} RUFUS_UPDATE;

/* Duplication of the TBPFLAG enum for Windows 7 taskbar progress */
typedef enum TASKBAR_PROGRESS_FLAGS
{
	TASKBAR_NOPROGRESS = 0,
	TASKBAR_INDETERMINATE = 0x1,
	TASKBAR_NORMAL = 0x2,
	TASKBAR_ERROR = 0x4,
	TASKBAR_PAUSED = 0x8
} TASKBAR_PROGRESS_FLAGS;

/* Windows versions */
enum WindowsVersion {
	WINDOWS_UNDEFINED = -1,
	WINDOWS_UNSUPPORTED = 0,
	WINDOWS_XP = 0x51,
	WINDOWS_2003 = 0x52,	// Also XP x64
	WINDOWS_VISTA = 0x60,
	WINDOWS_7 = 0x61,
	WINDOWS_8 = 0x62,
	WINDOWS_8_1_OR_LATER = 0x63,
	WINDOWS_MAX
};

/*
 * Globals
 */
extern HINSTANCE hMainInstance;
extern HWND hMainDialog, hLogDlg, hStatus, hDeviceList, hCapacity;
extern HWND hPartitionScheme, hFileSystem, hClusterSize, hLabel, hBootType, hNBPasses, hLog;
extern HWND hISOProgressDlg, hISOProgressBar, hISOFileName, hDiskID;
extern float fScale;
extern char szFolderPath[MAX_PATH], app_dir[MAX_PATH];
extern char* iso_path;
extern DWORD FormatStatus;
extern DWORD syslinux_ldlinux_len[2];
extern RUFUS_DRIVE_INFO SelectedDrive;
extern const int nb_steps[FS_MAX];
extern BOOL use_own_c32[NB_OLD_C32], detect_fakes, iso_op_in_progress, format_op_in_progress, right_to_left_mode;
extern RUFUS_ISO_REPORT iso_report;
extern int64_t iso_blocking_status;
extern uint16_t rufus_version[4], embedded_sl_version[2];
extern int nWindowsVersion;
extern char WindowsVersionStr[128];
extern char embedded_sl_version_str[2][12];
extern RUFUS_UPDATE update;
extern int dialog_showing;

/*
 * Shared prototypes
 */
extern void GetWindowsVersion(void);
extern BOOL is_x64(void);
extern const char* PrintWindowsVersion(enum WindowsVersion version);
extern const char *WindowsErrorString(void);
extern void DumpBufferHex(void *buf, size_t size);
extern void PrintStatus(unsigned int duration, BOOL debug, int msg_id, ...);
extern void UpdateProgress(int op, float percent);
extern const char* StrError(DWORD error_code, BOOL use_default_locale);
extern char* GuidToString(const GUID* guid);
extern char* SizeToHumanReadable(uint64_t size, BOOL log, BOOL fake_units);
extern void CenterDialog(HWND hDlg);
extern void ResizeMoveCtrl(HWND hDlg, HWND hCtrl, int dx, int dy, int dw, int dh);
extern void CreateStatusBar(void);
extern void SetTitleBarIcon(HWND hDlg);
extern BOOL CreateTaskbarList(void);
extern BOOL SetTaskbarProgressState(TASKBAR_PROGRESS_FLAGS tbpFlags);
extern BOOL SetTaskbarProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal);
extern INT_PTR CreateAboutBox(void);
extern BOOL CreateTooltip(HWND hControl, const char* message, int duration);
extern void DestroyTooltip(HWND hWnd);
extern void DestroyAllTooltips(void);
extern BOOL Notification(int type, const notification_info* more_info, char* title, char* format, ...);
extern BOOL Question(char* title, char* format, ...);
extern BOOL ExtractDOS(const char* path);
extern BOOL ExtractISO(const char* src_iso, const char* dest_dir, BOOL scan);
extern int64_t ExtractISOFile(const char* iso, const char* iso_file, const char* dest_file);
extern BOOL InstallSyslinux(DWORD drive_index, char drive_letter);
DWORD WINAPI FormatThread(void* param);
extern BOOL CreateProgress(void);
extern BOOL SetAutorun(const char* path);
extern char* FileDialog(BOOL save, char* path, char* filename, char* ext, char* ext_desc, DWORD options);
extern BOOL FileIO(BOOL save, char* path, char** buffer, DWORD* size);
extern unsigned char* GetResource(HMODULE module, char* name, char* type, const char* desc, DWORD* len, BOOL duplicate);
extern BOOL SetLGP(BOOL bRestore, BOOL* bExistingKey, const char* szPath, const char* szPolicy, DWORD dwValue);
extern LONG GetEntryWidth(HWND hDropDown, const char* entry);
extern DWORD DownloadFile(const char* url, const char* file, HWND hProgressDialog);
extern HANDLE DownloadFileThreaded(const char* url, const char* file, HWND hProgressDialog);
extern INT_PTR CALLBACK UpdateCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern BOOL SetUpdateCheck(void);
extern BOOL CheckForUpdates(BOOL force);
extern void DownloadNewVersion(void);
extern BOOL IsShown(HWND hDlg);
extern char* get_token_data_file(const char* token, const char* filename);
extern char* get_token_data_buffer(const char* token, unsigned int n, const char* buffer, size_t buffer_size);
extern char* insert_section_data(const char* filename, const char* section, const char* data, BOOL dos2unix);
extern char* replace_in_token_data(const char* filename, const char* token, const char* src, const char* rep, BOOL dos2unix);
extern void parse_update(char* buf, size_t len);
extern BOOL WimExtractCheck(void);
extern BOOL WimExtractFile(const char* wim_image, int index, const char* src, const char* dst);
extern int IsHDD(DWORD DriveIndex, uint16_t vid, uint16_t pid, const char* strid);

static __inline BOOL UnlockDrive(HANDLE hDrive)
{
	DWORD size;
	return DeviceIoControl(hDrive, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &size, NULL);
}

static __inline void *_reallocf(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	if (!ret)
		free(ptr);
	return ret;
}

/* Basic String Array */
typedef struct {
	char** String;
	size_t Index;	// Current array size
	size_t Max;		// Maximum array size
} StrArray;
extern void StrArrayCreate(StrArray* arr, size_t initial_size);
extern void StrArrayAdd(StrArray* arr, const char* str);
extern void StrArrayClear(StrArray* arr);
extern void StrArrayDestroy(StrArray* arr);
#define IsStrArrayEmpty(arr) (arr.Index == 0)

/*
 * typedefs for the function prototypes. Use the something like:
 *   PF_DECL(FormatEx);
 * which translates to:
 *   FormatEx_t pfFormatEx = NULL;
 * in your code, to declare the entrypoint and then use:
 *   PF_INIT(FormatEx, fmifs);
 * which translates to:
 *   pfFormatEx = (FormatEx_t) GetProcAddress(GetDLLHandle("fmifs"), "FormatEx");
 * to make it accessible.
 */
static __inline HMODULE GetDLLHandle(char* szDLLName)
{
	HMODULE h = NULL;
	if ((h = GetModuleHandleA(szDLLName)) == NULL)
		h = LoadLibraryA(szDLLName);
	return h;
}
#define PF_DECL(proc) proc##_t pf##proc = NULL
#define PF_INIT(proc, dllname) pf##proc = (proc##_t) GetProcAddress(GetDLLHandle(#dllname), #proc)
#define PF_INIT_OR_OUT(proc, dllname) \
	PF_INIT(proc, dllname); if (pf##proc == NULL) { \
	uprintf("Unable to locate %s() in %s.dll: %s\n", #proc, #dllname, \
	WindowsErrorString()); goto out; }

/* Clang/MinGW32 has an issue with intptr_t */
#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#ifdef _WIN64
  typedef unsigned __int64 uintptr_t;
#else
  typedef unsigned int uintptr_t;
#endif
#endif

/* Custom application errors */
#define FAC(f)                         (f<<16)
#define APPERR(err)                    (APPLICATION_ERROR_MASK|err)
#define ERROR_INCOMPATIBLE_FS          0x1201
#define ERROR_CANT_QUICK_FORMAT        0x1202
#define ERROR_INVALID_CLUSTER_SIZE     0x1203
#define ERROR_INVALID_VOLUME_SIZE      0x1204
#define ERROR_CANT_START_THREAD        0x1205
#define ERROR_BADBLOCKS_FAILURE        0x1206
#define ERROR_ISO_SCAN                 0x1207
#define ERROR_ISO_EXTRACT              0x1208
#define ERROR_CANT_REMOUNT_VOLUME      0x1209
#define ERROR_CANT_PATCH               0x120A
#define ERROR_CANT_ASSIGN_LETTER       0x120B
#define ERROR_CANT_MOUNT_VOLUME        0x120C

/* More niceties */
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef PBS_MARQUEE
#define PBS_MARQUEE 0x08
#endif
#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE (WM_USER+10)
#endif

/* Why oh why does Microsoft have to make everybody suffer with their braindead use of Unicode? */
#define _RT_ICON			MAKEINTRESOURCEA(3)
#define _RT_RCDATA			MAKEINTRESOURCEA(10)
#define _RT_GROUP_ICON		MAKEINTRESOURCEA((ULONG_PTR)(MAKEINTRESOURCEA(3) + 11))

/* The CM calls used in GetUSBDevices() - from MinGW's cfgmgr32.h header */
typedef DWORD CONFIGRET, DEVINST, *PDEVINST;
typedef CHAR *DEVINSTID_A;
#define CM_GETIDLIST_FILTER_SERVICE 2
DECLSPEC_IMPORT CONFIGRET WINAPI CM_Get_Device_ID_List_SizeA(PULONG pulLen, PCSTR pszFilter, ULONG ulFlags);
DECLSPEC_IMPORT CONFIGRET WINAPI CM_Get_Device_ID_ListA(PCSTR pszFilter, PCHAR Buffer, ULONG  BufferLen, ULONG ulFlags);
DECLSPEC_IMPORT CONFIGRET WINAPI CM_Locate_DevNodeA(PDEVINST pdnDevInst, DEVINSTID_A pDeviceID, ULONG ulFlags);
DECLSPEC_IMPORT CONFIGRET WINAPI CM_Get_Child(PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags);
