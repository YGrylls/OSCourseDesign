
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
#include "proto.h"


void clear()
{
	clear_screen(0, console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;

}
/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct task* p_task;
	struct proc* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		/* p_proc->nr_tty		= 0; */

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

        /* proc_table[NR_TASKS + 0].nr_tty = 0; */
        /* proc_table[NR_TASKS + 1].nr_tty = 1; */
        /* proc_table[NR_TASKS + 2].nr_tty = 1; */

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	char tty_name[] = "/dev_tty0";

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];
//	clear();
	printf("=========================================================================\n");
	printf("                             simpleOS \n");
	printf("                                based on Oranges' \n");
	printf("                 modified by \n");
	printf("                      1652668 Zhang Jialuo \n");
	printf("                      1652669 Yu Yang \n");
	printf("=========================================================================\n");
	printf("Enter -help for command list \n");

	while (1) {
		printf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if(strcmp(rdbuf, "-help") == 0)
			printf("No information\n");
		else if (strcmp(rdbuf, "hello") == 0)
			printf("hello world!\n");
		else
			if (rdbuf[0])
				printf("{%s}\n", rdbuf);
	}

	assert(0); /* never arrive here */
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	char tty_name[] = "/dev_tty1";

	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	
	printf("    *    ********************************************************         \n");
	printf("    *                    simpleOS's File Manager                          \n");
	printf("    *              touch ------ create a new  file                        \n");
	printf("    *              mkdir ------ create a new folder                       \n");
	printf("    *              ls --------- display all the files of current path     \n");
//	printf("    *              open -------open a existed file      \n");
	printf("    *              cd --------- open a folder or an existed file              \n");
//	printf("    *              cd .. ------ return to the superior level              \n");
//	printf("    *              cd / ------- return to root                            \n");
	printf("    *              rm --------- delete a file/folder                      \n");
//	printf("    *              sv --------- save changes                              \n");
	printf("    *              view ------ get the list of opened files               \n");
	printf("    *              close ------ close file with FD              \n");
	printf("    *              help ------- Instruction prompt                        \n");
	printf("    *    ********************************************************         \n");

	char rdbuf[128];
	char filename[MAX_FILENAME_LEN];
	char foldername[MAX_FILENAME_LEN];
	char name[MAX_FILENAME_LEN];
	char currentPath[MAX_PATH];
	char ordernum[10] = { 0 };
	int onum;

	while (1) {

		memset(foldername, 0, sizeof(foldername));
		memset(filename, 0, sizeof(filename));
		memset(name, 0, sizeof(name));
		memset(currentPath, 0, MAX_PATH);
		memset(ordernum, 0, 10);
		onum = 0;
		memcpy(currentPath, open_name,12);
		printf("$%s/ > ",currentPath);
		memset(rdbuf, 0, 128);
		int cnum = read(fd_stdin, rdbuf, 70);
		rdbuf[cnum] = 0;
		assert(fd_stdin == 0);
		if (rdbuf[0] == 't' && rdbuf[1] == 'o' && rdbuf[2] == 'u' && rdbuf[3] == 'c' && rdbuf[4] == 'h')
		{
			if (rdbuf[5] != ' ') 
			{
				printf("Please enter file name like : touch XXX\n");
				continue;
			}
			_Bool isempty = 1;
			_Bool islegal = 1;
			for (int i = 0; i < MAX_FILENAME_LEN && i < cnum - 6; i++) 
			{
				if (rdbuf[i + 6] != ' ')
				{
					isempty = 0;
				}
				if (rdbuf[i+6] == '/')
				{
					islegal = 0;
					break;
				}
				
				filename[i] = rdbuf[i + 6];
			}
			if (islegal == 0)
			{
				printf("/ cannot appear in file name\n");
				memset(filename, 0, sizeof(filename));
				continue;
			}
			if (isempty==1)
			{
				printf("file name cannot be empty\n");
				memset(filename, 0, sizeof(filename));
				continue;
			}
			else
			{
				printf("get filename %s ,creating\n", filename);//换成create函数
				int fd = open(filename, O_CREAT);
				if (fd != -1) {
					printf("created & opened success FD= %d\n", fd);
				}
				else {
					printf("failed\n");
				}
				memset(filename, 0, sizeof(filename));
			}
			
		}

		else if (rdbuf[0] == 'm' && rdbuf[1] == 'k' && rdbuf[2] == 'd' && rdbuf[3] == 'i' && rdbuf[4] == 'r') 
		{
			if (rdbuf[5] != ' ') {
				printf("Please enter folder name like : mkdir XXX\n");
				continue;
			}
			_Bool isempty = 1;
			_Bool islegal = 1;
			for (int i = 0; i < MAX_FILENAME_LEN && i < cnum - 6; i++) 
			{
				if (rdbuf[i + 6] != ' ')
				{
					isempty = 0;
				}
				if (rdbuf[i] == '/')
				{
					islegal = 0;
					break;
				}
				foldername[i] = rdbuf[i + 6];
			}
			if (islegal == 0)
			{
				printf("/ cannot appear in folder name\n");
				memset(foldername, 0, sizeof(foldername));
				continue;
			}
			if (isempty==1)
			{
				printf("folder name cannot be empty\n");
				memset(foldername, 0, sizeof(foldername));
				continue;
			}
			else
			{

				printf("get pathname %s ,creating\n", foldername);//换成create函数
				int fd = open(foldername, O_MKDR);
				if (fd != -1) {

					printf("created success\n");
				}
				else {
					printf("failed\n");
				}
				memset(foldername, 0, sizeof(foldername));
			}
		}

		else if (rdbuf[0] == 'l' && rdbuf[1] == 's')
		{
			_Bool flag = 1;
			for (int i = 2; i < cnum; i++)
			{
				if (rdbuf[i] != ' ')
				{
					flag = 0;
				}
			}
			if (flag==0)
			{
				continue;
			}
			else
			{
				printf("Display all files\n");//换成display函数
				int nr = ls();
				if (nr > 0) {
					int i = 0;
					for (i = 0; i < nr; i++) {
						printf(">  %s", dir_ls[i].name);
						struct inode* temp;
						if ((temp=get_inode(ROOT_DEV, dir_ls[i].inode_nr))->i_mode==I_DIRECTORY) {
							printf("/");
							
						}
						put_inode(temp);
						printf("\n");
					}
					printf("\n");
				}
				else {
					printf("nothing here, so lonely. there must be something wrong...\n");
				}
			}
		}

		else if (rdbuf[0] == 'c' && rdbuf[1] == 'd')  //cd
		{
			if (rdbuf[2] != ' ') 
			{
				printf("Please enter folder name like : cd XXX\n");
				continue;
			}
			for (int i = 0; i < MAX_PATH && i < cnum - 3; i++)
			{
				name[i] = rdbuf[i + 3];
			}
			int temp = open(name, O_RDWR);
			if (temp==0) {

				printf("open  %s \n", name);

			}
			else if (temp > 0) {
				printf("opened success FD = %d\n",temp);
			}
			
			else {
				printf("failed\npath wrong or not existed? MAKE SURE the path DOSE NOT end with /\n");
			}
			memset(name, 0, sizeof(name));
		}

		else if (rdbuf[0] == 'r' && rdbuf[1] == 'm')
		{
			if (rdbuf[2] != ' ') 
			{
				printf("Please enter name like : rm XXX\n");
				continue;
			}
			for (int i = 0; i < 20 && i < cnum - 3; i++)
			{
				name[i] = rdbuf[i + 3];
			}
			printf("Delete %s \n", name);
			int temp = unlink(name);
			if (temp == 0) {
				printf("success\n");
			}
			else {
				printf("failed\npath wrong or file opend? NOTICE THAT only empty directory can be removed\n");
			}
			memset(name, 0, sizeof(name));
		}


		else if (rdbuf[0] == 'h' && rdbuf[1] == 'e' && rdbuf[2] == 'l' && rdbuf[3] == 'p' && cnum == 4)
		{
			printf("    *    ********************************************************         \n");
			printf("    *                    simpleOS's File Manager                          \n");
			printf("    *              touch ------ create a new text file                    \n");
			printf("    *              mkdir ------ create a new folder                       \n");
			printf("    *              ls --------- display all the files of current path     \n");
			printf("    *              cd --------- open a folder                             \n");
//			printf("    *              cd .. ------ return to the superior level              \n");
//			printf("    *              cd / ------- return to root                            \n");
			printf("    *              rm --------- delete a file/folder                      \n");
//			printf("    *              sv --------- save changes                              \n");
			printf("    *              view ------ get the list of opened files               \n");
			printf("    *              close ------ close file with FD              \n");
			printf("    *              help ------- Instructions                              \n");
			printf("    *    ********************************************************         \n");
		}

		else if (rdbuf[0] == 'v' && rdbuf[1] == 'i' && rdbuf[2] == 'e' && rdbuf[3] == 'w')
		{
			_Bool flag = 1;
			for (int i =4; i < cnum; i++)
			{
				if (rdbuf[i] != ' ')
				{
					flag = 0;
				}
			}
			if (flag == 0)
			{
				continue;
			}
			else
			{
				printf("list of opened files\nFD---inode---cnt\n");//
				int i = 0;
				for (i = 0; i < NR_FILE_DESC; i++) {
					struct file_desc* temp = lastcaller->filp[i];
					if (temp != 0) {
						printf("> %d---%d---%d\n", i, temp->fd_inode,temp->fd_inode->i_cnt);
					}
				}
			}

		}
		else if (rdbuf[0] == 'c' && rdbuf[1] == 'l' && rdbuf[2] == 'o' && rdbuf[3] == 's' && rdbuf[4] == 'e')
		{
			if (rdbuf[5] != ' ') {
				printf("Please enter like : close 1\n");
				continue;
			}
			_Bool islegal = 1;
			int count = 0;
			for (int i = 0; i < cnum - 6 && count < 10; i++)
			{
				
				if (rdbuf[i + 6] == ' ') {

				}
				else if ((rdbuf[i + 6] < '0')||(rdbuf[i + 6] > '9'))
				{
					islegal = 0;
					break;
				}
				else if (rdbuf[i + 6] >= '0'&&rdbuf[i + 6] <= '9')
				{
					ordernum[i] = rdbuf[i + 6];
					count++;
				}
			}
			if (islegal == 0)
			{
				printf("input error,please input order number\n");
				continue;
			}
			else
			{
				for (int i = 0; i < sizeof(ordernum); i++)
				{
					if (ordernum[i] <= '9'&&ordernum[i] >= '0')
					{
						onum = onum * 10 + ordernum[i] - '0';
					}
				}
				printf("the order number is %d\n", onum);
				if (onum <= 1) {
					printf("NOPE, you CANNOT close this\n");
					continue;
				}
				if (close(onum) == 0) {
					printf("FD %d closed\n", onum);
				}
				else {
					printf("failed\n");
				}
			}
		}

		
	}

	assert(0); /* never arrive here */
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	spin("TestC");
	/* assert(0); */
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

