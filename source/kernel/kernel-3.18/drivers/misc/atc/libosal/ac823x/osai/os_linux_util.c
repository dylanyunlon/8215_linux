/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/



#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include "x_os.h"

#ifdef KERNEL_STANDARD_API
#include <linux/types.h>


#define restrict
#define strtoull simple_strtoull
extern void *memrchr(const void *s, s32 c, size_t n);
extern long long strtoll(const char *restrict str, char **restrict endptr, s32 base);


#if 0
void *x_memcpy(void *pv_to, const void *pv_from, u32 z_l)
{
    if ((pv_to == (char *) NULL))
    {
        return pv_to;
    }
    return memcpy(pv_to, pv_from, z_l);
}

EXPORT_SYMBOL(x_memcpy);


s32 x_memcmp(const void *pv_s1, const void *pv_s2, u32 z_l)
{
    return memcmp(pv_s1, pv_s2, z_l);
}

EXPORT_SYMBOL(x_memcmp);
#endif 

void *x_memmove(void *pv_to, const void *pv_from, u32 z_l)
{
    return memmove(pv_to, pv_from, z_l);
}

EXPORT_SYMBOL(x_memmove);


#if 0
void *x_memset(void *pv_to, u8 ui1_c, u32 z_l)
{
    return memset(pv_to, ui1_c, z_l);
}

EXPORT_SYMBOL(x_memset);

void *x_memchr(const void *pv_mem, u8 ui1_c, u32 z_len)
{
    return memchr(pv_mem, ui1_c, z_len);
}

EXPORT_SYMBOL(x_memchr);
#endif 


void *x_memrchr(const void *pv_mem, u8 ui1_c, u32 z_len)
{
    return memrchr(pv_mem, (int)ui1_c, (size_t)z_len);
}

EXPORT_SYMBOL(x_memrchr);


char *x_strdup(const char *ps_str)
{
    char *ps_dup_str = NULL;

    if (ps_str != NULL)
    {
        size_t z_len;

        z_len = strlen(ps_str) + (size_t)1;
        ps_dup_str = (char *)x_mem_alloc(z_len);
        if (ps_dup_str != NULL)
        {
            memcpy(ps_dup_str, ps_str, z_len);
        }
    }

    return ps_dup_str;
}

EXPORT_SYMBOL(x_strdup);


char *x_strcpy(char *ps_to, const char *ps_from)
{
    return (char *)strcpy((char *)ps_to, (const char *)ps_from);
}

EXPORT_SYMBOL(x_strcpy);


#if 0
char *x_strncpy(char *ps_to, const char *ps_from, u32 z_len)
{
    return (char *)strncpy((char *)ps_to, (const char *)ps_from, (size_t)z_len);
}

EXPORT_SYMBOL(x_strncpy);


s32 x_strcmp(const char *ps_str1, const char *ps_str2)
{
    return (s32)strcmp((const char *)ps_str1, (const char *)ps_str2);
}

EXPORT_SYMBOL(x_strcmp);


s32 x_strncmp(const char *ps_s1, const char *ps_s2, u32 z_l)
{
    return (s32)strncmp((const char *)ps_s1, (const char *)ps_s2, (size_t)z_l);
}

EXPORT_SYMBOL(x_strncmp);
#endif

s32 x_strcasecmp(const char *ps_str1, const char *ps_str2)
{
    return (s32)strcasecmp((const char *)ps_str1, (const char *)ps_str2);
}

EXPORT_SYMBOL(x_strcasecmp);


s32 x_strncasecmp(const char *ps_str1, const char *ps_str2, u32 z_len)
{
    return (s32)strncasecmp((const char *)ps_str1, (const char *)ps_str2, (size_t)z_len);
}

EXPORT_SYMBOL(x_strncasecmp);


char *x_strcat(char *ps_to, const char *ps_append)
{
    return (char *)strcat((char *)ps_to, (const char *)ps_append);
}

EXPORT_SYMBOL(x_strcat);


#if 0
char *x_strncat(char *ps_to, const char *ps_append, u32 z_len)
{
    return (char *)strncat((char *)ps_to, (const char *)ps_append, (size_t)z_len);
}

EXPORT_SYMBOL(x_strncat);


char *x_strchr(const char *ps_str, char c_char)
{
    return (char *)strchr((const char *)ps_str, (int)c_char);
}

EXPORT_SYMBOL(x_strchr);


char *x_strrchr(const char *ps_str, char c_char)
{
    return (char *)strrchr((const char *)ps_str, (int)c_char);
}

EXPORT_SYMBOL(x_strrchr);
#endif


char *x_strstr(const char *ps_str, const char *ps_find)
{
    return (char *)strstr((const char *)ps_str, (const char *)ps_find);
}

EXPORT_SYMBOL(x_strstr);


u64 x_strtoull(const char *pc_beg_ptr, char **ppc_end_ptr, u8 ui1_base)
{
    return (u64)strtoull((const char *)pc_beg_ptr, (char **)ppc_end_ptr, (int)ui1_base);
}

EXPORT_SYMBOL(x_strtoull);


s64 x_strtoll(const char *pc_beg_ptr, char **ppc_end_ptr, u8 ui1_base)
{
    return (s64)strtoll((const char *)pc_beg_ptr, (char **)ppc_end_ptr, (s32)ui1_base);
}

EXPORT_SYMBOL(x_strtoll);


#if 0
u32 x_strlen(const char *ps_str)
{
    if (ps_str == NULL)
    {
        return 0;
    }
    return (u32)strlen((const char *)ps_str);
}

EXPORT_SYMBOL(x_strlen);


u32 x_strspn(const char *ps_str, const char *ps_accept)
{
    return (u32)strspn((const char *)ps_str, (const char *)ps_accept);
}

EXPORT_SYMBOL(x_strspn);


u32 x_strcspn (const char *ps_str, const char *ps_reject)
{
    return (u32)strcspn((const char *)ps_str, (const char *)ps_reject);
}

EXPORT_SYMBOL(x_strcspn);
#endif 



char *x_str_toupper(char *ps_str)
{
    if (ps_str != NULL)
    {
        char *ps_cursor;
        char c_char;

        ps_cursor = ps_str;

        while ((c_char = *ps_cursor) != '\0')
        {
            *ps_cursor++ = (char)toupper((unsigned char)c_char);
        }
    }

    return ps_str;
}

EXPORT_SYMBOL(x_str_toupper);


char *x_str_tolower(char *ps_str)
{
    if (ps_str != NULL)
    {
        char *ps_cursor;
        char c_char;

        ps_cursor = ps_str;

        while ((c_char = *ps_cursor) != '\0')
        {
            *ps_cursor++ = (char)tolower((unsigned char)c_char);
        }
    }

    return ps_str;
}

EXPORT_SYMBOL(x_str_tolower);


char *x_strtok(char *ps_str, const char *ps_delimiters, char **pps_str, u32 *pz_token_len)
{
    char *ps_token;

    ps_token = NULL;

    if ((ps_str != NULL) && (ps_delimiters != NULL) && (pps_str != NULL))
    {
        ps_token = ps_str + x_strspn(ps_str, ps_delimiters);

        if (*ps_token != '\0')
        {
            u32 z_token_len;

            z_token_len = x_strcspn(ps_token, ps_delimiters);

            *pps_str = ps_token + z_token_len;

            if (pz_token_len != NULL)
            {
                /* original string is not modified */
                *pz_token_len = z_token_len;
            }
            else if (ps_token[z_token_len] != '\0')
            {
                /* a NULL character overwrites part of the original string */
                ps_token[z_token_len] = '\0';
                (*pps_str)++;
            }
			else
			{

			}
        }
        else
        {
            *pps_str = NULL;
            ps_token = NULL;
        }
    }

    return ps_token;
}

EXPORT_SYMBOL(x_strtok);


s32
x_sprintf(char *ps_str, const char *ps_format, ...)
{
    s32 i4_len;
    va_list t_ap;

    va_start(t_ap, ps_format);
    i4_len = vsprintf((char *)ps_str, (const char *)ps_format, t_ap);
    va_end(t_ap);

    return (s32)i4_len;
}

EXPORT_SYMBOL(x_sprintf);


s32
x_vsprintf(char *ps_str, const char *ps_format, VA_LIST va_list)
{
    return (s32)vsprintf((char *)ps_str, (const char *)ps_format, va_list);
}

EXPORT_SYMBOL(x_vsprintf);


s32
x_snprintf(char *ps_str, u32 z_size, const char *ps_format, ...)
{
    s32 i4_len;
    va_list t_ap;

    va_start(t_ap, ps_format);
    i4_len = vsnprintf((char *)ps_str, (size_t)z_size, (const char *)ps_format, t_ap);
    va_end(t_ap);

    return (s32)i4_len;
}

EXPORT_SYMBOL(x_snprintf);


s32
x_vsnprintf(char *ps_str, u32 z_size, const char *ps_format, VA_LIST va_list)
{
    return (s32)vsnprintf((char *)ps_str, (size_t)z_size, (const char *)ps_format, va_list);
}

EXPORT_SYMBOL(x_vsnprintf);


s32
x_sscanf(const char *ps_buf, const char *ps_fmt, ...)
{
    s32 i4_len;
    va_list t_ap;

    va_start(t_ap, ps_fmt);
    i4_len = vsscanf((const char *)ps_buf, (const char *)ps_fmt, t_ap);
    va_end(t_ap);

    return (s32)i4_len;
}

EXPORT_SYMBOL(x_sscanf);


s32
x_vsscanf(const char *ps_buf, const char *ps_fmt, VA_LIST t_ap)
{
    return (s32)vsscanf((const char *)ps_buf, (const char *)ps_fmt, t_ap);
}

EXPORT_SYMBOL(x_vsscanf);

#else //old api

#define restrict
#define strtoull simple_strtoull
extern void *memrchr(const void *s, int c, size_t n);
extern long long strtoll(const char *restrict str, char **restrict endptr, int base);


#if 0
void *x_memcpy(void *pv_to, const void *pv_from, size_t z_l)
{
    if ((pv_to == (char *) NULL))
    {
        return pv_to;
    }
    return memcpy(pv_to, pv_from, z_l);
}

EXPORT_SYMBOL(x_memcpy);


s32 x_memcmp(const void *pv_s1, const void *pv_s2, size_t z_l)
{
    return memcmp(pv_s1, pv_s2, z_l);
}

EXPORT_SYMBOL(x_memcmp);
#endif 

void *x_memmove(void *pv_to, const void *pv_from, size_t z_l)
{
    return memmove(pv_to, pv_from, z_l);
}

EXPORT_SYMBOL(x_memmove);


#if 0
void *x_memset(void *pv_to, u8 ui1_c, size_t z_l)
{
    return memset(pv_to, ui1_c, z_l);
}

EXPORT_SYMBOL(x_memset);

void *x_memchr(const void *pv_mem, u8 ui1_c, size_t z_len)
{
    return memchr(pv_mem, ui1_c, z_len);
}

EXPORT_SYMBOL(x_memchr);
#endif 


void *x_memrchr(const void *pv_mem, u8 ui1_c, size_t z_len)
{
    return memrchr(pv_mem, ui1_c, z_len);
}

EXPORT_SYMBOL(x_memrchr);


char *x_strdup(const char *ps_str)
{
    char *ps_dup_str = NULL;

    if (ps_str != NULL)
    {
        size_t z_len;

        z_len = strlen(ps_str) + 1;
        ps_dup_str = (char *)x_mem_alloc(z_len);
        if (ps_dup_str != NULL)
        {
            memcpy(ps_dup_str, ps_str, z_len);
        }
    }

    return ps_dup_str;
}

EXPORT_SYMBOL(x_strdup);


char *x_strcpy(char *ps_to, const char *ps_from)
{
    return (char *)strcpy((char *)ps_to, (const char *)ps_from);
}

EXPORT_SYMBOL(x_strcpy);


#if 0
char *x_strncpy(char *ps_to, const char *ps_from, size_t z_len)
{
    return (char *)strncpy((char *)ps_to, (const char *)ps_from, (size_t)z_len);
}

EXPORT_SYMBOL(x_strncpy);


s32 x_strcmp(const char *ps_str1, const char *ps_str2)
{
    return (s32)strcmp((const char *)ps_str1, (const char *)ps_str2);
}

EXPORT_SYMBOL(x_strcmp);


s32 x_strncmp(const char *ps_s1, const char *ps_s2, size_t z_l)
{
    return (s32)strncmp((const char *)ps_s1, (const char *)ps_s2, (size_t)z_l);
}

EXPORT_SYMBOL(x_strncmp);
#endif

__s32 x_strcasecmp(const char *ps_str1, const char *ps_str2)
{
    return (__s32)strcasecmp((const char *)ps_str1, (const char *)ps_str2);
}

EXPORT_SYMBOL(x_strcasecmp);


__s32 x_strncasecmp(const char *ps_str1, const char *ps_str2, size_t z_len)
{
    return (s32)strncasecmp((const char *)ps_str1, (const char *)ps_str2, (size_t)z_len);
}

EXPORT_SYMBOL(x_strncasecmp);


char *x_strcat(char *ps_to, const char *ps_append)
{
    return (char *)strcat((char *)ps_to, (const char *)ps_append);
}

EXPORT_SYMBOL(x_strcat);


#if 0
char *x_strncat(char *ps_to, const char *ps_append, size_t z_len)
{
    return (char *)strncat((char *)ps_to, (const char *)ps_append, (size_t)z_len);
}

EXPORT_SYMBOL(x_strncat);


char *x_strchr(const char *ps_str, char c_char)
{
    return (char *)strchr((const char *)ps_str, (int)c_char);
}

EXPORT_SYMBOL(x_strchr);


char *x_strrchr(const char *ps_str, char c_char)
{
    return (char *)strrchr((const char *)ps_str, (int)c_char);
}

EXPORT_SYMBOL(x_strrchr);
#endif


char *x_strstr(const char *ps_str, const char *ps_find)
{
    return (char *)strstr((const char *)ps_str, (const char *)ps_find);
}

EXPORT_SYMBOL(x_strstr);


__u64 x_strtoull(const char *pc_beg_ptr, char **ppc_end_ptr, __u8 ui1_base)
{
    return (__u64)strtoull((const char *)pc_beg_ptr, (char **)ppc_end_ptr, (int)ui1_base);
}

EXPORT_SYMBOL(x_strtoull);


__s64 x_strtoll(const char *pc_beg_ptr, char **ppc_end_ptr, __u8 ui1_base)
{
    return (__s64)strtoll((const char *)pc_beg_ptr, (char **)ppc_end_ptr, (int)ui1_base);
}

EXPORT_SYMBOL(x_strtoll);


#if 0
size_t x_strlen(const char *ps_str)
{
    if (ps_str == NULL)
    {
        return 0;
    }
    return (size_t)strlen((const char *)ps_str);
}

EXPORT_SYMBOL(x_strlen);


size_t x_strspn(const char *ps_str, const char *ps_accept)
{
    return (size_t)strspn((const char *)ps_str, (const char *)ps_accept);
}

EXPORT_SYMBOL(x_strspn);


size_t x_strcspn (const char *ps_str, const char *ps_reject)
{
    return (size_t)strcspn((const char *)ps_str, (const char *)ps_reject);
}

EXPORT_SYMBOL(x_strcspn);
#endif 



char *x_str_toupper(char *ps_str)
{
    if (ps_str != NULL)
    {
        char *ps_cursor;
        char c_char;

        ps_cursor = ps_str;

        while ((c_char = *ps_cursor) != '\0')
        {
            *ps_cursor++ = toupper(c_char);
        }
    }

    return ps_str;
}

EXPORT_SYMBOL(x_str_toupper);


char *x_str_tolower(char *ps_str)
{
    if (ps_str != NULL)
    {
        char *ps_cursor;
        char c_char;

        ps_cursor = ps_str;

        while ((c_char = *ps_cursor) != '\0')
        {
            *ps_cursor++ = tolower(c_char);
        }
    }

    return ps_str;
}

EXPORT_SYMBOL(x_str_tolower);


char *x_strtok(char *ps_str, const char *ps_delimiters, char **pps_str, size_t *pz_token_len)
{
    char *ps_token;

    ps_token = NULL;

    if ((ps_str != NULL) && (ps_delimiters != NULL) && (pps_str != NULL))
    {
        ps_token = ps_str + x_strspn(ps_str, ps_delimiters);

        if (*ps_token != '\0')
        {
            size_t z_token_len;

            z_token_len = x_strcspn(ps_token, ps_delimiters);

            *pps_str = ps_token + z_token_len;

            if (pz_token_len != NULL)
            {
                /* original string is not modified */
                *pz_token_len = z_token_len;
            }
            else if (ps_token[z_token_len] != '\0')
            {
                /* a NULL character overwrites part of the original string */
                ps_token[z_token_len] = '\0';
                (*pps_str)++;
            }
        }
        else
        {
            *pps_str = NULL;
            ps_token = NULL;
        }
    }

    return ps_token;
}

EXPORT_SYMBOL(x_strtok);


__s32
x_sprintf(char *ps_str, const char *ps_format, ...)
{
    int i4_len;
    va_list t_ap;

    va_start(t_ap, ps_format);
    i4_len = vsprintf((char *)ps_str, (const char *)ps_format, t_ap);
    va_end(t_ap);

    return (__s32)i4_len;
}

EXPORT_SYMBOL(x_sprintf);


__s32
x_vsprintf(char *ps_str, const char *ps_format, VA_LIST va_list)
{
    return (__s32)vsprintf((char *)ps_str, (const char *)ps_format, va_list);
}

EXPORT_SYMBOL(x_vsprintf);


__s32
x_snprintf(char *ps_str, size_t z_size, const char *ps_format, ...)
{
    int i4_len;
    va_list t_ap;

    va_start(t_ap, ps_format);
    i4_len = vsnprintf((char *)ps_str, (size_t)z_size, (const char *)ps_format, t_ap);
    va_end(t_ap);

    return (__s32)i4_len;
}

EXPORT_SYMBOL(x_snprintf);


__s32
x_vsnprintf(char *ps_str, size_t z_size, const char *ps_format, VA_LIST va_list)
{
    return (__s32)vsnprintf((char *)ps_str, (size_t)z_size, (const char *)ps_format, va_list);
}

EXPORT_SYMBOL(x_vsnprintf);


__s32
x_sscanf(const char *ps_buf, const char *ps_fmt, ...)
{
    int i4_len;
    va_list t_ap;

    va_start(t_ap, ps_fmt);
    i4_len = vsscanf((const char *)ps_buf, (const char *)ps_fmt, t_ap);
    va_end(t_ap);

    return (__s32)i4_len;
}

EXPORT_SYMBOL(x_sscanf);


__s32
x_vsscanf(const char *ps_buf, const char *ps_fmt, VA_LIST t_ap)
{
    return (__s32)vsscanf((const char *)ps_buf, (const char *)ps_fmt, t_ap);
}

EXPORT_SYMBOL(x_vsscanf);

const char *basename(const char *full_name)
{
	size_t z_len;
	
	z_len = strlen(full_name);
	while (z_len --)
	{
		if ('/' == *(full_name + z_len))
		{
			return full_name + z_len + 1;
		}
	}
	return full_name;
}

EXPORT_SYMBOL(basename);



#endif



