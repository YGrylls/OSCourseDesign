/*************************************************************************//**
 *****************************************************************************
 * @file   misc.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2008
 *****************************************************************************
 *****************************************************************************/

/* Orange'S FS */

#include "type.h"
#include "config.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"
#include "hd.h"
#include "fs.h"

/*****************************************************************************
 *                                search_file
 *****************************************************************************/
/**
 * Search the file and return the inode_nr.
 *
 * @param[in] path The full path of the file to search.
 * @return         Ptr to the i-node of the file if successful, otherwise zero.
 * 
 * @see open()
 * @see do_open()
 *****************************************************************************/

PUBLIC int search_file(char * path)
{
	int i, j;

	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	struct inode * dir_inode;
	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;

	if (filename[0] == 0)	/* path: "/" */
		return dir_inode->i_num;

	/**
	 * Search the dir for the file.
	 */
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries =
	  dir_inode->i_size / DIR_ENTRY_SIZE; /**
					       * including unused slots
					       * (the file has been deleted
					       * but the slot is still there)
					       */
	int m = 0;
	struct dir_entry * pde;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)
				return pde->inode_nr;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			break;
	}

	/* file not found */
	return 0;
}

/*****************************************************************************
 *                                strip_path
 *****************************************************************************/
/**
 * Get the basename from the fullpath.
 *
 * In Orange'S FS v1.0, all files are stored in the root directory.
 * There is no sub-folder thing.
 *
 * This routine should be called at the very beginning of file operations
 * such as open(), read() and write(). It accepts the full path and returns
 * two things: the basename and a ptr of the root dir's i-node.
 *
 * e.g. After stip_path(filename, "/blah", ppinode) finishes, we get:
 *      - filename: "blah"
 *      - *ppinode: root_inode
 *      - ret val:  0 (successful)
 *
 * Currently an acceptable pathname should begin with at most one `/'
 * preceding a filename.
 *
 * Filenames may contain any character except '/' and '\\0'.
 *
 * @param[out] filename The string for the result.
 * @param[in]  pathname The full pathname.
 * @param[out] ppinode  The ptr of the dir's inode will be stored here.
 * 
 * @return Zero if success, otherwise the pathname is not valid.
 *****************************************************************************/
PUBLIC int strip_path(char * filename, const char * pathname,
		      struct inode** ppinode)
{
	const char * s = pathname;
	char * t = filename;
	memset(filename, 0, MAX_PATH);
	struct inode* father_level=dir_open;

	if (s == 0)
		return -1;

	if (*s == '/') {     //root directory
		father_level = root_inode;
		s++;
	}
	while (*s) {		/* check each character */
		if (*s == '/') {
			if ((father_level->i_mode & I_TYPE_MASK) != I_DIRECTORY) {
				return -1;                     //if the father_level isn't a directory,failed
			}
			// when comes to a '/'
			*t = 0;
			int dir_blk0_nr = father_level->i_start_sect;
			int nr_dir_blks = (father_level->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
			int nr_dir_entries = father_level->i_size / DIR_ENTRY_SIZE; 
			int m = 0;
			struct dir_entry * pde;
			int i;
			int j;
			for (i = 0; i < nr_dir_blks; i++) {
				memset(fsbuf, 0, SECTOR_SIZE);
				RD_SECT(father_level->i_dev, dir_blk0_nr + i);
				pde = (struct dir_entry *)fsbuf;
				for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {  //iterate dir_entry
					if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)  //found the file or path
					{
					//	printl("@%s %d", filename,pde->inode_nr);//test
						father_level = get_inode(father_level->i_dev, pde->inode_nr);
						//put_inode(father_level);
						break;
					}
					if (++m > nr_dir_entries)
						break;
				}
				if (m > nr_dir_entries) /* all entries have been iterated */
					break;
			}


			//
			memset(t, 0, MAX_FILENAME_LEN); //clear the temp filename
			t = filename;
			s++;

		}
		
		*t++ = *s++;
		/* if filename is too long, just truncate it */
		if (t - filename >= MAX_FILENAME_LEN)
			break;
	}
//	*t = 0;
	
	//find the father-directroy
	assert(father_level->i_mode  == I_DIRECTORY);
//	if (father_level->i_mode != I_DIRECTORY) {
//		printl("%d", father_level->i_num);//test
//	}
	*ppinode = father_level;
	//
	return 0;
}

//ls
//return the number of dir entry under current directory
PUBLIC int dir_list(struct inode* ino, struct dir_entry * list) {
	printl("@dirlist mark1\n");//test
	memset((void*)list, 0, 4096*DIR_ENTRY_SIZE);
	printl("@dirlist mark2\n");//test
	struct dir_entry * pde;
	int i;
	int j;
	int m = 0;
	int n = 0;
	int dir_blk0_nr = ino->i_start_sect;
	int nr_dir_blks = (ino->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries = ino->i_size / DIR_ENTRY_SIZE;
	for (i = 0; i < nr_dir_blks; i++) {
		memset(fsbuf, 0, SECTOR_SIZE);
//		printl("@dirlist mark3 %d\n ",i);//test
		RD_SECT(ino->i_dev, dir_blk0_nr + i);
		printl("@dirlist mark3 %d\n ", i);//test
		pde = (struct dir_entry *)fsbuf;
//		printl("%s\n", pde->name);//test
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {  //iterate dir_entry
			printl("%s\n", pde->name);//test
			if (pde->name[0] != 0) {
				memcpy((u8 *)list + m*DIR_ENTRY_SIZE, fsbuf + m*DIR_ENTRY_SIZE, DIR_ENTRY_SIZE);
			}
			else { n++; }
			if (++m >= nr_dir_entries)
				break;
		}
	
	}
//	printl("@dir_list_m > %d\n", m);//test
	return (m-n);
}