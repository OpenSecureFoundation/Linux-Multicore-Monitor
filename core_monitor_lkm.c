#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cpumask.h>

#define DEVICE_NAME "core_monitor"
#define BUF_SIZE 131072 // 128 Ko

static int major;

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    struct task_struct *task;
    char *kbuf;
    int pos = 0;
    int cpu;

    if (*offset > 0) return 0;
    kbuf = kvzalloc(BUF_SIZE, GFP_KERNEL);
    if (!kbuf) return -ENOMEM;

    pos += snprintf(kbuf + pos, BUF_SIZE - pos, "{\"cores\":[");

    for_each_online_cpu(cpu) {
        pos += snprintf(kbuf + pos, BUF_SIZE - pos, "{\"id\":%d,\"procs\":[", cpu);
        int first = 1;

        rcu_read_lock();
        for_each_process(task) {
            if (task_cpu(task) == cpu) {
                if (!first) pos += snprintf(kbuf + pos, BUF_SIZE - pos, ",");
                pos += snprintf(kbuf + pos, BUF_SIZE - pos, "{\"pid\":%d,\"name\":\"%s\"}",
                                task->pid, task->comm);
                first = 0;
            }
        }
        rcu_read_unlock();
        pos += snprintf(kbuf + pos, BUF_SIZE - pos, "]}%s", (cpu == nr_cpu_ids - 1) ? "" : ",");
    }
    pos += snprintf(kbuf + pos, BUF_SIZE - pos, "]}");

    if (copy_to_user(buf, kbuf, pos)) {
        kvfree(kbuf);
        return -EFAULT;
    }

    kvfree(kbuf);
    *offset = pos;
    return pos;
}

static struct file_operations fops = { .owner = THIS_MODULE, .read = dev_read };

static int __init monitor_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    pr_info("CORE_MONITOR: Major %d\n", major);
    return (major < 0) ? major : 0;
}

static void __exit monitor_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(monitor_init);
module_exit(monitor_exit);
MODULE_LICENSE("GPL");
