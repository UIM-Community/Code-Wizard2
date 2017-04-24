/************************************************************************
 * CodeWizard:  C
 * This code was generated with the Nimsoft CodeWizard version 1.70
 * Date: lundi 24 avril 2017
 ************************************************************************/

#include <nimbus.h>
#include <nim.h>
#include <port.h>

/* Global variables */
char *gProbeName;
char *gVersion;
char *gCfgFile;
char *gLogfile;
int   gLoglevel;
int   gLogsize;
int   gInterval;

/* The ProbeFinished variable is used when a signal is received
 * and the probe must terminate. */
int gProbeFinished = FALSE;

/****************************************************
 * read_setup - read elements from configuration file.
 ****************************************************/
int read_setup (char *file) {
    cfgHandle *cfgh=NULL;

    if (!(cfgh = cfgOpen (file, TRUE))) return FALSE;

    /* Note: minor memory leak here (gLogfile is not freed) to make sample code clearer */
    gLogfile  = cfgKeyReadStr (cfgh, "/setup", "logfile", gLogfile);
    gLoglevel = cfgKeyReadInt (cfgh, "/setup", "loglevel", gLoglevel);
    gLogsize  = cfgKeyReadInt (cfgh, "/setup", "logsize", gLogsize);
    gInterval = cfgKeyReadInt (cfgh, "/setup", "interval", gInterval);

    cfgClose(cfgh);
    return TRUE;
}

/****************************************************
 * cmd_list_entries - Callback for the 'list_entries'
 * command made available by the probe.
 ****************************************************/
int cmd_list_entries (NIMCB *CB) {
    int Status = NIME_OK;
    PDS *Ret=NULL, *Entry=NULL;
    char name[64] = "";
    int cnt = 0;

    nimLog (1, "%s called from %s", CB->command, CB->msg->fromName);

    if (!(Ret = pdsCreate ())) {
        Status = NIME_NOMEM;
    } else {
        if (!(Entry = pdsCreate ())) {
            Status = NIME_NOMEM;
        } else {
            /* Create 5 dummy entries */
            for (cnt=1; cnt <= 5; cnt++) {
                sprintf(name,"test-%d",cnt);
                pdsPut_PCH (Entry, "name", name);
                pdsPut_PCH (Entry, "type", "value");
                pdsPut_INT (Entry, "value", (3 * cnt));
                pdsPut_LONG(Entry, "at", time (NULL));

                pdsPutTable (Ret, PDS_PDS, "entries", Entry);
                pdsReset (Entry); /* re-use Entry */
            }
        }
    }

    nimSendReply (CB->msg, Status, Ret);
    if (Ret)   pdsDelete (Ret);
    if (Entry) pdsDelete (Entry);
    return 0;
}

/*************************************************************
 * cmd_set_loglevel - callback function modifying the loglevel
 * the names (and types) of arguments passed in args are  
 * specified in the call to nimSessionAddCallbackPds in the 
 * main function.
 *************************************************************/
int cmd_set_loglevel (NIMCB *cb, PDS *args) {
    int Status = NIME_OK;
    int level = 0;

    if (args) {
        pdsGet_INT(args,"level",&level);
    }
    if (level < 0) {
        Status = NIME_INVAL;
    } else {
        nimLog (1, "%s change level from %d to %d", cb->command, gLoglevel, level);

        gLoglevel = level;
        nimLogSetLevel (level);
    }
    nimSendReply (cb->msg, Status, NULL);
    return NIME_OK;
}

/*********************************************************************
 * cmd_stop - standard callback function called by deactivate/(_stop)
 *********************************************************************/
int cmd_stop (NIMCB *cb) {
    int Status = NIME_OK;

    nimLog (1, "%s from %s", cb->command, cb->msg->fromName);

    nimSendReply (cb->msg, Status, NULL);
    return NIMSW_EXIT;
}

/************************************************************************
 * cmd_restart - standard callback function called by restart/(_restart)
 ************************************************************************/
int cmd_restart(NIMCB *cb) {
    int Status = NIME_OK;

    nimLog (1, "%s from %s", cb->command, cb->msg->fromName);

    /* configuration file may have changed, read it again */
    if (read_setup (gCfgFile)) {
        nimLogSet (gLogfile, gProbeName, gLoglevel, 0);
        /* Automatic log file truncation when file is gLogsize KB */
        nimLogTruncateSize (gLogsize * 1024);
    } else {
        nimLog (0, "Failed to open/read configuration file %s.", gCfgFile);
    }

    nimSendReply (cb->msg, Status, NULL);
    return NIME_OK;
}

/*****************************************************
 * publish_qos - example on how to send QoS definition
 *              and QoS value. 
 *****************************************************/
void publish_qos (char *Target, long SampleValue) {
    static int DefinitionSent = FALSE;
    NIMQOS *QoS=NULL;
CIHANDLE *pCI = NULL;
char ciMetric[64] = "";
    int Interval = gInterval; /* Check interval (in seconds) */
    char   *Source  = NULL;      /* Where the QoS originates (default host) */

    if (!DefinitionSent) {
        /* ONLY SEND QoS DEFINITION ONCE !! */
        nimQoSSendDefinition ("QOS_TEST",                 /* QOS Name */
                              "QOS_APPLICATION",          /* QOS Group */
                              "My Application Response",  /* QOS Description */
                              "Milliseconds",             /* Unit */
                              "ms",                       /* Short unit */
                              NIMQOS_DEF_NONE);             /* Flags */

        DefinitionSent = TRUE;
    }
    if (!(QoS = nimQoSCreate ("QOS_TEST", Source, Interval,-1))) {
        nimLog (0, "(publish_qos) Failed to create NIMQOS");
        return;
    }
pCI = ciOpenLocalDevice("CI Type","CI Name");
 /* If user wants to monitor remotely deployed probe use 
     pCI = ciOpenRemoteDevice("CI Type","CI Name","Hostname");  */
if (pCI) {
     nimLog(2,"SUCCESS: OpenLocalDevice handle");
} else {
     nimLog(0,"FAILED: OpenLocalDevice handle");
}
     ciBindQoS(pCI,QoS,"ciMetric");
    if (SampleValue < 0) {
        nimQoSSendNull (QoS, Target);
    } else {
        nimQoSSendValue (QoS, Target, SampleValue);
    }

    nimQoSFree (QoS);

if (pCI) {
     ciClose(pCI);
     pCI = NULL;
}

}

/***************************************************
 * do_work - function is called when timeout expires
 ***************************************************/
void do_work () {
    static time_t last = 0;
    time_t now = time(NULL);

    if ((last + gInterval) <= now) {
        last = now;
        nimLog (2, "(do_work) - Work in progress...");

        /* Send a QoS value based on the current time */
        publish_qos ("example_probe",now % 1000);

        /* ADD YOUR CODE HERE */
    }
}

/************************************
 * Terminate - Trap handler function
 ************************************/
int Terminate (int iSig) {

    /* Note: you should not do I/O in a signal handler.
     * Calls to nimLog will cause a deadlock if nimLog was 
     * running when the signal was received! */

    gProbeFinished = TRUE;
    return 0;
}

/********************************************************
 * list_robots - example on how to perform calls to other
 *               Nimsoft components
 ********************************************************/
int list_robots () {
    PDS *psIn=NULL, *psOut=NULL, *psEntry=NULL;
    int i=0, iRes=0, iWait=30;
    char *Tmp=NULL, *HubDomain=NULL, *HubName=NULL, *HubRobotName=NULL;
    const char *Err=NULL;
    char zHubAddress [512]="";

    /* Issue the gethub command to the local controller */
    iRes = nimNamedRequest ("controller", "gethub", psIn, &psOut, iWait);
    if (iRes != NIME_OK) {
        Err = nimError2Txt (iRes);
        nimLog (0, "Unable to contact controller - %s", Err);
        return FALSE;
    }

    /* Get the hubdomain, hubname and hubrobot entries from the returned PDS */
    if (pdsGet_PCH (psOut, "hubdomain", &HubDomain) != PDS_ERR_NONE) {
        nimLog (0, "Could not find hub domain");
        pdsDelete (psOut);
        return FALSE;
    }
    if (pdsGet_PCH (psOut, "hubname", &HubName) != PDS_ERR_NONE) {
        nimLog (0, "Could not find hub name");
        free (HubDomain);
        pdsDelete (psOut);
        return FALSE;
    }
    if (pdsGet_PCH (psOut, "hubrobotname", &HubRobotName) != PDS_ERR_NONE) {
        nimLog (0, "Could not find hub robot name");
        free (HubDomain);
        free (HubName);
        pdsDelete (psOut);
        return FALSE;
    }
    pdsDelete (psOut);
    psOut = NULL;

    /* Build a full Nimsoft address for the Hub */
    sprintf (zHubAddress, "/%s/%s/%s/hub", HubDomain, HubName, HubRobotName);
    free (HubDomain);
    free (HubName);
    free (HubRobotName);

    /* Issue the getrobots command to the hub */
    iRes = nimNamedRequest (zHubAddress, "getrobots", psIn, &psOut, iWait);
    if (iRes != NIME_OK) {
        Err = nimError2Txt (iRes);
        nimLog (0, "Unable to contact %s - %s", zHubAddress, Err);
        return FALSE;
    }

    /* Loop through the robotlist in the return PDS one element at a time */
    psEntry = NULL;
    i = 0;
    while (pdsGetTable (psOut, PDS_PDS, "robotlist", i, &psEntry) == PDS_ERR_NONE) {
        if (pdsGet_PCH (psEntry, "name", &Tmp) == PDS_ERR_NONE) {
            nimLog (2, "Found robot named %s", Tmp);
            free (Tmp);
        }
        pdsDelete (psEntry);
        i++;
    }

    pdsDelete (psOut);
    return TRUE;
}

/********************************************************************
 * cmd_subscribe - this function is called on every incoming message.
 ********************************************************************/
int cmd_subscribe (NIMCB *cb) {
    char    *from = cb->msg->fromName;
    PDS     *pdsUserData = NULL, *pdsMsg = cb->msg->udata;

    nimSendReply (cb->msg, NIME_OK, NULL);
    if (pdsGet_PDS (pdsMsg, "udata", &pdsUserData)) {
        nimLog (0, "Packet missing 'udata' element from %s", from);
        return NIMSW_ERROR;
    }
    nimLogPds (pdsUserData, "pdsUserData", 1, 0);

    /* ADD YOUR MESSAGE DECODING CODE HERE */

    pdsDelete (pdsUserData);
    return 0;
}

/************************************************************
 * subscriber_cb - this function is called on channel errors.
 ************************************************************/
int subscriber_cb (NIMSESS *nims, int err) {
    switch (err) {
    case NIMSW_ERROR:
         nimLog (1, "The subscriber-channel was disconnected by HUB");
         return NIMSW_ERROR;
         break;
    }
    return 0;
}

/****************************************************************
 * init_subscriber - initiates a subscriber channel to the closest
 * Nimsoft Hub. The probe attempts a connection to a fixed queue
 * (same name as probe) using 'attach', and defaults to a
 * 'subscribe' if the queue does not exist.
 ****************************************************************/
int init_subscriber (NIMSESSLIST * list) {
    if (nimHubAttach (list, gProbeName, gProbeName, NULL, cmd_subscribe, subscriber_cb) != NIME_OK) {
        if (nimHubSubscribe (list, "alarm", gProbeName, NULL, cmd_subscribe, subscriber_cb) != NIME_OK) {
            return FALSE;
        }
    }
    return TRUE;
}

/***************
 * main program
 ***************/
int main (int argc, char **argv) {
    extern char *optarg;
    extern int optind;
    NIMSESS *nims=NULL;
    NIMSESSLIST *list=NULL;
    NIMINFO niminfo;
    int connected = 0;
    int opt=0;

    gProbeName = strdup ("example_probe");
    gVersion = strdup ("1.00");
    gCfgFile = strdup ("example_probe.cfg");
    gLogfile = strdup ("example_probe.log");
    gLoglevel = 0;
    gLogsize = 100;
    gInterval = 300;

    nimInit (0);

    nimLogSet (gLogfile, gProbeName, 0, 0);
    /* Automatic log file truncation when file is gLogsize KB */
    nimLogTruncateSize (gLogsize * 1024);

    /* Fill in structure used for deault probe information and
     * standard callback functions. */
    niminfo.name        = gProbeName;
    niminfo.version     = gVersion;
    niminfo.company     = "Nimsoft Corporation";
    niminfo.stop        = cmd_stop;
    niminfo.restart     = cmd_restart;
    niminfo.callback    = NULL;

    /* Read the probe configuration file. */
    if (!read_setup (gCfgFile)) {
        nimLog (0, "Failed to open/read configuration file %s.", gCfgFile);
    }

    /* Parse command-line options */
    while ((opt = getopt (argc, argv, "d:l:Vh")) != EOF) {
        switch (opt) {
        case 'd':                   /* Set default debug level */
            gLoglevel = atoi( optarg);
            break;
        case 'l':                   /* Override log file */
            if (gLogfile) free (gLogfile);
            gLogfile = strdup (optarg);
            break;
        case 'V':                   /* Get version information */
            printf ("%s %s\n", gProbeName, gVersion);
            exit (0);
            break;
        case 'h':
        default:
            printf ("usage: %s [dlVh]\n", gProbeName);
            exit (1);
            break;
        }
    }

    nimLogSet (gLogfile, gProbeName, gLoglevel, 0);
    /* Automatic log file truncation when file is gLogsize KB */
    nimLogTruncateSize (gLogsize * 1024);
    nimSetTrapHandler (Terminate);

    nimLog (0, "****************[ Starting ]****************");
    nimLog (1, "%s %s", niminfo.name, niminfo.version);
    nimLog (1, "%s", niminfo.company);

    if (!(list = nimSessionNewList())) {
        nimLog (0, "Unable to create new session list.");
        exit (1);
    }

    if (!(nims = nimSession(NULL,NIMPORT_ANY))) {
        nimLog (0, "nimSession failed");
        exit(1);
    }
    nimLog (1, "port=%d PID=%d", nims->iPort, getpid());

    /* Register active probe with robot controller */
    if (nimRegisterProbe(gProbeName,nims->iPort)) {
        nimLog (0, "nimRegisterProbe failed for %s on port %d", gProbeName, nims->iPort);
        exit(1);
    }

    /* Add standard callbacks using the 'niminfo' struct.*/
    nimSessionAddStdCallback (nims, &niminfo,0);

    /* Register user-callback functions */
    nimSessionAddCallbackPds (nims,"set_loglevel",cmd_set_loglevel,"level%d",0);
    nimSessionAddCallbackPds (nims, "list_entries", cmd_list_entries, "", NIMP_READ);

    /* Add session (nims) to session-list */
    nimSessionAddList (list,nims);

    /* Send an Alarm 
     * If you want to be able to clear the alarm from your probe you must
     * use the suppression/checkpoint mechanism. 
     * If you do not require a suppression key use NULL, not an empty string.
     * If you want to impersonate another system (e.g. if you are monitoring 
     * remotely) set the impersonate variable. */ 
    {
        char chkpoint[300]="",*id=NULL,*impersonate=NULL;
        int i=0;
CIHANDLE *pCI = NULL;
char ciMetric[64] = "";

        /* Checkpoint should be unique for each alarm situation */
        sprintf (chkpoint,"%s|checkpoint",gProbeName);

pCI = ciOpenLocalDevice("CI Type","CI Name");
 /* If user wants to monitor remotely deployed probe use 
     pCI = ciOpenRemoteDevice("CI Type","CI Name","Hostname");  */
if (pCI) {
     nimLog(2,"SUCCESS: OpenLocalDevice handle");
} else {
     nimLog(0,"FAILED: OpenLocalDevice handle");
}
        /* Generate critical alarm (get message-id 'id') */
        i = ciAlarm (pCI,"CIMetricID","Alarm with suppression tag generated from C",NIML_CRITICAL,chkpoint,"1.2.3",impersonate,&id);
        if (i != NIME_OK) {
            nimLog (0, "Failed to send alarm: %s", nimError2Txt (i));
        } else {
            nimLog (1, "Alarm sent with id %s",id);
        }
        if (id) free(id);

        /* Clear previous critical alarm(s) that have the same checkpoint */
        i = ciAlarm (pCI,"CIMetricID","Alarm with suppression tag cleared from C",NIML_CLEAR,chkpoint,"1.2.3",impersonate,NULL);
        if (i != NIME_OK) {
            nimLog (0, "Failed to send clear alarm: %s", nimError2Txt (i));
        } else {
            nimLog (1, "Clear alarm sent");
        }
        if (impersonate) free(impersonate);

if (pCI) {
     ciClose(pCI);
     pCI = NULL;
}

    }

    /* Publish a Message 
     * Create a PDS, add two strings and a number to the PDS.
     * Post it using the subject provided. */
    {
        PDS *pds=NULL;

        if (pds = pdsCreate()) {
            pdsPut_PCH (pds,"name","Donald Duck");
            pdsPut_PCH (pds,"phone","123-123-123");
            pdsPut_INT (pds,"age",60);

            nimPostMessage ("YOUR_SUBJECT",1,NULL,pds,NULL);
            pdsDelete (pds);
        }
     }

    /* Perform nimsoft calls */
    list_robots ();

    if (!(connected = init_subscriber(list))) {
        nimLog (0, "Failed to initialize subscriber...");
        exit(1);
    }

    while (!gProbeFinished) {
        switch (nimSessionDispatch (list, 5000, TRUE)) {
        case NIMSW_TIMEOUT:
            if (! connected && !(connected = init_subscriber(list))) {
                    nimLog(0, "Failed to re-connect subscriber...");
            }
            do_work();
            break;
        case NIMSW_ERROR:
            if (!(connected = init_subscriber(list))) {
                nimLog (0, "Failed to re-connect subscriber...");
            }
            break;
        case NIMSW_EXIT:
            gProbeFinished = TRUE;
            break;
        }
    }

    nimLog (0, "****************[ Terminating ]****************");

    nimSessionFreeList (&list);
    nimUnRegisterProbe (gProbeName);
    nimEnd (0);

    if (gProbeName) free (gProbeName);
    if (gVersion) free (gVersion);
    if (gCfgFile) free (gCfgFile);
    if (gLogfile) free (gLogfile);
    exit (0);
}
