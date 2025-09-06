#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void ft_print_offset(int n, char buf[8], int no_flag)
{
    char c;
    char *hex_base;
    int idx;

    idx = 7;
    if (no_flag)
        idx--;
    hex_base = "0123456789abcdef";
    while (n != 0)
    {
        c = hex_base[n % 16];
        buf[idx--] = c;
        n /= 16;
    }
    while (idx >= 0)
        buf[idx--] = '0';
}

void ft_hex_value(int num, char buf[48])
{
    unsigned char first;
    unsigned char second;
    char *hex_base;

    hex_base = "0123456789abcdef";
    first = hex_base[(unsigned char)num / 16];
    second = hex_base[(unsigned char)num % 16];
    buf[0] = first;
    buf[1] = second;
}

void ft_print_line_small(int offset, char *str, int size)
{
    int idx;
    int counter;
    char buf[48];

    idx = 7;
    counter = 0;
    buf[idx++] = ' ';
    ft_print_offset(offset, buf, 1);
    while (counter < size)
    {
        if (counter < size - 1)
            ft_hex_value(str[counter + 1], buf + idx);
        else
            ft_hex_value(0, buf + idx);
        ft_hex_value(str[counter], buf + idx + 2);
        idx += 4;
        if (counter < 14)
            buf[idx++] = ' ';
        counter += 2;
    }
    while (idx < 47)
        buf[idx++] = ' ';
    buf[idx++] = '\n';
    write(1, buf, idx);
}

void ft_write_ascii(int *idx, char *str, char buf[79], int size)
{
    int counter;

    counter = 0;
    buf[(*idx)++] = '|';
    while (counter < size)
    {
        if (str[counter] >= 32 && str[counter] <= 126)
            buf[(*idx)++] = str[counter];
        else
            buf[(*idx)++] = '.';
        counter++;
    }
    buf[(*idx)++] = '|';
    buf[(*idx)++] = '\n';
}

void ft_print_line(int offset, char *str, int size, int no_flag)
{
    int counter;
    char buf[79];
    int idx;

    if (no_flag)
        return (ft_print_line_small(offset, str, size));
    idx = 8;
    ft_print_offset(offset, buf, no_flag);
    buf[idx++] = ' ';
    buf[idx++] = ' ';
    counter = 0;
    while (counter < size)
    {
        ft_hex_value(str[counter], buf + idx);
        idx += 2;
        buf[idx++] = ' ';
        if (counter++ == 7)
            buf[idx++] = ' ';
    }
    while (idx < 60)
        buf[idx++] = ' ';
    ft_write_ascii(&idx, str, buf, size);
    write(1, buf, idx);
}

int ft_strlen(char *str)
{
    int count;

    count = 0;
    while (str[count] != '\0')
        count++;
    return (count);
}

int ft_strcmp(char *s1, char *s2)
{
    int i;

    i = 0;
    while (s1[i] == s2[i] && i < 16)
        i++;
    if (i == 16)
        return (0);
    return (s1[i] - s2[i]);
}

void ft_strcpy(char s1[16], char s2[16])
{
    int count;

    count = 0;
    while (count < 16)
    {
        s1[count] = s2[count];
        count++;
    }
}

void ft_print_error(char *path, char *prog, int for_all)
{
    int prog_len;
    int path_len;
    int error_len;
    char *error;

    prog_len = ft_strlen(prog);
    path_len = ft_strlen(path);
    error = strerror(errno);
    error_len = ft_strlen(error);
    if (for_all == 1)
    {
        write(2, prog, prog_len);
        write(2, ": all input file arguments failed\n", 35);
    }
    else
    {
        write(2, prog, prog_len);
        write(2, ": ", 2);
        write(2, path, path_len);
        write(2, ": ", 2);
        write(2, error, error_len);
        write(2, "\n", 1);
    }
}

void ft_read_to_buf(int fd, char buf[16], int *count, int no_flag)
{
    char old_buf[16];
    int printed_same;
    int to_read;
    ssize_t bytes_read;

    to_read = 16;
    printed_same = 0;
    if (*count % 16 != 0)
        to_read = 16 - *count % 16;
    while (1)
    {
        bytes_read = read(fd, buf + (16 - to_read), to_read);
        *count += (int)bytes_read;
        if (bytes_read <= 0 || *count % 16 != 0)
            break;
        if (ft_strcmp(old_buf, buf) == 0)
        {
            if (printed_same == 0)
            {
                write(1, "*\n", 2);
                printed_same = 1;
            }
        }
        else
        {
            printed_same = 0;
            ft_print_line(*count - 16, buf, 16, no_flag);
        }
        ft_strcpy(old_buf, buf);
        to_read = 16;
    }
}

void ft_read_files(int nfiles, char **file_paths, char *prog, int no_flag)
{
    int fd;
    int idx;
    int count;
    int one_worked;
    char buf[16];

    idx = 0;
    count = 0;
    one_worked = 0;
    while (idx < nfiles)
    {
        fd = open(file_paths[idx], O_RDONLY);
        if (fd == -1)
        {
            ft_print_error(file_paths[idx++], prog, 0);
            continue;
        }
        one_worked = 1;
        ft_read_to_buf(fd, buf, &count, no_flag);
        close(fd);
        idx++;
    }
    if (one_worked == 0 || count <= 0)
    {
        if (one_worked == 0)
            ft_print_error("", prog, 1);
        return;
    }
    if (count % 16 != 0)
        ft_print_line(count - (count % 16), buf, count % 16, no_flag);
    ft_print_offset(count, buf, no_flag);
    idx = 7;
    if (!no_flag)
        idx++;
    buf[idx] = '\n';
    write(1, buf, idx + 1);
}

int ft_check_arg(char *arg, char *prog)
{
    int len;
    int prog_len;

    len = ft_strlen(arg);
    prog_len = ft_strlen(prog);
    if (arg[0] != '-' || len == 1)
        return (1);
    if (arg[1] != 'C' || len > 2)
    {
        write(2, prog, prog_len);
        write(2, ": invalid option -- '", 21);
        if (arg[1] != 'C')
            write(2, &arg[1], 1);
        else
            write(2, &arg[2], 1);
        write(2, "'\n", 2);
        write(2, "Try 'hexdump --help' for more information.\n", 43);
        return (-1);
    }
    return (0);
}

int main(int argc, char **argv)
{
    char *prog;
    int arg_worked;
    int start_at;

    prog = basename(argv[0]);
    start_at = 1;
    if (argc > 1)
    {
        arg_worked = ft_check_arg(argv[1], prog);
        if (arg_worked == -1)
            return (1);
        if (arg_worked == 0)
            start_at++;
        ft_read_files(argc - start_at, argv + start_at, prog, arg_worked);
    }
    return (0);
}
