/****************************************************************************
 *          dcc project general header
 * (C) Cristina Cifuentes, Mike van Emmerik
 ****************************************************************************/
#pragma once
#include <llvm/ADT/ilist.h>
#include <bitset>
#include "Enums.h"
#include "types.h"
#include "ast.h"
#include "icode.h"
#include "locident.h"
#include "error.h"
#include "graph.h"
#include "bundle.h"
#include "Procedure.h"
#include "BasicBlock.h"

typedef llvm::iplist<Function> FunctionListType;
typedef FunctionListType lFunction;
typedef lFunction::iterator ilFunction;


/* SYMBOL TABLE */
struct SYM
{
    SYM() : label(0),size(0),flg(0),type(TYPE_UNKNOWN)
    {

    }
    char        name[10];   /* New name for this variable   */
    uint32_t       label;      /* physical address (20 bit)    */
    int         size;       /* maximum size                 */
    uint32_t     flg;        /* SEG_IMMED, IMPURE, WORD_OFF  */
    hlType      type;       /* probable type                */
    eDuVal      duVal;      /* DEF, USE, VAL                */
};

typedef std::vector<SYM> SYMTAB;
/* CALL GRAPH NODE */
struct CALL_GRAPH
{
        ilFunction proc;               /* Pointer to procedure in pProcList	*/
        std::vector<CALL_GRAPH *> outEdges; /* array of out edges                   */
public:
        void write();
        CALL_GRAPH() : outEdges(0)
        {
        }
public:
        void writeNodeCallGraph(int indIdx);
        boolT insertCallGraph(ilFunction caller, ilFunction callee);
        boolT insertCallGraph(Function *caller, ilFunction callee);
        void insertArc(ilFunction newProc);
};
//#define NUM_PROCS_DELTA		5		/* delta # procs a proc invokes		 	*/
//extern std::list<Function> pProcList;
extern FunctionListType pProcList;
extern CALL_GRAPH * callGraph;	/* Pointer to the head of the call graph     */
extern bundle cCode;			/* Output C procedure's declaration and code */

/**** Global variables ****/

extern char *asm1_name, *asm2_name; /* Assembler output filenames 		*/

typedef struct {            /* Command line option flags */
    unsigned verbose        : 1;
    unsigned VeryVerbose    : 1;
    unsigned asm1           : 1;    /* Early disassembly listing */
    unsigned asm2           : 1;    /* Disassembly listing after restruct */
    unsigned Map            : 1;
    unsigned Stats          : 1;
    unsigned Interact       : 1;    /* Interactive mode */
    unsigned Calls          : 1;    /* Follow register indirect calls */
    char	filename[80];			/* The input filename */
} OPTION;

extern OPTION option;       /* Command line options             */
extern SYMTAB symtab;       /* Global symbol table              */

struct PROG /* Loaded program image parameters  */
{
    int16_t     initCS;
    int16_t     initIP;     /* These are initial load values    */
    int16_t     initSS;     /* Probably not of great interest   */
    int16_t     initSP;
    bool        fCOM;       /* Flag set if COM program (else EXE)*/
    int         cReloc;     /* No. of relocation table entries  */
    uint32_t *  relocTable; /* Ptr. to relocation table         */
    uint8_t *   map;        /* Memory bitmap ptr                */
    int         cProcs;     /* Number of procedures so far      */
    int         offMain;    /* The offset  of the main() proc   */
    uint16_t    segMain;    /* The segment of the main() proc   */
    bool        bSigs;		/* True if signatures loaded		*/
    int         cbImage;    /* Length of image in bytes         */
    uint8_t *   Image;      /* Allocated by loader to hold entire
                             * program image                    */
};

extern PROG prog;   		/* Loaded program image parameters  */
extern std::bitset<32> duReg[30];   /* def/use bits for registers		*/

//extern uint32_t duReg[30];		/* def/use bits for registers		*/
extern uint32_t maskDuReg[30];	/* masks off du bits for regs		*/

/* Registers used by icode instructions */
static constexpr const char *allRegs[21] = {"ax", "cx", "dx", "bx", "sp", "bp",
                                    "si", "di", "es", "cs", "ss", "ds",
                                    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
                                    "tmp"};

/* Memory map states */
#define BM_UNKNOWN  0   /* Unscanned memory     */
#define BM_DATA     1   /* Data                 */
#define BM_CODE     2   /* Code                 */
#define BM_IMPURE   3   /* Used as Data and Code*/

/* Intermediate instructions statistics */
struct STATS
{
        int		numBBbef;		/* number of basic blocks initially 	       */
        int		numBBaft;		/* number of basic blocks at the end 	       */
        int		nOrder;			/* n-th order								   */
        int		numLLIcode;		/* number of low-level Icode instructions      */
        int		numHLIcode; 	/* number of high-level Icode instructions     */
        int		totalLL;		/* total number of low-level Icode insts       */
        int		totalHL;		/* total number of high-level Icod insts       */
};

extern STATS stats; /* Icode statistics */


/**** Global function prototypes ****/

void    FrontEnd(char *filename, CALL_GRAPH * *);            /* frontend.c   */
void   *allocMem(int cb);                                   /* frontend.c   */

void    udm(void);                                          /* udm.c        */
void    freeCFG(BB * cfg);                                  /* graph.c      */
BB *    newBB(BB *, int, int, uint8_t, int, Function *);      /* graph.c      */
void    BackEnd(char *filename, CALL_GRAPH *);              /* backend.c    */
char   *cChar(uint8_t c);                                      /* backend.c    */
eErrorId scan(uint32_t ip, ICODE &p);                          /* scanner.c    */
void    parse (CALL_GRAPH * *);                             /* parser.c     */

int     strSize (uint8_t *, char);                             /* parser.c     */
void    disassem(int pass, Function * pProc);              /* disassem.c   */
void    interactDis(Function * initProc, int initIC);      /* disassem.c   */
bool   JmpInst(llIcode opcode);                            /* idioms.c     */
queue::iterator  appendQueue(queue &Q, BB *node);                  /* reducible.c  */

void    SetupLibCheck(void);                                /* chklib.c     */
void    CleanupLibCheck(void);                              /* chklib.c     */
bool    LibCheck(Function &p);                            /* chklib.c     */

/* Exported functions from procs.c */
boolT	insertCallGraph (CALL_GRAPH *, ilFunction, ilFunction);
void	allocStkArgs (ICODE *, int);
void	placeStkArg (ICODE *, COND_EXPR *, int);
void	adjustActArgType (COND_EXPR *, hlType, Function *);

/* Exported functions from ast.c */
std::string walkCondExpr (const COND_EXPR *exp, Function * pProc, int *);
int       hlTypeSize (const COND_EXPR *, Function *);
hlType	  expType (const COND_EXPR *, Function *);
bool      insertSubTreeReg(COND_EXPR *&, COND_EXPR *, uint8_t, LOCAL_ID *);
bool	  insertSubTreeLongReg (COND_EXPR *, COND_EXPR **, int);


/* Exported functions from hlicode.c */
std::string writeCall (Function *, STKFRAME *, Function *, int *);
char 	*writeJcond (HLTYPE, Function *, int *);
char 	*writeJcondInv (HLTYPE, Function *, int *);
int     power2 (int);

/* Exported funcions from locident.c */
boolT checkLongEq (LONG_STKID_TYPE, iICODE, int, Function *, Assignment &asgn, iICODE atOffset);
boolT checkLongRegEq (LONGID_TYPE, iICODE, int, Function *, Assignment &asgn, iICODE);
uint8_t otherLongRegi (uint8_t, int, LOCAL_ID *);
