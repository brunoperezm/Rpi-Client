// Parto de drv3 para realizar las modificaciones a rea y write para ver qué hace
// estoy yendo en la dirección de clipboard.c ..!!

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>


#define BUF_SIZE            512
#define PIN_DHT             4


static dev_t first; 		// Global variable for the first device number
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class

static char c;

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "city_bike: open()\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "city_bike: close()\n");
    return 0;
}

// ssize_t resulta ser una palabra con signo.
// Por lo tanto, puede ocurrir que devuelva un número negativo. Esto sería un error. 
// Pero un valor de retorno no negativo tiene un significado adicional. 
// Para my_read sería el número de bytes leídos

// Cuando hago un $ cat /dev/SdeC_drv3, se convoca a my_read.!!
// my_read lee "len" bytes, los guarda en "buf" y devuelve la cantidad leida, que puede
// ser menor, pero nunca mayor que len.

// En SdeC_drv3, devuelve cero. Dado que es un archivo, esto significa no hay mas datos ó EOF.
// Lo que tendría que ocurrir es que el device escriba sobre buf para que el usuario pueda 
// obtener una lectura no nula.

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "city_bike: read()\n");

    if (*off == 0)
    {
        if ( copy_to_user(buf, &c, 1) != 0)
            return -EFAULT;
        else
        {
            (*off)++;
            return 1;
        }
    }
    else
        return 0;

}

/**
 * Esta funcion permite escribir un pin de la RPI.
 * Soporta 2 comandos:
 * Commando             Descripcion
 * high {pin}           Setea el GPIO {pin} en modo output y en alto
 * low  {pin}           Setea el GPIO {pin} en modo output y en bajo
 *
 */

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    char kbuf[BUF_SIZE];
    printk(KERN_INFO "city_bike: write()\n");

    if ( copy_from_user(kbuf, buf, len) != 0 )
        return -EFAULT;
    else {
        kbuf[len] = '\0';
        
        int gpioPin;
        if (sscanf (kbuf, "high %d",&gpioPin) == 1) {
            printk(KERN_INFO "city_bike: entro a high\n");
            gpio_request(gpioPin, "PIN_DHT");
	        gpio_direction_output(gpioPin, 1);
        }
        else if (sscanf (kbuf, "low %d",&gpioPin) == 1) {
            printk(KERN_INFO "city_bike: entro a low\n");
            gpio_request(gpioPin, "PIN_DHT");
	        gpio_direction_output(gpioPin, 0);
        }
        else {
            printk(KERN_INFO "city_bike: no se reconocio comando\n");
        }
        return len;
    }
}

static struct file_operations pugs_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init drv4_init(void) /* Constructor */
{
    int ret;
    struct device *dev_ret;

    printk(KERN_INFO "city_bike: Registrado exitosamente..!!\n");

    if ((ret = alloc_chrdev_region(&first, 0, 1, "city_bike")) < 0)
    {
        return ret;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "gpio_rpi_class")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "city_bike")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &pugs_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }
    return 0;
}

static void __exit drv4_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "city_bike: dice Adios mundo cruel..!!\n");
}

module_init(drv4_init);
module_exit(drv4_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grupo: Perez, Pardina, Benso, Depetris, Briasco");
MODULE_DESCRIPTION("Driver de caracter para controlar GPIO");