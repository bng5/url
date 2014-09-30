/* -*- encoding: utf-8; -*- */
/* -*- c-mode -*- */
/* File-name:    <url.c> */
/* Author:       <Xsoda> */
/* Create:       <Friday December 20 12:38:02 2013> */
/* Time-stamp:   <Wednesday December 25, 10:10:57 2013> */
/* Mail:         <Xsoda@Live.com> */

#include "url.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *str_hosttype[] = { "host ipv4", "host ipv6", "host domain", NULL };

static char *strndup2(const char *str, int n) {
    char *dst;
    if (!str) return NULL;
    if (n < 0) n = strlen(str);
    if (n == 0) return NULL;
    if ((dst = (char *)malloc(n + 1)) == NULL)
        return NULL;
    memcpy(dst, str, n);
    dst[n] = 0;
    return dst;
}

static int host_is_ipv4(char *str)
{
   if (!str) return 0;
   while (*str)
   {
      if ((*str >= '0' && *str <= '9') || *str == '.')
         str++;
      else
         return 0;
   }
   return 1;
}

void parse_query(url_field_t *url, char *query)
{
   int length;
   int offset;
   char *chr;
   length = strlen(query);
   offset = 0;
   chr = strchr(query, '=');
   while (chr)
   {
      if (url->query)
         url->query = realloc(url->query, (url->query_num + 1) * sizeof(*url->query));
      else
         url->query = malloc(sizeof(*url->query));
      url->query[url->query_num].name = strndup2(query, chr - query);
      query = chr + 1;
      chr = strchr(query, '&');
      if (chr)
      {
         url->query[url->query_num].value = strndup2(query, chr - query);
         url->query_num++;
         query = chr + 1;
         chr = strchr(query, '=');
      }
      else
      {
         url->query[url->query_num].value = strndup2(query, -1);
         url->query_num++;
         break;
      }
   }
}
url_field_t *url_parse (const char *str)
{
   const char *pch;
   char *query;
   url_field_t *url;
   query = NULL;
   if ((url = (url_field_t *)malloc(sizeof(url_field_t))) == NULL)
      return NULL;
   memset(url, 0, sizeof(url_field_t));
   if (str && str[0])
   {
      url->href = strndup2(str, -1);
      pch = strchr(str, ':');   /* parse schema */
      if (pch && pch[1] == '/' && pch[2] == '/')
      {
         url->schema = strndup2(str, pch - str);
         str = pch + 3;
      }
      else {
         url->schema = strndup2("http", 4);
         //goto __fail;
      }
      pch = strchr(str, '@');   /* parse user info */
      if (pch)
      {
         pch = strchr(str, ':');
         if (pch)
         {
            url->username = strndup2(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '@');
            if (pch)
            {
               url->password = strndup2(str, pch - str);
               str = pch + 1;
            }
            else
               goto __fail;
         }
         else
            goto __fail;
      }
      if (str[0] == '[')        /* parse host info */
      {
         str++;
         pch = strchr(str, ']');
         if (pch)
         {
            url->host = strndup2(str, pch - str);
            str = pch + 1;
            if (str[0] == ':')
            {
               str++;
               pch = strchr(str, '/');
               if (pch)
               {
                  url->port = strndup2(str, pch - str);
                  str = pch + 1;
               }
               else
               {
                  url->port = strndup2(str, -1);
                  str = str + strlen(str);
               }
            }
            else {
                if(strcmp(url->schema, "http") == 0) {
                    url->port = strndup2("80", 2);
                }
                else if(strcmp(url->schema, "https") == 0) {
                    url->port = strndup2("443", 3);
                }
            }
            url->host_type = HOST_IPV6;
         }
         else
            goto __fail;
      }
      else
      {
         const char *pch_slash;
         pch = strchr(str, ':');
         pch_slash = strchr(str, '/');
         if (pch && (!pch_slash || (pch_slash && pch<pch_slash)))
         {
            url->host = strndup2(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '/');
            if (pch)
            {
               url->port = strndup2(str, pch - str);
               str = pch + 1;
            }
            else
            {
               url->port = strndup2(str, -1);
               str = str + strlen(str);
            }
         }
         else
         {
            pch = strchr(str, '/');
            if (pch)
            {
               url->host = strndup2(str, pch - str);
               str = pch + 1;
            }
            else
            {
               url->host = strndup2(str, -1);
               str = str + strlen(str);
            }
         }
         url->host_type = host_is_ipv4(url->host) ? HOST_IPV4 : HOST_DOMAIN;
         if (!url->port) {
             if(strcmp(url->schema, "http") == 0) {
                 url->port = strndup2("80", 2);
             }
             else if(strcmp(url->schema, "https") == 0) {
                 url->port = strndup2("443", 3);
             }
         }
      }
      if (str[0])               /* parse path, query and fragment */
      {
         pch = strchr(str, '?');
         if (pch)
         {
            url->path = strndup2(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '#');
            if (pch)
            {
               query = strndup2(str, pch - str);
               str = pch + 1;
               url->fragment = strndup2(str, -1);
            }
            else
            {
               query = strndup2(str, -1);
               str = str + strlen(str);
            }
            parse_query(url, query);
            free(query);
         }
         else
         {
            pch = strchr(str, '#');
            if (pch)
            {
               url->path = strndup2(str, pch - str);
               str = pch + 1;
               url->fragment = strndup2(str, -1);
               str = str + strlen(str);
            }
            else
            {
               url->path = strndup2(str, -1);
               str = str + strlen(str);
            }
         }
      }
   }
   else
   {
__fail:
      url_free(url);
      return NULL;
   }
   return url;
}

void url_free(url_field_t *url)
{
   if (!url) return;
   if (url->href) free(url->href);
   if (url->schema) free(url->schema);
   if (url->username) free(url->username);
   if (url->password) free(url->password);
   if (url->host) free(url->host);
   if (url->port) free(url->port);
   if (url->path) free(url->path);
   if (url->query)
   {
      int i;
      for (i = 0; i < url->query_num; i++)
      {
         free(url->query[i].name);
         free(url->query[i].value);
      }
      free(url->query);
   }
   if (url->fragment) free(url->fragment);
   free(url);
}

void url_field_print(url_field_t *url)
{
   if (!url) return;
   fprintf(stdout, "\nurl field:\n");
   fprintf(stdout, "  - href:     '%s'\n", url->href);
   fprintf(stdout, "  - schema:   '%s'\n", url->schema);
   if (url->username)
      fprintf(stdout, "  - username: '%s'\n", url->username);
   if (url->password)
      fprintf(stdout, "  - password: '%s'\n", url->password);
   fprintf(stdout, "  - host:     '%s' (%s)\n", url->host, str_hosttype[url->host_type]);
   if (url->port)
      fprintf(stdout, "  - port:     '%s'\n", url->port);
   if (url->path)
   fprintf(stdout, "  - path:     '%s'\n", url->path);
   if (url->query_num > 0)
   {
      int i;
      fprintf(stdout, "  - query\n");
      for (i = 0; i < url->query_num; i++)
      {
         fprintf(stdout, "    * %s : %s\n", url->query[i].name, url->query[i].value);
      }
   }
   if (url->fragment)
      fprintf(stdout, "  - fragment: '%s'\n", url->fragment);
}
// # if 0
int main(int argc, char *argv[])
{
   url_field_t *url = url_parse(argv[1]);
   url_field_print(url);
   url_free(url);
}
// # endif

