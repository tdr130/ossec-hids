/*   $OSSEC, config.c, v0.3, 2005/11/09, Daniel B. Cid$   */

/* Copyright (C) 2004,2005 Daniel B. Cid <dcid@ossec.net>
 * All rights reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shared.h"

#include "os_xml/os_xml.h"
#include "os_regex/os_regex.h"
#include "os_net/os_net.h"

#include "remoted.h"


/* RemotedConfig v0.3, 2005/11/09
 * Read the config file (the remote access)
 * v0.2: New OS_XML
 * v0.3: Some improvements and cleanup
 */ 
int RemotedConfig(char *cfgfile, remoted *logr)
{
    OS_XML xml;
    
    XML_NODE node = NULL;
    
    int i = 0;
    int j = 0;
    int rentries = 0;
    int fentries = 0;
    

    /*** XML Definitions ***/
    
    /* Allowed and denied IPS */
    char *(xml_allowips[])={xml_global, "allowed-ips",NULL};
    char *(xml_denyips[])={xml_global, "denied-ips",NULL};

    /* Remote options */	
    char *xml_remote_port = "port";
    char *xml_remote_group = "group";
    char *xml_remote_connection = "connection";

    /* Initializing */
    logr->port = NULL;
    logr->group = NULL;
    logr->conn = NULL;
    logr->allowips = NULL;
    logr->denyips = NULL;

    
    if(OS_ReadXML(cfgfile,&xml) < 0)
    {
        merror("%s: XML Error: %s",ARGV0, xml.err);
        return(OS_INVALID);
    }

    /* Return 0, if not configured to run xml_remote */
    if(!(rentries = OS_RootElementExist(&xml, xml_remote)))
    {
        merror("%s: No configuration entry. Exiting... ",ARGV0);
        OS_ClearXML(&xml);
        return(0);
    }	

    fentries = rentries; /* Saving the number of entries */
    
    /* Getting allowed ips */
    logr->allowips = OS_GetElementContent(&xml, xml_allowips);


    /* Getting denied ips */
    logr->denyips = OS_GetElementContent(&xml, xml_denyips);


    /* Allocating the entries */
    logr->port = (int *)calloc(rentries+1,sizeof(int));	
    logr->group = (char **)calloc(rentries+1,sizeof(char *));	
    logr->conn = (int *)calloc(rentries+1,sizeof(int));	

    node = OS_GetElementsbyNode(&xml,NULL);
    if(node == NULL)
    {
        merror("remoted: Error reading the XML.");
        OS_ClearXML(&xml);
        return(OS_CFGERR);
    }
    
    i = 0;
    while(node[i])
    {
        if(node[i]->element)
        {
            if(strcasecmp(node[i]->element,xml_remote) == 0)
            {
                XML_NODE chld_node = NULL;

                j = 0;

                if(rentries <= 0)
                {
                    merror("%s: Error reading XML nodes",ARGV0);
                    OS_ClearXML(&xml);
                    return(OS_CFGERR);
                }

                /* Initializing the config values */
                rentries--;
                logr->conn[rentries] = 0;
                logr->port[rentries] = 0;
                logr->group[rentries] = NULL;

                chld_node = OS_GetElementsbyNode(&xml,node[i]);

                while(chld_node[j])
                {
                    if((!chld_node[j]->element)||(!chld_node[j]->content))
                    {
                        merror("%s: Error reading XML child nodes",ARGV0);
                        OS_ClearXML(&xml);
                        return(OS_CFGERR);
                    }

                    else if(strcasecmp(chld_node[j]->element,
                                xml_remote_connection) == 0)
                    {
                        if(strcmp(chld_node[j]->content, "syslog") == 0)
                        {
                            logr->conn[rentries] = SYSLOG_CONN;
                        }
                        else if(strcmp(chld_node[j]->content, "secure") == 0)
                        {
                            logr->conn[rentries] = SECURE_CONN;
                        }
                        else
                        {
                            merror("%s: Invalid remote connection: %s",
                                    ARGV0, chld_node[j]->content);
                            OS_ClearXML(&xml);
                            return(OS_CFGERR);
                        }
                    }

                    else if(strcasecmp(chld_node[j]->element,
                                xml_remote_port) == 0)
                    {
                        logr->port[rentries] = atoi(chld_node[j]->content);
                    }

                    else if(strcasecmp(chld_node[j]->element,
                                xml_remote_group) == 0)
                    {
                        logr->group[rentries] = 
                            strdup(chld_node[j]->content);                
                    }

                    else
                    {
                        merror("%: Invalid entry '%s' for remoted",
                                ARGV0,chld_node[j]->element);
                        OS_ClearXML(&xml);
                        return(OS_CFGERR);
                    }

                    j++;
                } /* while chld_node */

                if(logr->conn[rentries] == 0)
                {
                    merror("%s: No connection in the remote configuraton", ARGV0);
                    OS_ClearXML(&xml);
                    return(OS_CFGERR);
                }

                /* Clearing chld_node */
                OS_ClearNode(chld_node);            
            }
        }

        i++;
    }
    
    OS_ClearNode(node);
    OS_ClearXML(&xml);

    return(fentries);
}


/* EOF */
