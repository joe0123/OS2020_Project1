#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage int sys_my_printk(const char* message){
    printk(message);
    return 0;
}
