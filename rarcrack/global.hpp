#ifndef _RAR_GLOBAL_
#define _RAR_GLOBAL_

#ifdef INCLUDEGLOBAL
  #define EXTVAR
#else
  #define EXTVAR extern
#endif

EXTVAR thread_local ErrorHandler ErrHandler;



#endif
