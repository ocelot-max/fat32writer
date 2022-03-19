#ifndef _FILE_H
#define _FILE_H

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    // 目录项
    int8_t          file_name[8];
    // 短文件名
    /* 长文件名规则:
     *  A：系统取长文件名的前6个字符加上“~1”，形成短文件名，其扩展名保持不变
     *  B：如果已经存在这个名字的文件，数字自动加1，“~2”
     *  C：如果有DOS与Winddows 3.x的非法字符，以“­_”替代。
     *  系统将长文件名以13个字符为单位进行切割，每一组占据一个目录项
     *  所以可能一个文件需要多个目录项，这时长文件名的各个目录项按倒序排列在目录表中，以防与其他文件名混淆
     *  长文件名的第一部分距离短文件名目录项是最近的
    */
    int8_t          file_ext[3];
    // 短拓展名
    int8_t          file_type;
    // 文件属性
    int8_t          save;
    // 未用
    int8_t          file_create_ctime;
    // 创建时间的10毫秒位(比如134->1340ms)
    int16_t         file_create_time;
    // 文件创建时间(b0-b4)*2s(+file_create_time):(b5-b10)min:(b11-b15)h
    int16_t         file_create_date;
    // 文件创建日期(b0-b4)d:(b5-b8)m:(b9-b15)y(+1980)
    int16_t         file_last_date;
    // 文件最后访问日期(同上)
    int16_t         file_start_sector_high;
    // 文件起始簇号的高16位
    int16_t         file_edit_time;
    // 文件最近修改时间
    int16_t         file_edit_date;
    // 文件最近修改日期
    int16_t         file_start_sector_low;
    // 文件起始簇号的低16位
    int32_t         file_length;
    // 文件长度
    
} __attribute__((packed))
dir_desc;

/* dir_desc 说明: */
    /* file_name */
#define SHORT_NAME_SPACE        0x20
// 短文件名不足8字节时的填充符
        /* 第一个字节也可以表述目录项状态 */
#define SHORT_NAME_UNUSED       0x00
// 从未用过的目录项
#define SHORT_NAME_DELETE       0xe5
// 被删除的目录项

    /* file_type */
#define TYPE_FILE_RW            00000000b
// 读写文件
#define TYPE_FILE_R             00000001b
// 只读文件
#define TYPE_DL                 00000010b
// 隐藏文件
#define TYPE_OS                 00000100b
// 系统文件
#define TYPE_TYPE               00001000b
// 卷标
#define TYPE_DIR                00010000b
// 子目录
#define TYPE_TAR                00100000b
// 存档
#define TYPE_LONG_NAME          0x0f
// 长文件名项

typedef struct
{
    int8_t          info;
    /* 杂信息 
     * b7,b5:保留未用,b6:1->长文件名的最后一个目录项,b0-b4(顺序号,l(max)=424bits)
    */
   int8_t           long_name_low[10];
   // 长文件名(unicode码)
   /* 如果文件名结束但还有未使用的字节，则会在文件名后先填充两个字节的“00”，
    *然后开始使用 0xFF 填充
    */
   int8_t           flag;
   // 长文件名标志(0x0f)
    int8_t          save;
    // 保留
    int8_t          check;
    // 校验值
    int8_t          long_name_med[12];
    // 长文件名(unicode码)
    int16_t         start_sector;
    // 文件起始簇号(目前置0)
    int8_t         long_name_high[4];
    // 长文件名(unicode码)
} __attribute__((packed))
ext_name_dir_desc;

/* 卷标目录项同一般目录项
 * 文件名->卷标(unicode码)
 *  如果创建文件系统时指定了卷标，则会在根目录下第一个目录项的位置建立一个卷标目录项
 */

typedef struct
// 时间定义
{
    unsigned short second;  // s
    unsigned short minute;  // min
    unsigned short hour;    // hour
}_time;

typedef struct
// 日期定义
{
    unsigned short day;
    unsigned short month;
    unsigned short year;
}_date;

typedef struct
// 文件内部描述符
{
    struct scan_file *parent;
    // 父文件夹
    unsigned long dir_n;
    // 目录项中的起始项(指短文件名那一项)
    unsigned char isExist;
    // 是否启用
    char *file_name;
    // 文件名
    char *file_ext;
    // 文件拓展名
    _time create_time;
    // 创建时间
    _date create_date;
    // 创建日期
    _date write_ldate;
    // 最后修改日期
    _date call_ldate;
    // 最后访问日期
    unsigned long start_sector;
    // 起始簇号
    unsigned int file_length;
    // 文件大小
} scan_file;

#endif