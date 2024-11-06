

// Standard headers

#ifdef _WIN32
#include <windows.h>
#undef  ABC   // we have conflict with char *ABC now renamed __ABC
#else
#include <unistd.h>
#endif

#include <pthread.h>
// libxml2 headers
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/threads.h>
int rarmain(int argc, char* argv[]);

#define MAX_ARGS 5
#define MAX_THREADS 32



// Default char list
const char default_ABC[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

thread_local char* rABC  = (char*)default_ABC;;
thread_local int ABCLEN;


// Command checks the type of file
const char CMD_DETECT[] = "file -i -b %s";

// File extensions
// "" is the end of the list
const char *TYPE[] = { "rar", "7z", "zip", "" };

// File Types
const char *MIME[] = { "application/x-rar;", "application/x-7z-compressed;", "application/zip;", "" };

// Commnds for each file type
const char *CMD[] = { "unrar t -y -p%s %s 2>&1", "7z t -y -p%s %s 2>&1", "unzip -P%s -t %s 2>&1", "" };

// Max password length
#define PWD_LEN 100

char *getfirstpassword();
void crack_start(unsigned int threads);



char password[PWD_LEN+1] = {'\0','\0'}; //this contains the actual password
char password_good[PWD_LEN+1] = {'\0', '\0'};  //this changed only once, when we found the good passord
unsigned int curr_len = 1; //current password length
long counter = 0;	//this couning probed passwords
xmlMutexPtr pwdMutex;	//mutex for password char array
char filename[255];	//the archive file name
char statname[259];	//status xml file name filename + ".xml"
xmlDocPtr status;
int finished = 0;
xmlMutexPtr finishedMutex;
char finalcmd[300] = {'\0', '\0'}; //this depending on arhive file type, it's a command to test file with password

char *getfirstpassword() {
    static char ret[2];
    ret[0] = rABC[0];
    ret[1] = '\0';
    return (char*) &ret;
}

void savestatus() {
    xmlNodePtr root = NULL;
    xmlNodePtr node = NULL;
    xmlChar* tmp = NULL;
    if ((strlen(statname) > 0) && status) {
        root = xmlDocGetRootElement(status);
        if (root) {
            xmlMutexLock(finishedMutex);
            for (node = root->children; node; node = node->next) {
                if (xmlStrcmp(node->name, (const xmlChar*)"current") == 0) {
                    xmlMutexLock(pwdMutex);
                    tmp = xmlEncodeEntitiesReentrant(status, (const xmlChar*) &password);
                    xmlMutexUnlock(pwdMutex);

                    if (node->children) {
                        if (password[0] == '\0') {
                            xmlNodeSetContent(node->children, (const xmlChar*)getfirstpassword());
                        } else {
                            xmlNodeSetContent(node->children, tmp);
                        }
                    }

                    xmlFree(tmp);
                } else if ((finished == 1) && (xmlStrcmp(node->name, (const xmlChar*)"good_password") == 0)) {
                    tmp =  xmlEncodeEntitiesReentrant(status, (const xmlChar*) &password_good);

                    if (node->children) {
                        xmlNodeSetContent(node->children, tmp);
                    }

                    xmlFree(tmp);
                }
            }
            xmlMutexUnlock(finishedMutex);
        }
        xmlSaveFormatFileEnc(statname, status, "UTF-8", 1);
    }
}

int abcnumb(char a) {
    int i = 0;
    for (i = 0; i < ABCLEN; i++) {
        if (rABC[i] == a) {
            return i;
        }
    }

    return 0;
}

int loadstatus() {
    xmlNodePtr root = NULL;
    xmlNodePtr node = NULL;
    xmlParserCtxtPtr parserctxt;
    int ret = 0;
    char* tmp;
    FILE* totest;
    totest = fopen(statname, "r");
    if (totest) {
        fclose(totest);
        status = xmlParseFile(statname);
    }

    if (status) {
        root = xmlDocGetRootElement(status);
    } else {
        status = xmlNewDoc(NULL);
    }

    if (root) {
        parserctxt = xmlNewParserCtxt();
        for (node = root->children; node; node = node->next) {
            if (xmlStrcmp(node->name, (const xmlChar*)"abc") == 0) {
                if (node->children && (strlen((const char*)node->children->content) > 0)) {
                    rABC = (char *)xmlStringDecodeEntities(parserctxt, (const xmlChar*)node->children->content, XML_SUBSTITUTE_BOTH, 0, 0, 0);
                } else {
                    ret = 1;
                }
            } else if (xmlStrcmp(node->name, (const xmlChar*)"current") == 0) {
                if (node->children && (strlen((const char*)node->children->content) > 0)) {
                    tmp = (char *)xmlStringDecodeEntities(parserctxt, (const xmlChar*)node->children->content, XML_SUBSTITUTE_BOTH, 0, 0, 0);
                    strcpy(password,tmp);
                    curr_len = strlen(password);
                    printf("INFO: Resuming cracking from password: '%s'\n",password);
                    xmlFree(tmp);
                } else {
                    ret = 1;
                }
            } else if (xmlStrcmp(node->name, (const xmlChar*)"good_password") == 0) {
                if (node->children && (strlen((const char*)node->children->content) > 0)) {
                    tmp = (char *)xmlStringDecodeEntities(parserctxt, node->children->content, XML_SUBSTITUTE_BOTH,0,0,0);
                    strcpy(password,tmp);
                    curr_len = strlen(password);
                    xmlMutexLock(finishedMutex);
                    finished = 1;
                    xmlMutexUnlock(finishedMutex);
                    strcpy((char*) &password_good, (char*) &password);
                    printf("GOOD: This archive was succesfully cracked\n");
                    printf("      The good password is: '%s'\n", password);
                    xmlFree(tmp);
                    ret = 1;
                }
            }
        }

        xmlFreeParserCtxt(parserctxt);
    } else {
        root = xmlNewNode(NULL, (const xmlChar*)"rarcrack");
        xmlDocSetRootElement(status, root);
        node = xmlNewTextChild(root, NULL, (const xmlChar*)"ABC", (const xmlChar*)rABC);
        node = xmlNewTextChild(root, NULL, (const xmlChar*)"current", (const xmlChar*)getfirstpassword());
        node = xmlNewTextChild(root, NULL, (const xmlChar*)"good_password", (const xmlChar*)"");
        savestatus();
    }

    return ret;
}

void nextpass2(char *p, unsigned int n) {
    int i;
    if (p[n] == rABC[ABCLEN-1]) {
        p[n] = rABC[0];

        if (n > 0) {
            nextpass2(p, n-1);
        } else {
            for (i=curr_len; i>=0; i--) {
                p[i+1]=p[i];
            }

            p[0]=rABC[0];
            p[++curr_len]='\0';
        }
    } else {
        p[n] = rABC[abcnumb(p[n])+1];
    }
}

char *nextpass() {
    //IMPORTANT: the returned string must be freed
    char *ok = (char *)malloc(sizeof(char)*(PWD_LEN+1));
    xmlMutexLock(pwdMutex);
    strcpy(ok, password);
    nextpass2((char*) &password, curr_len - 1);
    xmlMutexUnlock(pwdMutex);
    return ok;
}

void* __cdecl status_thread(void *) {
    int pwds;
    const short status_sleep = 3;
    while(1) {
#ifdef WIN32
        _sleep(status_sleep);
#else
        sleep(status_sleep);
#endif
        xmlMutexLock(finishedMutex);
        pwds = counter / status_sleep;
        counter = 0;

        if (finished != 0) {
            break;
        }

        xmlMutexUnlock(finishedMutex);
        xmlMutexLock(pwdMutex);
        printf("Probing: '%s' [%d pwds/sec]\n", password, pwds);
        xmlMutexUnlock(pwdMutex);
        savestatus();	//FIXME: this is wrong, when probing current password(s) is(are) not finished yet, and the program is exiting
    }
    return NULL;
}

void* __cdecl crack_thread(void *) {
    char *current;
    char ret[200];
    char cmd[400];
    FILE *Pipe;
    while (1) {
        current = nextpass();
        sprintf((char*)&cmd, finalcmd, current, filename);
        if (!strcmp("100", current)) {
            printf("pw='100',%s\n\r", cmd);
        }
       
        char* argv[MAX_ARGS];
        int argc = 0;

        // Use strtok to split the command line into tokens
        char* token = strtok(cmd," "); // Split by spaces
        while (token != NULL && argc < MAX_ARGS) {
            argv[argc++] = token; // Store the token in argv and increment argc
            token = strtok(NULL, " "); // Get the next token
        }
        //printf("%s %s %s %s %s\n\r", argv[0], argv[1], argv[2],argv[3], argv[4]);
        // Null-terminate the argv array
        argv[argc] = NULL;

        int r=rarmain(argc,argv);
        if (r == 0) {
            strcpy(password_good, current);
            xmlMutexLock(finishedMutex);
            finished = 1;
            printf("GOOD: password cracked: '%s'\n", current);
            xmlMutexUnlock(finishedMutex);
        }
        
        xmlMutexLock(finishedMutex);
        counter++;

        if (finished != 0) {
            xmlMutexUnlock(finishedMutex);
            break;
        }

        xmlMutexUnlock(finishedMutex);
        free(current);
    }
    return NULL;
}

void crack_start(unsigned int threads) {
    pthread_t th[MAX_THREADS+1];
    unsigned int i;

    for (i = 0; i < threads; i++) {
        (void) pthread_create(&th[i], NULL,  crack_thread, NULL);
    }

    (void) pthread_create(&th[MAX_THREADS], NULL, status_thread, NULL);

    for (i = 0; i < threads; i++) {
        (void) pthread_join(th[i], NULL);
    }

    (void) pthread_join(th[MAX_THREADS], NULL);
}

void init(int argc, char **argv) {
    int i, j;
   
   
    int help = 0;
    int threads = 1;
    int archive_type = -1;
    FILE* totest;
    char test[300];
    xmlInitThreads();
    pwdMutex = xmlNewMutex();
    finishedMutex = xmlNewMutex();
    if (argc == 1) {
        printf("USAGE: rarcrack encrypted_archive.ext [--threads NUM] [--type rar|zip|7z]\n");
        printf("       For more information please run \"rarcrack --help\"\n");
        help = 1;
    } else {
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i],"--help") == 0) {
                printf("Usage:   rarcrack encrypted_archive.ext [--threads NUM] [--type rar|zip|7z]\n\n");
                printf("Options: --help: show this screen.\n");
                printf("         --type: you can specify the archive program, this needed when\n");
                printf("                 the program couldn't detect the proper file type\n");
                printf("         --threads: you can specify how many threads\n");
                printf("                    will be run, maximum %i (default: 2)\n\n", MAX_THREADS);
                printf("Info:    This program supports only RAR, ZIP and 7Z encrypted archives.\n");
                printf("         RarCrack! usually detects the archive type.\n\n");
                help = 1;
                break;
            } else if (strcmp(argv[i],"--threads") == 0) {
                if ((i + 1) < argc) {
                    sscanf(argv[++i], "%d", &threads);
                    if (threads < 1) threads = 1;
                    if (threads > MAX_THREADS) {
                        printf("INFO: number of threads adjusted to %i\n", MAX_THREADS);
                        threads = MAX_THREADS;
                    }
                } else {
                    printf("ERROR: missing parameter for option: --threads!\n");
                    help = 1;
                }
            } else if (strcmp(argv[i],"--type") == 0) {
                if ((i + 1) < argc) {
                    sscanf(argv[++i], "%s", test);
                    for (j = 0; strcmp(TYPE[j], "") != 0; j++) {
                        if (strcmp(TYPE[j], test) == 0) {
                            strcpy(finalcmd, CMD[j]);
                            archive_type = j;
                            break;
                        }
                    }

                    if (archive_type < 0) {
                        printf("WARNING: invalid parameter --type %s!\n", argv[i]);
                        finalcmd[0] = '\0';
                    }
                } else {
                    printf("ERROR: missing parameter for option: --type!\n");
                    help = 1;
                }
            } else {
                strcpy((char*)&filename, argv[i]);
            }
        }
    }

    if (help == 1) {
        return;
    }

    sprintf((char*)&statname,"%s.xml",(char*)&filename);
    totest = fopen(filename,"r");
    if (totest == NULL) {
        printf("ERROR: The specified file (%s) is not exists or \n", filename);
        printf("       you don't have a right permissions!\n");
        return;
    } else {
        fclose(totest);
    }

    if (finalcmd[0] == '\0') {
        //when we specify the file type, the programm will skip the test
        sprintf((char*)&test, CMD_DETECT, filename);
#ifdef WIN32
        totest = _popen(test,"r");
#else
        totest = popen(test, "r");
#endif
        fscanf(totest,"%s",(char*)&test);
#ifdef WIN32
        _pclose(totest);
#else
        pclose(totest);
#endif

        for (i = 0; strcmp(MIME[i],"") != 0; i++) {
            if (strcmp(MIME[i],test) == 0) {
                strcpy(finalcmd,CMD[i]);
                archive_type = i;
                break;
            }
        }

        if (archive_type > -1 && archive_type < 3) {
            printf("INFO: detected file type: %s\n", TYPE[archive_type]);
        }
    } else {
        if (archive_type > -1 && archive_type < 3) {
            printf("INFO: the specified archive type: %s\n", TYPE[archive_type]);
        }
    }

    if (finalcmd[0] == '\0') {
        printf("ERROR: Couldn't detect archive type\n");
        return;
    }

    printf("INFO: cracking %s, status file: %s\n", filename, statname);

    if (loadstatus() == 1) {
        printf("ERROR: The status file (%s) is corrupted!\n", statname);
        return;
    }

    ABCLEN = strlen((const char *)rABC);

    if (password[0] == '\0') {
        password[0] = rABC[0];
    }

    crack_start(threads);
}

int main(int argc, char **argv) {
    // Print author
    printf("RarCrack! 0.2 by David Zoltan Kedves (kedazo@gmail.com)\n\n");
    init(argc,argv);

    if (rABC != (char*) &default_ABC) {
        xmlFree(rABC);
    }

    if (status) {
        xmlFreeDoc(status);
    }

    // Free memory
    xmlFreeMutex(pwdMutex);
    xmlFreeMutex(finishedMutex);

    // 0
    return EXIT_SUCCESS;
}
