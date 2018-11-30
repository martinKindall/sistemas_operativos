/* Necessary includes for device drivers */
#include <linux/init.h>
/* #include <linux/config.h> */
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/uaccess.h> /* copy_from/to_user */

#include "kmutex.h"

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of functions */
static int syncwrite_open(struct inode *inode, struct file *filp);
static int syncwrite_release(struct inode *inode, struct file *filp);
static ssize_t syncwrite_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t syncwrite_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

void syncwrite_exit(void);
int syncwrite_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations syncwrite_fops = {
  read: syncwrite_read,
  write: syncwrite_write,
  open: syncwrite_open,
  release: syncwrite_release
};

/* Declaration of the init and exit functions */
module_init(syncwrite_init);
module_exit(syncwrite_exit);

/*** El driver para escrituras sincronas *************************************/

#define TRUE 1
#define FALSE 0

/* Global variables of the driver */

int syncwrite_major = 65;     /* Major number */

/* Buffer to store data */
#define MAX_SIZE 8192

// globales locales
static char *syncwrite_buffer_0;
static char *syncwrite_buffer_1;

static ssize_t curr_size_0;
static ssize_t curr_size_1;

static int readers;
static int writing;
static int pend_open_write;

/* El mutex y la condicion para syncwrite */
static KMutex mutex;
static KCondition cond;


int syncwrite_init(void) {
  int rc;

  /* Registering device */
  rc = register_chrdev(syncwrite_major, "syncwrite", &syncwrite_fops);
  if (rc < 0) {
    printk(
      "<1>syncwrite: cannot obtain major number %d\n", syncwrite_major);
    return rc;
  }

  readers= 0;
  writing= FALSE;
  pend_open_write= 0;
  curr_size= 0;
  m_init(&mutex);
  c_init(&cond);

  /* Allocating syncwrite_buffer */
  syncwrite_buffer_0 = kmalloc(MAX_SIZE, GFP_KERNEL);
  syncwrite_buffer_1 = kmalloc(MAX_SIZE, GFP_KERNEL);

  if (syncwrite_buffer_0 == NULL || syncwrite_buffer_1 == NULL) {
    syncwrite_exit();
    return -ENOMEM;
  }
  
  memset(syncwrite_buffer_0, 0, MAX_SIZE);
  memset(syncwrite_buffer_1, 0, MAX_SIZE);

  printk("<1>Inserting syncwrite module\n");
  return 0;
}


void syncwrite_exit(void) {
	/* Freeing the major number */
  unregister_chrdev(syncwrite_major, "syncwrite");

  /* Freeing buffer syncread */
  if (syncwrite_buffer_0) {
    kfree(syncwrite_buffer_0);
  }

  if (syncwrite_buffer_1) {
    kfree(syncwrite_buffer_1);
  }

  printk("<1>Removing syncwrite module\n");
}


int syncwrite_open(struct inode *inode, struct file *filp) {
	// int rc= 0;
	m_lock(&mutex);

  	if (filp->f_mode & FMODE_WRITE) {
  		
  		printk("El minor es %d\n", minor);

  		/* Se debe esperar hasta que no hayan otros lectores o escritores */
	    pend_open_write++;
	    while (writing || readers>0) {
	      if (c_wait(&cond, &mutex)) {
	        pend_open_write--;
	        c_broadcast(&cond);
	        rc= -EINTR;
	        goto epilog;
	      }
	    }
	    writing= TRUE;
	    pend_open_write--;
	    curr_size= 0;
	    c_broadcast(&cond);
	    printk("<1>open for write successful\n");
  	}
	else if (filp->f_mode & FMODE_READ) {
		/* Para evitar la hambruna de los escritores, si nadie esta escribiendo
		* pero hay escritores pendientes (esperan porque readers>0), los
	   	* nuevos lectores deben esperar hasta que todos los lectores cierren
	   	* el dispositivo e ingrese un nuevo escritor.
	   	*/
	  // 	while (!writing && pend_open_write>0) {
	  //   	if (c_wait(&cond, &mutex)) {
	  //     	rc= -EINTR;
	  //     	goto epilog;
	  //   	}
	  // 	}
	 	// readers++;
		printk("<1>open for read\n");
	}

	epilog:
		m_unlock(&mutex);
		return rc;
}


int syncwrite_release(struct inode *inode, struct file *filp) {
	printk("<1>release %p\n", filp);
	return 0;
}


ssize_t syncwrite_read(struct file *filp, char *buf,
                    size_t count, loff_t *f_pos) {
	return 0;
}


ssize_t syncwrite_write( struct file *filp, const char *buf,
                      size_t count, loff_t *f_pos) {

	int minor = iminor(filp->f_path.dentry->d_inode);

	ssize_t rc;
	loff_t last;

	m_lock(&mutex);

	last= *f_pos + count;
	if (last>MAX_SIZE) {
		count -= last-MAX_SIZE;
	}
	printk("<1>write %d bytes at %d\n", (int)count, (int)*f_pos);

	if (minor == 1){

	}

	/* Transfiriendo datos desde el espacio del usuario */
	if (copy_from_user(syncread_buffer+*f_pos, buf, count)!=0) {
		/* el valor de buf es una direccion invalida */
		rc= -EFAULT;
		goto epilog;
	}

	*f_pos += count;
	curr_size= *f_pos;
	rc= count;
	c_broadcast(&cond);

	epilog:
		m_unlock(&mutex);
		return rc;
}