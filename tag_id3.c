/*
 * Tag (ID3) Read Part
 * Read (ID3) Tag from music file.
 */

#include "tag_id3.h"

/* 
 * Get ID3v1 Tag.
 */

gchar **tag_get_id3v1(FILE *file)
{
    gchar *b;
    gchar *temp;
    static gchar a[130], *tag[3];
    gsize r,w;
    gint i,n;
    tag[0] = NULL;
    tag[1] = NULL;
    tag[2] = NULL;
    if(fseek(file, -128, 2)==-1)
    {
        return NULL;
    }
    if(!fread(a, 128, 1, file))
    {
        return NULL;
    }
    b=a;
    if(*b=='T' && *(b+1)=='A' && *(b+2)=='G')
    {
        b+=3;
        for(n=0;n<3;n++)
	{
            if(n==0) tag[2] = g_strndup(b, 30);
            else if(n==1) tag[0] = g_strndup(b, 30);
            else if(n==2) tag[1] = g_strndup(b, 30);
            b+=30;
	    for(i=1;!(*(b-i)>=33 && *(b-i)<=122);i++)
                if(*(b-i)==0) break;
		else if(*(b-i)==32) *(b-i)=0;
	}
        for(i=0;i<3;i++)
        {
            if(!g_utf8_validate(tag[i],-1,NULL))
            {
                temp = g_convert(tag[i], -1, "UTF-8",
                    extra_encoding, &r, &w, NULL);
                if(tag[i]!=NULL) g_free(tag[i]);
                tag[i] = temp;
            }
            else
            {
                g_free(tag[i]);
                tag[i] = NULL;
            }
        }
        return tag;
    }
    return NULL;
}

/* 
 * Get ID3v2 Tag.
 */

static char **tag_get_id3v2(FILE *file)
{
    gchar *a, *c, b[5], *d;
    static gchar *tag[3];
    gint j, header_size, version;
    glong i;
    gint size[4], tag_type;
    gsize r, w;
    tag[0] = NULL;
    tag[1] = NULL;
    tag[2] = NULL;
    a = g_malloc(10);
    fread(a, 10, 1, file);
    /* Judge if it is an ID3v2 tag. */
    if(*a=='I' && *(a+1)=='D' && *(a+2)=='3')
    {
        version = *(a+3);
        size[0] = *(a+6);
        if(size[0]<0) size[0]+=256;
        size[1] = *(a+7);
        if(size[1]<0) size[1]+=256;
        size[2] = *(a+8);
        if(size[2]<0) size[2]+=256;
        size[3] = *(a+9);
        if(size[3]<0) size[3]+=256;
        i = (size[0]&0x7F)*0x200000 + (size[1]&0x7F)*0x4000 + 
            (size[2]&0x7F)*0x80 + (size[3]&0x7F);
        header_size = i;
        g_free(a);
        a = g_malloc(i);
        if(!fread(a,i,1,file))
        {
            return NULL;
        };
        c = a;
        /* If it is an ID3v2.3 tag. */
        if(version==3)
        {
            /* Read each tag in the loop. */
            for(;c!=a+header_size;)
            {
                if(tag[0]!=NULL && tag[1]!=NULL && tag[2]!=NULL) return tag;
                strcpy(b,c);
                b[4] = 0;
                tag_type = 0;
                size[0] = *(c+4);
                if(size[0]<0) size[0]+=256;
                size[1] = *(c+5);
                if(size[1]<0) size[1]+=256;
                size[2]=*(c+6);
                if(size[2]<0) size[2]+=256;
                size[3]=*(c+7);
                if(size[3]<0) size[3]+=256;
                i = size[0]*0x1000000+size[1]*0x10000+size[2]*0x100+size[3];
                if(i<0) i+=256;
                if((c+i)>(a+header_size)) break;
                if(i==0) break;
                tag_type = 0;
                if(!strcmp(b, "TPE1")) tag_type = 1;
                if(!strcmp(b, "TALB")) tag_type = 2;
                if(!strcmp(b, "TIT2")) tag_type = 3;
                if(!tag_type)
                {
                    c+=i+10;
                    continue;
                }
                c+=10;
                d = g_malloc(i);
                if(d==NULL) return NULL;
                for(j=0;i!=0;i--)
                {
                    if(*c!=0)
                    {
                        d[j] = *c;
                        j++;
                    }
                    c++;
                }
                d[j]=0;
                if(g_utf8_validate(d,-1,NULL))
                    tag[tag_type-1] = NULL;
                else
                {
                    tag[tag_type-1]=g_convert(d, -1, "UTF-8", extra_encoding,
                        &r, &w, NULL);
                }
                g_free(d);
            }
            if(tag[0]==NULL && tag[1]==NULL && tag[2]==NULL) return NULL;
            return tag;
        }
        /* If it is an ID3v2.2 tag. */
        else if(version==2)
        {
            /* Read each tag in the loop. */
            for(;c!=a+header_size;)
            {
                if(tag[0]!=NULL && tag[1]!=NULL && tag[2]!=NULL) return tag;
                strcpy(b,c);
                b[3] = 0;
                size[0] = *(c+3);
                size[1] = *(c+4);
                size[2] = *(c+5);
                i = size[0]*0x10000 + size[1]*0x100 + size[2];
                if(i<0) i+=256;
                if((c+i)>(a+header_size)) break;
                if(i==0) break;
                tag_type=0;
                if(!strcmp(b,"TP1")) tag_type = 1;
                if(!strcmp(b,"TAL")) tag_type = 2;
                if(!strcmp(b,"TT2")) tag_type = 3;
                if(!tag_type)
                {
                    c+=i+6;
                    continue;
                }
                c+=6;
                d = g_malloc(i);
                if(d==NULL) return NULL;
                for(j=0;i!=0;i--)
                {
                    if(*c==0)
                    {
                        c++;
                        continue;
                    }
                    d[j] = *c;
                    j++;
                    c++;
                }
                d[j] = 0;
                if(g_utf8_validate(d,-1,NULL))
                    tag[tag_type-1] = NULL;
                else
                {
                    tag[tag_type-1] = g_convert(d, -1, "UTF-8", 
                        extra_encoding, &r, &w, NULL);
                }
                g_free(d);
            }
            if(tag[0]==NULL && tag[1]==NULL && tag[2]==NULL) return NULL;
            return tag;
        }
    }
    return NULL;
}

/*
 * Read tags from music file.
 */

gchar **tag_get_id3(gchar *filename) 
{
    FILE *file;
    static gchar **tag;
    RCSetting *rc_setting = get_setting();
    extra_encoding = rc_setting->tag_ex_encoding;
    if(filename==NULL || *filename==0)
    {
        g_printf("Get tag: Invaild filename.\n");
        return NULL;
    }
    file=fopen(filename, "rb");
    if(file==NULL)
    {
        g_printf("Get tag: Can't open file %s\n", filename);
        return NULL;
    }
    /* Get ID3v2 tag, if it returns NULL, then try to get ID3v1 tag. */
    tag=tag_get_id3v2(file);
    if(tag==NULL)
    {
        rewind(file);
        tag=tag_get_id3v1(file);
    }
    fclose(file);
    return tag;
}


