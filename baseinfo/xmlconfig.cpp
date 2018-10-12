/*************************************************************************
	> File Name: xml.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月19日 星期五 11时33分16秒
 ************************************************************************/

#include<stdio.h>
#include <mxml.h>
#include <stdlib.h>
#include <string.h>
#include "xmlconfig.h"
#include <logger.h>

static mxml_type_t type_cb(mxml_node_t *node)
{
    const char *type;
    type = mxmlElementGetAttr(node, "type");
    if (type == NULL)
        type = node->value.element.name;
    if (!strcmp(type, "integer"))
        return (MXML_INTEGER);
    else if (!strcmp(type, "opaque"))
        return (MXML_OPAQUE);
    else if (!strcmp(type, "real"))
        return (MXML_REAL);
    else
        return (MXML_TEXT);
}

static const char * whitespace_cb(mxml_node_t *node,int where)
{
    const char *name;        /* 我们可以在任何XML元素之前或之后有条件的添加换行。这些是一些常见的HTML元素 */
    name = node->value.element.name;
    if (!strcmp(name, "html") ||
            !strcmp(name, "head") ||
            !strcmp(name, "body") ||
            !strcmp(name, "pre") ||
            !strcmp(name, "p") ||
            !strcmp(name, "h1") ||
            !strcmp(name, "h2") ||
            !strcmp(name, "h3") ||
            !strcmp(name, "h4") ||
            !strcmp(name, "h5") ||
            !strcmp(name, "h6"))
    {        /* 在打开之前和关闭之后时换行。*/
      if (where == MXML_WS_BEFORE_OPEN ||
              where == MXML_WS_AFTER_CLOSE)
          return ("\n");
    }
    else if (!strcmp(name, "dl") ||
             !strcmp(name, "ol") ||
             !strcmp(name, "ul"))
    {
        /* 在列表元素前后都添加换行。*/
        return ("\n");
    }
    else if (!strcmp(name, "dd") ||
             !strcmp(name, "dt") ||
             !strcmp(name, "li"))
    {        /*  添加一个"跳格"在<li>, * <dd>,和 <dt>之前, 以及一个换行在他们后面 */
        if (where == MXML_WS_BEFORE_OPEN)
            return ("\t");
        else if (where == MXML_WS_AFTER_CLOSE)
            return ("\n");
    }
    /*        * 如果不需要添加空白字符则返回NULL。       */
    return (NULL);
}

mxml_node_t *load_xmlfile(const char *file)
{
    mxml_node_t *tree;
    FILE *fp;

    fp = fopen(file,"r");
    if (NULL == fp){
        LOG_WARN("%s open failed",file);
        return NULL;
    }
    tree = mxmlLoadFile(NULL,fp,MXML_TEXT_CALLBACK);
    fclose(fp);
    return tree;
}

void unload_xmlfile(mxml_node_t *root)
{
    if (!root)
        return;
    mxmlDelete(root);
}

static mxml_node_t *get_mxml_node(mxml_node_t *top,const char *name)
{
    mxml_node_t *node = mxmlFindElement(top,top,name,
                           NULL,NULL,MXML_DESCEND);
    if (!node){
        LOG_DEBUG("node %p get failed",node);
        return NULL;
    }
    return node;
}

static BOOL get_mxml_attr(mxml_node_t *top,mxml_node_t *node,const char *attr,S8 buffer[],int max)
{
    mxml_node_t *xml_node = node;
    if (!xml_node)
        return FALSE;
    const char *get = mxmlElementGetAttr(xml_node,attr);
    if (!get){
        LOG_DEBUG("node %p attribute %s get failed",node,attr);
        return FALSE;
    }
    int len;
    if ((len=strlen(get))>=max){
        LOG_DEBUG("node %p attribute %s buffer len(%d)>max(%d)",node,attr,len,max);
        return FALSE;
    }
    strcpy(buffer,get);
    return TRUE;
}

static BOOL get_mxml_attr(mxml_node_t *top,const char *node,const char *attr,S8 buffer[],int max)
{
    mxml_node_t *xml_node = get_mxml_node(top,node);
    if (!xml_node)
        return FALSE;
    return get_mxml_attr(top,xml_node,attr,buffer,max);
}

static BOOL get_mxml_value(mxml_node_t *top,const char *node,S8 buffer[],int max)
{
    mxml_node_t *xml_node = get_mxml_node(top,node);
    if (!xml_node)
        return FALSE;
    const char *get = xml_node->child->value.text.string;
    if (!get){
        LOG_DEBUG("node %p value get failed",xml_node);
        return FALSE;
    }
    int len;
    if ((len=strlen(get))>=max){
        LOG_DEBUG("node %p  buffer len(%d)>max(%d)",node,len,max);
        return FALSE;
    }
    strcpy(buffer,get);
    return TRUE;
}


BOOL read_config_baseinfo(gateway_config_t *config)
{
//    mxml_node_t *tree,*node,*child;
//    char config_file[256];

//    DAS_ASSERT(config);
//    /* set as 0*/
//    bzero(config,sizeof(gateway_config_t));

//    sprintf(config_file,"%s/config/config.xml",
//            getenv(CONFIG_DIR_ENV));
//    tree = load_xmlfile(config_file);
//    if (!tree){
//        return FALSE;
//    }

//    node = get_mxml_node(tree,"gateway");

//    if (!get_mxml_attr(tree,node,"id",config->id,sizeof(config->id)-1))
//        return FALSE;

//    LOG_INFO("gateway id = %s",config->id);
//    child = mxmlFindElement(node,tree,"heartbeatPeriod",
//                           NULL,NULL,MXML_DESCEND);
//    LOG_INFO("type=%d,%s,name=%s",child->child->type,child->child->value.text.string,child->value.element.name);
//    config->heartbeat = atoi(child->child->value.text.string);
//    LOG_INFO("gateway heartbeat = %d",config->heartbeat);

//    /* datacenter */
//    node = get_mxml_node(tree,"dataCenter",
//                           MXML_DESCEND);
//    if (!get_mxml_attr(tree,node,"id",config->datacenter.id,sizeof(config->datacenter.id)-1))
//        return FALSE;
//    LOG_INFO("dataCenter id = %s",config->datacenter.id);
//    if (!get_mxml_attr(tree,node,"id",config->datacenter.type,sizeof(config->datacenter.type)-1))
//        return FALSE;
//    LOG_INFO("dataCenter type = %s",config->datacenter.type);

//    child = get_mxml_node(node,tree,"ip",
//                           NULL,NULL,MXML_DESCEND);
//    strcpy(config->datacenter.netaddr,child->value.text.string);
//    child = mxmlFindElement(node,tree,"port",
//                           NULL,NULL,MXML_DESCEND);
//    config->datacenter.port=child->value.integer;
    return true;
}


