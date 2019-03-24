//=============================================================================
// Projet : WinIDE
// Fichier: resources.h
//
//=============================================================================

#include <windows.h>
#include <commctrl.h> // -lcomctl32
#include <richedit.h>
#define TBSTYLE_FLAT 0x0800
//#define TBSTYLE_LIST 0x1000
//#define TBSTYLE_TRANSPARENT 0x8000
//#define TBSTYLE_REGISTERDROP 0x4000
#define TB_SETMAXTEXTROWS (WM_USER + 60)


//=============================================================================
//                              Constantes
//=============================================================================

#define NOM_APP                  "WinIDE"

#define VERSION_APP              "v 0.2"

#define LARG                     800
#define HAUT                     500

#define PX_ECRAN                 (GetSystemMetrics(SM_CXSCREEN)/2) - (LARG/2)
#define PY_ECRAN                 (GetSystemMetrics(SM_CYSCREEN)/2) - (HAUT/2)

#define TOOLBAR_ELEMENTS         8+3 /* +3 séparateurs */

#define TAILLE_TOOLBAR           36
#define TAILLE_STATUSBAR         20
#define TAILLE_MOINRIGHT         16
#define TAILLE_MOINBOTTOM        58

#define DOSSIER_COMPILATEUR      "MinGW"
#define CHEMIN_COMPILATEUR       "MinGW\\bin\\gcc.exe"

#define IDI_ICONE                100

/* Menu */
#define IDM_OUVRIR               101
#define IDM_ENREGISTRER          102
#define IDM_ENREGISTRER_SOUS     103
#define IDM_COMPILER             104
#define IDM_COMPILER_EXECUTER    105
#define IDM_EXECUTER             106
#define IDM_QUITTER              107
#define IDM_ANNULER              108
#define IDM_COLORATION           109
#define IDM_SHOW_TOOLBAR         110
#define IDM_SHOW_LINEBAR         111
#define IDM_EXEMPLE1             112
#define IDM_APROPOS              113
#define IDM_POLICE               114
#define IDM_TOUT_SELECTIONNER    115
#define IDM_COPIER               116
#define IDM_COUPER               117
#define IDM_COLLER               118
#define IDM_RECOLORIER           119
#define IDM_TOUTENNOIR           120

/* ToolBar Bitmaps */
#define IDB_OUVRIR               121
#define IDB_ENREGISTRER          122
#define IDB_COMPILER             123
#define IDB_EXECUTER             124
#define IDB_COMPILER_EXECUTER    125
#define IDB_ANNULER              126
#define IDB_RECOLORIER           127
#define IDB_TOUTENNOIR           128

/* Popup Menu Bitmaps */
#define IDB_OUVRIR_16            129
#define IDB_ENREGISTRER_16       130
#define IDB_COMPILER_16          131
#define IDB_EXECUTER_16          132
#define IDB_COMPILER_EXECUTER_16 133

/* StatusBar */
#define IDS_STATUS_BAR           134

/* RichEdit */
#define IDE_RICHEDIT             135

/* Hook clavier */
#define VK_S                     0x53
#define VK_O                     0x4F

//=============================================================================
//                          Variables globales
//=============================================================================

HINSTANCE hInst;

HWND hwndGeneral;

HANDLE hHook;

DWORD TempPremierAppui;

BOOL ACTIVE_COLORATION, SHOW_TOOLBAR, TOUCHE_CTRL, FOLLOW_COLORATION;

char LastOpenedFile[MAX_PATH], LastCompiledFile[MAX_PATH];

short dieze_preprocesseur = 0, doubleCoteOuvert = 0, commentaireOuvert = 0, commentaireUneLigne = 0; /* normalement un booléan suffirait mais je n'arrive pas
                                                                                                        a les initialisés par FALSE içi, ché pas pq vrm... */
//=============================================================================
//                              Prototypes
//=============================================================================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void creerToolBarButton(HWND htb, TBBUTTON *tbb, UINT i, UINT bitmap, UINT id, char *nom);
void creerToolBarSeparateur(TBBUTTON *tbb, UINT i);
BOOL ouvrirFichier(HWND hwnd, HWND hEdit);
BOOL enregistrerFichierSous(HWND hwnd, HWND hEdit);
BOOL enregistrerFichier(char *nomFichier, HWND hEdit);
char *extrairePath(char *chaine);
char *extraireExtension(char *chaine);
char *extraireNom(char *chaine);
BOOL APIENTRY AproposDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
__declspec(dllexport) LRESULT CALLBACK HookProc(int Format, WPARAM WParam, LPARAM LParam);
BOOL changerPolice(HWND hwnd, HWND hEdit, LOGFONT *lf);
int colorierTexte(HWND hwnd, char cDebut, char cFin, char *texte, int texteSize, COLORREF couleur);
int colorierCaractere(HWND hwnd, char c, char *texte, int texteSize, COLORREF couleur);
void changerCouleurSelection(HWND hEdit, CHARRANGE Selection, COLORREF couleur);
int retournerNombreDeLigne(char *buffer, int sortie);
void activerColorationSyntaxique(HWND hEdit);
void texteCouleurParDefaut(HWND hRichEdit);
void GetMyDirectory(char *path, unsigned int taille);

//=============================================================================
//                                Threads
//=============================================================================

DWORD WINAPI Compiler(LPVOID lParam);
