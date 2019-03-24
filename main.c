//=============================================================================
// Projet : WinIDE
// Version: 0.2
// Fichier: main.c
// Auteur : AXeL
// Date de création : 01/12/2013 (v 0.1)
// Date de la dernière modification : 03/12/2013 - 06/12/2013
// Lacunes : Hook clavier permet 2 options au même temp, et marche même si fenêtre 
//           minimisé... (à revoir (récupérer avec le message WM_KEYDOWN) || enlever)
// Idées d'amélioration : - Ajouter Options précedent/suivant
//                        - Ajouter une barre de lignes à gauche du RichEdit
//                        - Sauvegarde avant de quitter && avant de compiler 
//                          si changement dans le RichEdit
//                        - Ajouter les options d'édition dans la barre d'outils
//                        - Soutenance du language batch
//                        - Espacement automatique si retour à la ligne
//                        - Grisé la sauvegarde du menu et de la barre d'outils si 
//                          rien a sauvegardé ou lorsqu'on vien de sauvegarder
// Mise à jour : 
//                - 08/02/2014: (v 0.2)
//                   - Ajout des fonctions de la coloration syntaxique par contre
//                     cette dernière ne marche pas parfaitement...
//                   - Maintenant quand on veut compiler ou compiler et éxécuter
//                     la sauvegarde se fait automatiquement
//                - 09/02/2014:
//                   - Problème de coloration syntaxique résolu (la coloration en
//                     même temp "just in time" causait le problème, il faut la
//                     désactiver quand on veut colorier tout le texte)
//                   - Par contre il y'a toujours des soucis dans la coloration
//                     en même temp, alors j'ai ajouter un boutton dans la toolbar
//                     pour raffraichir la coloration
//                   - Ajout du boutton "Annuler" dans la toolbar
//                   - Optimisation de la coloration syntaxique
//                   - Ajout du boutton "Tout en noir" dans la toolbar
//                - 11/02/2014:
//                   - Le programme compile maintenant depuis le compilateur inclus
//                     dans son dossier (le dossier ou se trouve notre programme)
//
//=============================================================================

#include "resources.h"


//=============================================================================
//                          Fonction principale
//=============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrecedente, LPSTR lpCmdLine, int nCmdShow)
{
   HINSTANCE hInstRich32;
   HWND hMain;
   MSG msg;
   WNDCLASS wc;
   
   InitCommonControls(); /* Initialisation des contrôles communs */
    
   hInstRich32 = LoadLibrary("RICHED32.DLL"); /* Chargement de la DLL pour les Rich Edit */
	
   if(!hInstRich32)
   {
      MessageBox(hMain, "Impossible de charger RICHEDIT32.DLL !", "Erreur", MB_OK | MB_ICONWARNING);
      return FALSE;
   }
   
   hInst = hInstance; /* Hinstance générale */
   
   wc.style = 0; 
   wc.lpfnWndProc = MainWndProc; 
   wc.cbClsExtra = 0; 
   wc.cbWndExtra = 0; 
   wc.hInstance = hInstance; 
   wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONE)); 
   wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
   wc.hbrBackground = NULL; 
   wc.lpszMenuName =  "IDEMENU"; 
   wc.lpszClassName = "IDEWinClass";
   
   if(!RegisterClass(&wc)) return FALSE;
   
   /* Création de la fenêtre principale */
   hMain = CreateWindow("IDEWinClass", NOM_APP, WS_OVERLAPPEDWINDOW, PX_ECRAN, PY_ECRAN, LARG, HAUT, NULL, NULL, hInstance, NULL); 
   
   if (!hMain) return FALSE;

   ShowWindow(hMain, nCmdShow); /* Affichage de la fenêtre */
   
   /* Initialisation pour le hook */
   TOUCHE_CTRL = FALSE;
   TempPremierAppui = 0;
   
   /* Récupération du handle du hook clavier */
   hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)HookProc, hInstance, 0);
 
   while (GetMessage(&msg, NULL, 0, 0)) 
   { 
       TranslateMessage(&msg); 
       DispatchMessage(&msg); 
   } 
    
   /* Libération de la DLL pour les Rich Edit */
   if(!FreeLibrary (hInstRich32))
      MessageBox(hMain, "Bibliothèque RICHEDIT non libérée !", "Erreur", MB_OK | MB_ICONWARNING);
      
   UnhookWindowsHookEx(hHook); /* Arrêt du hook clavier */
		
   return msg.wParam; 
}

//=============================================================================
//                         Procédure principale
//=============================================================================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    static HWND hRichEdit;
    static HWND hToolBar;
    static HWND hStatusBar;
    static HMENU hMenu;
    static LOGFONT lf;
    /* Exemple1 : "Hello World !" */
    static const char HELLO_WORLD[] = "//Headers\n"
                                      "#include <stdio.h>\n"
                                      "#include <stdlib.h>\n\n"
                                      "//Fonction principale\n"
                                      "int main (int argc, char *argv[])\n{\n"
                                      "   printf(\"Hello World !\\n\"); //Fonction qui affiche un message sur l'écran\n"
                                      "   system(\"pause>nul\"); //Commande qui demande au système d'attendre un événnement clavier\n"
                                      "   return 0;\n}\n";
    switch (uMsg)
    { 
        case WM_CREATE:
        {
             /* Handle general de la fenêtre */
             hwndGeneral = hwnd;
             /* Création de la ToolBar */
             TBADDBITMAP tbab; 
             TBBUTTON tbb[TOOLBAR_ELEMENTS];
             TBADDBITMAP bitmap;
             hToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT //| TBSTYLE_TRANSPARENT | TBSTYLE_LIST
                                       , 0, 0, 0, 0, hwnd, NULL, hInst, NULL);
           
             tbab.hInst =  HINST_COMMCTRL; 
             tbab.nID   =  IDB_STD_LARGE_COLOR; 
             SendMessage(hToolBar, TB_ADDBITMAP, 1, (WPARAM)&tbab);
             /* Création des buttons de la ToolBar */
             creerToolBarSeparateur(tbb, 0);
             bitmap.nID = IDB_OUVRIR;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 1, SendMessage(hToolBar, TB_ADDBITMAP, 1, (LPARAM)&bitmap), IDM_OUVRIR, "Ouvrir (Ctrl+O)");
             bitmap.nID = IDB_ENREGISTRER;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 2, SendMessage(hToolBar, TB_ADDBITMAP, 2, (LPARAM)&bitmap), IDM_ENREGISTRER, "Enregistrer (Ctrl+S)");
             creerToolBarSeparateur(tbb, 3);
             bitmap.nID = IDB_COMPILER;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 4, SendMessage(hToolBar, TB_ADDBITMAP, 4, (LPARAM)&bitmap), IDM_COMPILER, "Compiler (Ctrl+F9)");
             bitmap.nID = IDB_EXECUTER;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 5, SendMessage(hToolBar, TB_ADDBITMAP, 5, (LPARAM)&bitmap), IDM_EXECUTER, "Exécuter (Ctrl+F10)");
             bitmap.nID = IDB_COMPILER_EXECUTER;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 6, SendMessage(hToolBar, TB_ADDBITMAP, 6, (LPARAM)&bitmap), IDM_COMPILER_EXECUTER, "Compiler et éxécuter");
             creerToolBarSeparateur(tbb, 7);
             bitmap.nID = IDB_ANNULER;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 8, SendMessage(hToolBar, TB_ADDBITMAP, 8, (LPARAM)&bitmap), IDM_ANNULER, "Annuler");
             bitmap.nID = IDB_RECOLORIER;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 9, SendMessage(hToolBar, TB_ADDBITMAP, 9, (LPARAM)&bitmap), IDM_RECOLORIER, "Recolorier");
             bitmap.nID = IDB_TOUTENNOIR;
             bitmap.hInst = hInst;
             creerToolBarButton(hToolBar, tbb, 10, SendMessage(hToolBar, TB_ADDBITMAP, 10, (LPARAM)&bitmap), IDM_TOUTENNOIR, "Tout en noir");
             /* Taille et instructions de création des buttons */
             SendMessage(hToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
             SendMessage(hToolBar, TB_ADDBUTTONS, TOOLBAR_ELEMENTS, (LPARAM)&tbb);
             SendMessage(hToolBar, TB_SETMAXTEXTROWS, 0, 0);
             SendMessage(hToolBar, TB_SETBUTTONSIZE, 0, MAKELPARAM(36, 32)); /* Taille buttons */
             /* Création du RichEdit */
             hRichEdit = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, ""
                         , WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL //| ES_SELECTIONBAR //| ES_DISABLENOSCROLL
                         , 0, 0, 0, 0, hwnd, (HMENU)IDE_RICHEDIT, hInst, NULL); /* | ES_SUNKEN (Slim border) */
             /* Font du RichEdit */
             //HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
             ZeroMemory(&lf, sizeof(LOGFONT));
             lstrcpy(lf.lfFaceName, "Courier New");
             lf.lfHeight = -13;
             //lf.lfItalic = FALSE;
             lf.lfWeight = FW_NORMAL;
             HFONT hFont = CreateFontIndirect(&lf);
             SendMessage(hRichEdit, WM_SETFONT, (UINT)hFont, TRUE);
             /* On fixe les marges du RichEdit */
             SendMessage(hRichEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(5, 5));
             /* Interception du message EN_CHANGE pour le RichEdit */
			 SendMessage(hRichEdit, EM_SETEVENTMASK, 0, ENM_CHANGE);
             /* Création de la bar de status */
             hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hwnd, IDS_STATUS_BAR);
             /* Récupération du handle du Menu */
             hMenu = GetMenu(hwnd);
             HMENU hPopup = GetSubMenu(hMenu, 0);
             /* Bitmaps des items du 1er Popup Menu */
             SetMenuItemBitmaps(hPopup, IDM_OUVRIR, MF_BYCOMMAND, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OUVRIR_16)), NULL);
             SetMenuItemBitmaps(hPopup, IDM_ENREGISTRER, MF_BYCOMMAND, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ENREGISTRER_16)), NULL);
             SetMenuItemBitmaps(hPopup, IDM_COMPILER, MF_BYCOMMAND, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_COMPILER_16)), NULL);
             SetMenuItemBitmaps(hPopup, IDM_EXECUTER, MF_BYCOMMAND, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXECUTER_16)), NULL);
             SetMenuItemBitmaps(hPopup, IDM_COMPILER_EXECUTER, MF_BYCOMMAND, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_COMPILER_EXECUTER_16)), NULL);
             /* On coche l'item Coloration syntaxique */
             CheckMenuItem(hMenu, IDM_COLORATION, MF_CHECKED);
             ACTIVE_COLORATION = TRUE;
             FOLLOW_COLORATION = TRUE;
             /* On coche l'item Barre d'outils */
             CheckMenuItem(hMenu, IDM_SHOW_TOOLBAR, MF_CHECKED);
             SHOW_TOOLBAR = TRUE;
             /* Désactivation de l'item Barre d'alignement */
             EnableMenuItem(hMenu, IDM_SHOW_LINEBAR, MF_GRAYED);
             /* On affiche le nombre de ligne dans la bar de status */
             SendDlgItemMessage(hwnd, IDS_STATUS_BAR, SB_SETTEXT, 0, (LONG)" 1 ligne dans le fichier");
             /* Focus sur le RichEdit */
             SetFocus(hRichEdit);
             break;
        }
        case WM_CLOSE:
             DestroyWindow(hwnd);
             break;
        case WM_COMMAND:
             switch (LOWORD(wParam))
             {
                 case IDM_OUVRIR:
                      ouvrirFichier(hwnd, hRichEdit);
                      if (ACTIVE_COLORATION)
                         activerColorationSyntaxique(hRichEdit);
                      break;
                 case IDM_ENREGISTRER:
                      if (LastOpenedFile[0] != '\0') /* Si fichié ouvert */
                      {
                         enregistrerFichier(LastOpenedFile, hRichEdit);
                         break;
                      }
                      else /* Si nn enregistrer sous */
                 case IDM_ENREGISTRER_SOUS:
                      if (GetWindowTextLength(hRichEdit))
                         enregistrerFichierSous(hwnd, hRichEdit);
                      else
                         MessageBox(hwnd, "Erreur, rien à enregistrer !", NOM_APP, MB_OK | MB_ICONWARNING);
                      break;
                 case IDM_COMPILER:
                 case IDM_EXECUTER:
                 case IDM_COMPILER_EXECUTER:
                      if ((GetFileAttributes(DOSSIER_COMPILATEUR) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                         MessageBox(hwnd, "Le compilateur MinGW n'a pas été trouvé !", NOM_APP, MB_OK | MB_ICONSTOP);
                      else
                      {
                         switch (LOWORD(wParam))
                         {
                             case IDM_COMPILER:
                                  if (LastOpenedFile[0] == '\0')
                                  {
                                     if (!GetWindowTextLength(hRichEdit))
                                     {
                                        MessageBox(hwnd, "Erreur, rien à compiler !", NOM_APP, MB_OK | MB_ICONWARNING);
                                        break;                                  
                                     }
                                     else if (!enregistrerFichierSous(hwnd, hRichEdit))
                                        break;                   
                                  }
                                  enregistrerFichier(LastOpenedFile, hRichEdit); /* Enregistrement */
                                  CreateThread(NULL, 0, Compiler, 0, 0, 0); /* Compilation */
                                  break;
                             case IDM_EXECUTER:
                                  if (LastCompiledFile[0] != '\0')
                                     ShellExecute(NULL, "open", LastCompiledFile, NULL, NULL, SW_SHOWNORMAL); /* Exécution */
                                  else
                                     MessageBox(hwnd, "Le fichier n'a pas encore été compilé !", NOM_APP, MB_OK | MB_ICONSTOP);
                                  break;
                             case IDM_COMPILER_EXECUTER:
                                  if (LastOpenedFile[0] == '\0')
                                  {
                                     if (!GetWindowTextLength(hRichEdit))
                                     {
                                        MessageBox(hwnd, "Erreur, rien à compiler !", NOM_APP, MB_OK | MB_ICONWARNING);
                                        break;                                  
                                     }
                                     else if (!enregistrerFichierSous(hwnd, hRichEdit))
                                        break;
                                  }
                                  enregistrerFichier(LastOpenedFile, hRichEdit); /* Enregistrement */
                                  CreateThread(NULL, 0, Compiler, (LPVOID)1, 0, 0); /* Compilation + éxécution */
                                  break;
                         }
                      }
                      break;
                 case IDM_QUITTER:
                      DestroyWindow(hwnd);
                      break;
                 case IDM_TOUT_SELECTIONNER:
                      SendMessage(hRichEdit, EM_SETSEL, 0, -1);
                      break;
                 case IDM_COPIER:
                      SendMessage(hRichEdit, WM_COPY, 0, 0);
                      break;
                 case IDM_COUPER:
                      SendMessage(hRichEdit, WM_CUT, 0, 0);
                      break;
                 case IDM_COLLER:
                      SendMessage(hRichEdit, WM_PASTE, 0, 0);
                      if (ACTIVE_COLORATION)
                         activerColorationSyntaxique(hRichEdit);
                      break;
                 case IDM_ANNULER:
                      SendMessage(hRichEdit, EM_UNDO, 0, 0); /* Annulation de la dernière saisie/action */
                      break;
                 case IDM_RECOLORIER:
                      if (!ACTIVE_COLORATION)
                      {
                         CheckMenuItem(hMenu, IDM_COLORATION, MF_CHECKED);
                         ACTIVE_COLORATION = TRUE;                      
                      }
                      activerColorationSyntaxique(hRichEdit);
                      break;
                 case IDM_TOUTENNOIR:
                      ACTIVE_COLORATION = TRUE;
                 case IDM_COLORATION:
                      if (ACTIVE_COLORATION)
                      {
                         CheckMenuItem(hMenu, IDM_COLORATION, MF_UNCHECKED);
                         ACTIVE_COLORATION = FALSE;
                         texteCouleurParDefaut(hRichEdit);
                      }
                      else
                      {
                         CheckMenuItem(hMenu, IDM_COLORATION, MF_CHECKED);
                         ACTIVE_COLORATION = TRUE;
                         activerColorationSyntaxique(hRichEdit);
                      }
                      break;
                 case IDM_SHOW_TOOLBAR:
                 {
                      RECT Rect;
                      GetWindowRect(hwnd, &Rect); /* Récupération du positionnement + taille de la fenêtre */
                      
                      if (SHOW_TOOLBAR)
                      {
                         CheckMenuItem(hMenu, IDM_SHOW_TOOLBAR, MF_UNCHECKED); /* On décoche le menu item Barre d'outils */
                         SHOW_TOOLBAR = FALSE;
                         ShowWindow(hToolBar, SW_HIDE); /* On cache la ToolBar */
                         MoveWindow(hRichEdit, 0, 0, Rect.right-Rect.left-TAILLE_MOINRIGHT, Rect.bottom-Rect.top-TAILLE_STATUSBAR-TAILLE_MOINBOTTOM, TRUE); /* Redimenssionnement du RichEdit */    
                      }
                      else
                      {
                         CheckMenuItem(hMenu, IDM_SHOW_TOOLBAR, MF_CHECKED); /* On coche l'item Barre d'outils */
                         SHOW_TOOLBAR = TRUE;
                         ShowWindow(hToolBar, SW_SHOW); /* On affiche la ToolBar */
                         MoveWindow(hRichEdit, 0, TAILLE_TOOLBAR, Rect.right-Rect.left-TAILLE_MOINRIGHT, Rect.bottom-Rect.top-TAILLE_TOOLBAR-TAILLE_STATUSBAR-TAILLE_MOINBOTTOM, TRUE); /* Redimenssionnement du RichEdit */
                         MoveWindow(hToolBar, 0, 0, Rect.right-Rect.left-TAILLE_MOINRIGHT, Rect.bottom-Rect.top-TAILLE_MOINBOTTOM, TRUE); /* Redimenssionnement de la ToolBar */
                      }
                      break;
                 }
                 case IDM_SHOW_LINEBAR:
                      break;
                 case IDM_EXEMPLE1:
                      if (!GetWindowTextLength(hRichEdit) || MessageBox(hwnd, "Le texte existant va être éffacé, continuer ?", NOM_APP, MB_YESNO | MB_ICONQUESTION) == IDYES) /* Si RichEdit vide ou choix d'éffacé écran */
                         SetWindowText(hRichEdit, HELLO_WORLD); /* On colle l'exemple 1 */
                      if (ACTIVE_COLORATION)
                         activerColorationSyntaxique(hRichEdit);
                      break;
                 case IDM_APROPOS:
                      DialogBox(hInst, "APROPOS", hwnd, (DLGPROC)AproposDlgProc);
                      break;
                 case IDM_POLICE:
                      changerPolice(hwnd, hRichEdit, &lf);
                      if (ACTIVE_COLORATION)
                         activerColorationSyntaxique(hRichEdit);
                      break;
             }
             switch (HIWORD(wParam))
             {
                case EN_CHANGE: /* Si Changement dans un Edit (un RichEdit dans notre cas) */
                    switch (LOWORD(wParam))
                    {
                       case IDE_RICHEDIT:
                       {
                          int bufferSize = GetWindowTextLength(hRichEdit);
                          char buffer[bufferSize+1];
                          GetWindowText(hRichEdit, buffer, bufferSize+1); /* On récupère le texte depuis le RichEdit */
                          if (ACTIVE_COLORATION && FOLLOW_COLORATION) /* Si coloration syntaxique activé */
                          {
                             /* On récupère la sélection courante */
                             CHARRANGE CurrentSelection;
                             SendMessage(hRichEdit, EM_EXGETSEL, 0, (LPARAM)&CurrentSelection);
                             if (CurrentSelection.cpMin > 0)
                                CurrentSelection.cpMin -= 1; /* On enlève toujours 1 du début de la selection à cause de '\r\n' */
                             int selection = CurrentSelection.cpMin + retournerNombreDeLigne(buffer, CurrentSelection.cpMin); /* séléction courante */
                             
                             if (buffer[selection] == '#' && (buffer[selection-1] == '\n' || buffer[selection-1] == '\0')) /* Si dieze (language préprocesseur) au début de la ligne */
                             {
                                dieze_preprocesseur = 1;
                                changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 150, 0)); /* vert */
                             }
                             else if (buffer[selection] == '\n' || buffer[selection] == '\0') /* Si nn si retour à la ligne ou chaine vide */
                             {
                                if (dieze_preprocesseur)
                                   dieze_preprocesseur = 0;
                                if (commentaireUneLigne)
                                   commentaireUneLigne = 0;
                                if (doubleCoteOuvert)
                                   doubleCoteOuvert = 0;
                             }
                             else if (buffer[selection] == '"' && dieze_preprocesseur == 0) /* Si nn si " (chaine de caractère) */
                             {
                                if (doubleCoteOuvert == 0) /* Si début de la chaine */
                                {
                                   doubleCoteOuvert = 1;
                                   changerCouleurSelection(hRichEdit, CurrentSelection, RGB(255, 0, 0)); /* rouge */
                                }
                                else /* Si fin de la chaine */
                                {
                                   doubleCoteOuvert = 0;
                                   CurrentSelection.cpMin++;
                                   changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 0, 0)); /* noir */    
                                }
                             }
                             else if (buffer[selection] == '/') /* Si nn si commentaire */
                             {                    
                                if (commentaireOuvert && buffer[selection-1] == '*') /* Si fin de commentaire sur plusieurs lignes */
                                {
                                   commentaireOuvert = 0;
                                   CurrentSelection.cpMin++;
                                   if (dieze_preprocesseur) /* Si dieze préprocesseur existe sur la même ligne */
                                      changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 150, 0)); /* vert */
                                   else
                                      changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 0, 0)); /* noir */      
                                }
                                else if (buffer[selection-1] == '/')
                                {
                                   commentaireUneLigne = 1;
                                   CurrentSelection.cpMin--;
                                   changerCouleurSelection(hRichEdit, CurrentSelection, RGB(40, 148, 255)); /* bleu ciel */
                                }
                             }
                             else if (buffer[selection-1] == '/' && buffer[selection] == '*') /* Si nn si début de commentaire sur plusieurs lignes */
                             {
                                if (commentaireOuvert == 0)
                                {
                                   commentaireOuvert = 1;
                                   CurrentSelection.cpMin--;
                                   changerCouleurSelection(hRichEdit, CurrentSelection, RGB(40, 148, 255)); /* bleu ciel */
                                }
                             }
                             else
                             {
                                if (dieze_preprocesseur == 0 && doubleCoteOuvert == 0 && commentaireOuvert == 0 && commentaireUneLigne == 0) /* Si on ne dérange pas les séléctions en haut ;) */
                                {
                                   int i, trouver = 0;
                                   for (i = '0'; i <= '9'; i++) /* On cherche si la séléction actuelle est un numéro */
                                   {
                                      if (buffer[selection] == i) /* Si ui on le colorie */
                                      {
                                         trouver = 1;
                                         changerCouleurSelection(hRichEdit, CurrentSelection, RGB(140, 0, 140)); /* mauve */
                                      }
                                   }
                                
                                   if (trouver == 0) /* Si pas de numéro trouvé */
                                   {
                                      if (buffer[selection] == '{' || buffer[selection] == '}') /* Si accolade (o/f) */
                                         changerCouleurSelection(hRichEdit, CurrentSelection, RGB(255, 0, 0)); /* rouge */
                                      else if (buffer[selection] == '(' || buffer[selection] == ')') /* Si parenthèse (o/f) */
                                         changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 0, 160)); /* bleu */
                                      else /* Si nn on change la couleur en noir (naturelement) */
                                         changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 0, 0)); /* noir */
                                   }
                                }
                             }
                          }
                          
                          if (buffer[strlen(buffer)-1] == '\n' || buffer[strlen(buffer)-1] != '\n') /* Si saut de ligne ou backSpace (effacement de saut de ligne, de lettres, ...) */
                          {
                             char nbrLignes[64];
                             /* On récupère le nombre de ligne dans le RichEdit */
                             sprintf(nbrLignes, " %d lignes dans le fichier", SendMessage(hRichEdit, EM_GETLINECOUNT, 0, 0));
                             /* On affiche le nombre de ligne dans la bar de status */
                             SendDlgItemMessage(hwnd, IDS_STATUS_BAR, SB_SETTEXT, 0, (LONG)nbrLignes);
                          }
                       } // Fin case IDE_RICHEDIT:
                       break;
                    }
                    break;
             }
             break;
        case WM_SIZE:
             if (SHOW_TOOLBAR) /* Si ToolBar activé */
             {
                MoveWindow(hRichEdit, 0, TAILLE_TOOLBAR, LOWORD(lParam), HIWORD(lParam)-TAILLE_TOOLBAR-TAILLE_STATUSBAR, TRUE); /* Redimenssionnement du RichEdit (-36 => ToolBar, -20 => StatusBar) */
                MoveWindow(hToolBar, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE); /* Redimenssionnement de la ToolBar */
             }
             else
                MoveWindow(hRichEdit, 0, 0, LOWORD(lParam), HIWORD(lParam)-TAILLE_STATUSBAR, TRUE);
             MoveWindow(hStatusBar, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE); /* Redimenssionnement de la StatusBar */
             break;
        case WM_DESTROY:
             PostQuitMessage(0);
             break;
        default: 
             return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return FALSE;
}

//=============================================================================
//               Fonction qui crée les buttons de la ToolBar
//=============================================================================

void creerToolBarButton(HWND htb, TBBUTTON *tbb, UINT i, UINT bitmap, UINT id, char *nom)
{
   tbb[i].iBitmap = bitmap;
   tbb[i].idCommand = id; 
   tbb[i].fsState = TBSTATE_ENABLED; 
   tbb[i].fsStyle = TBSTYLE_BUTTON; 
   tbb[i].dwData = 0; 
   tbb[i].iString = SendMessage(htb, TB_ADDSTRING, 0, (LPARAM)nom);
}

//=============================================================================
//              Fonction qui crée les séparateurs de la ToolBar
//=============================================================================

void creerToolBarSeparateur(TBBUTTON *tbb, UINT i)
{
   tbb[i].iBitmap = 0; 
   tbb[i].idCommand = -1; 
   tbb[i].fsState = 0; 
   tbb[i].fsStyle = TBSTYLE_SEP;
   tbb[i].dwData = 0; 
   tbb[i].iString = -1;   
}

//=============================================================================
//                    Fonction d'ouverture des fichiers
//=============================================================================

BOOL ouvrirFichier(HWND hwnd, HWND hEdit)
{
   OPENFILENAME ofn;

   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hwnd;
   ofn.lpstrFile = LastOpenedFile;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFilter = "Fichier source (*.c)\0*.c\0Tout les fichiers (*.*)\0*.*\0";
   ofn.nFilterIndex = 1;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

   if (GetOpenFileName(&ofn) == TRUE)
   {
      HANDLE hf;
      DWORD FileSize, nbcharRead;
      CHAR *buffer;
      
      hf = CreateFile(LastOpenedFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      FileSize = GetFileSize(hf, NULL);
      buffer = (PCHAR)LocalAlloc(LMEM_FIXED, FileSize+1);
      ReadFile(hf, buffer, FileSize, &nbcharRead, NULL) ;
      buffer[FileSize] = 0;
      SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)buffer);
      LocalFree(buffer);
      CloseHandle(hf);
      return TRUE;
   }
   else
      return FALSE;
}

//=============================================================================
//                 Fonctions d'enregistrement des fichiers
//=============================================================================

BOOL enregistrerFichierSous(HWND hwnd, HWND hEdit)
{
   OPENFILENAME ofn;

   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   
   #ifdef OPENFILENAME_SIZE_VERSION_400
      ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
   #else
      ofn.lStructSize = sizeof(OPENFILENAME);
   #endif
   
   ofn.hwndOwner = hwnd;
   ofn.lpstrFile = LastOpenedFile;
   ofn.nMaxFile = sizeof(LastOpenedFile);
   ofn.lpstrFilter = "Fichier source (*.c)\0*.c\0Tout les fichiers (*.*)\0*.*\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrDefExt = "c";
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
   
   if (GetSaveFileName(&ofn) == TRUE)
   {
      if (enregistrerFichier(LastOpenedFile, hEdit))
         return TRUE;
   }
      
   return FALSE;
}

BOOL enregistrerFichier(char *nomFichier, HWND hEdit)
{
      HANDLE hf;
      DWORD FileSize, nbcharRead;
      char *buffer;
      FileSize = GetWindowTextLength(hEdit);
      buffer = (char*)LocalAlloc(LMEM_FIXED, FileSize+1);
      GetWindowText(hEdit, buffer, FileSize+1);
      hf = CreateFile(nomFichier, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      WriteFile(hf, buffer, FileSize, &nbcharRead, NULL);
      CloseHandle(hf);
      LocalFree(buffer);
      return TRUE;
}

//=============================================================================
//  Fonction qui extrait le path (chemin) depuis le chemin complet du fichier
//=============================================================================

char *extrairePath(char *chaine)
{
   int i;
   char *path;
   
   path = malloc(strlen(chaine)+1);
   sprintf(path, "%s", chaine);
   for (i = lstrlen(path)-1; i >= 0; i--)
   {
      if (path[i] == '\\')
      {
         path[i] = '\0';
         break;
      }
   }
   
   return path;
}

//=============================================================================
//     Fonction qui extrait l'extension du nom/chemin complet du fichier
//=============================================================================

char *extraireExtension(char *chaine)
{
   int i;
   char *sansExtension;
   
   sansExtension = malloc(strlen(chaine)+1);
   sprintf(sansExtension, "%s", chaine);
   for (i = lstrlen(sansExtension)-1; i >= 0; i--)
   {
      if (sansExtension[i] == '.')
      {
         sansExtension[i] = '\0';
         break;
      }
   }
   
   return sansExtension;
}

//=============================================================================
//     Fonction qui extrait le nom depuis nom/chemin complet du fichier
//=============================================================================

char *extraireNom(char *chaine)
{
   int i, debut, fin, j = 0;
   char *tmp, *nom;
   
   tmp = malloc(strlen(chaine)+1);
   sprintf(tmp, "%s", chaine);
   for (i = lstrlen(tmp)-1; i >= 0; i--) /* On parcoure tmp */
   {
      if (tmp[i] == '.') /* avant '.' c'est la fin du nom */
         fin = i--;
      else if (tmp[i] == '\\') /* après '\\' c'est le debut du nom */
      {
         debut = i++;
         break;     
      }
   }
   
   nom = malloc((fin-debut)+1); 
   for (i = debut; i <= fin; i++) /* On connait le debut et la fin on les copie alors */
   {
      nom[j] = tmp[i];
      j++;
   }
   
   nom[j] = '\0'; /* Fin de la chaine nom */
   
   free(tmp); /* On vide tmp */
   
   return nom;
}

//=============================================================================
//                         Thread de compilation
//=============================================================================

DWORD WINAPI Compiler(LPVOID lParam)
{
   int tailleChemin = 200+lstrlen(CHEMIN_COMPILATEUR);
   char commande[1024], cheminCompilateur[tailleChemin];
   int typeCompilation = (int)lParam;
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   ZeroMemory(&pi, sizeof(pi));
   GetMyDirectory(cheminCompilateur, tailleChemin); /* Chemin du programme */
   lstrcat(cheminCompilateur, CHEMIN_COMPILATEUR); /* Chemin du compilateur */

   sprintf(LastCompiledFile, "%s.exe", extraireExtension(LastOpenedFile)); /* Chemin fichier compilé */
   if (typeCompilation == 1)
      sprintf(commande, "cmd /k \"%s %s -o %s&&%s\"", cheminCompilateur, LastOpenedFile, LastCompiledFile, LastCompiledFile); /* Commande de compilation + éxécution (pour fermer console "/c" au début) */ 
   else
      sprintf(commande, "cmd /k \"%s %s -o %s&& echo Fin de compilation.\"", cheminCompilateur, LastOpenedFile, LastCompiledFile); /* Commande de compilation + gardé console ouverte */
   //ShellExecute(NULL, "open", "cmd.exe", commande, NULL, SW_SHOWNORMAL); 
   CreateProcess(NULL, commande, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi); /* Compilation */
   WaitForSingleObject(pi.hProcess, INFINITE); /* On attend que la compilation prend fin */
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);
   
   return 0;   
}

//=============================================================================
//                           Apropos DialogBox
//=============================================================================

BOOL APIENTRY AproposDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
       case WM_INITDIALOG:
            /* Affichage de l'icone */
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONE)));
            break;
       case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                     EndDialog(hwnd, 0);
                     break;       
            }
            break;
   }
   
   return 0;
}

//=============================================================================
//    Fonction de gestion du hook (simulation d'injection de dll sans dll)
//=============================================================================

__declspec(dllexport) LRESULT CALLBACK HookProc(int Format, WPARAM WParam, LPARAM LParam) 
{ 
    DWORD TempSecondAppui = 0, TempIntervalle = 0;
    
    if (Format == HC_ACTION) /* Si clavier en action */
    { 
        KBDLLHOOKSTRUCT hookstruct = *((KBDLLHOOKSTRUCT*)LParam); /* Structure d'info sur la touche appuyée */
        
        if (!TOUCHE_CTRL) /* Si Ctrl non appuyée */
        {
           if((GetKeyState(VK_CONTROL) & 0x80)) /* Si Ctrl (droite/gauche) appuyée */
           {
              TOUCHE_CTRL = TRUE; /* Ctrl est appuyée */
              TempPremierAppui = GetTickCount(); /* On récupère le temp d'appui */
           }
        }
        else /* Si nn (si Ctrl appuyée) */
        {
           TempSecondAppui = GetTickCount(); /* On récupère le temp du 2ème d'appui */
           TempIntervalle = TempSecondAppui - TempPremierAppui; /* Intervalle/Laps de temp passé entre les 2 appuies */
              
           if (TempIntervalle <= 500) /* Si temp passé moin ou égal 500 ms */
           {
              if ((GetKeyState(VK_O) & 0x80)) /* Si touche O appuyée/enfoncée */
                 PostMessage(hwndGeneral, WM_COMMAND, (WPARAM)IDM_OUVRIR, 0);
              else if ((GetKeyState(VK_S) & 0x80)) /* Si touche S appuyée */
                 PostMessage(hwndGeneral, WM_COMMAND, (WPARAM)IDM_ENREGISTRER, 0);
              else if ((GetKeyState(VK_F9) & 0x80)) /* Si touche F9 appuyée */
                 PostMessage(hwndGeneral, WM_COMMAND, (WPARAM)IDM_COMPILER, 0);
              else if ((GetKeyState(VK_F10) & 0x80)) /* Si touche F10 appuyée */
                 PostMessage(hwndGeneral, WM_COMMAND, (WPARAM)IDM_EXECUTER, 0);
              else
                 TempPremierAppui = 0; /* Rénitialisation du temp du premier appui */
           }
           
           TOUCHE_CTRL = FALSE; /* Ctrl n'est plus appuyée */
        }
    } 
   
   return CallNextHookEx(hHook, Format, WParam, LParam); /* Renvoi des messages au sytème pour permettre d'autres hooks */
}

//=============================================================================
//            Fonction qui change la police de l'edit spécifié
//=============================================================================

BOOL changerPolice(HWND hwnd, HWND hEdit, LOGFONT *lf)
{
   HFONT hFont; 
   CHOOSEFONT cf;
   
   ZeroMemory(&cf, sizeof(CHOOSEFONT));
   cf.lStructSize = sizeof(CHOOSEFONT);
   cf.hwndOwner = hwnd;
   cf.lpLogFont = &(*lf);
   cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

   if (ChooseFont(&cf))
   {
      DeleteObject(hFont);
      hFont = CreateFontIndirect(&(*lf));
      SendMessage(hEdit, WM_SETFONT, (UINT)hFont, TRUE);
      return TRUE;
   }
   
   return FALSE;
}

//=============================================================================
// Ces 3 fonctions sont du copier/coller + modification depuis le projet Alice
//              (Quoi je ne vais pas tout réécrire comme même)
//=============================================================================

//=============================================================================
//       1 - Fonction qui colorie le texte spécifié par cDebut et cFin
//=============================================================================

int colorierTexte(HWND hwnd, char cDebut, char cFin, char *texte, int texteSize, COLORREF couleur)
{
   int i, cDebutTrouver = 0;
   unsigned int trouver = 0, nbrDeLigne = 0;
   CHARRANGE Selection;
   ZeroMemory(&Selection, sizeof(CHARRANGE));
    
   for (i = 0; i < texteSize; i++)
   {
      if (texte[i] == cDebut) /* Si caractère de début trouvé */
      {
         Selection.cpMin = (i-nbrDeLigne)+1; /* sans ce +1 ça ne marche pas bien (à mon avie il ne sert à rien) */
         cDebutTrouver = 1;    
      }
      else if (cDebutTrouver && texte[i] == cFin)
      {
         Selection.cpMax = (i-nbrDeLigne)+1;
         trouver++;
         changerCouleurSelection(hwnd, Selection, couleur);
         cDebutTrouver = 0;
      }
      else if (texte[i] == '\n')
         nbrDeLigne++;
   } // FIN FOR

   return trouver;
}

//=============================================================================
//             2 - Fonction qui colorie le caractère spécifié
//=============================================================================

int colorierCaractere(HWND hwnd, char c, char *texte, int texteSize, COLORREF couleur)
{
   int i;
   unsigned int trouver = 0, nbrDeLigne = 0;
   CHARRANGE Selection;
   ZeroMemory(&Selection, sizeof(CHARRANGE));
   
   for (i = 0; i < texteSize; i++)
   {
      if (texte[i] == c) /* Si caractère trouvé */
      {
         Selection.cpMin = i-nbrDeLigne;
         Selection.cpMax = (i-nbrDeLigne)+1;
         trouver++;
         changerCouleurSelection(hwnd, Selection, couleur);
      }
      else if (texte[i] == '\n')
         nbrDeLigne++;
   } // FIN FOR

   return trouver;
}

//=============================================================================
//  3 - Fonction qui change la couleur de la selection dans l'edit spécifié
//=============================================================================

void changerCouleurSelection(HWND hEdit, CHARRANGE Selection, COLORREF couleur)
{
   /* Format du texte */
   CHARFORMAT2 Format;
   ZeroMemory(&Format, sizeof(CHARFORMAT2));
   Format.cbSize = sizeof(CHARFORMAT2);
   Format.dwMask = CFM_COLOR;
   
   /* On récupère la sélection courante afin de la rétablir après le traitement */
   CHARRANGE CurrentSelection;
   SendMessage(hEdit, EM_EXGETSEL, 0, (LPARAM)&CurrentSelection);
   /* La couleur a utilisé */
   Format.crTextColor = couleur;
   /* On indique au Rich Edit que l'on va utiliser la sélection : Selection */
   SendMessage(hEdit, EM_EXSETSEL, 0, (LPARAM)&Selection);
   /* On indique au Rich Edit que l'on va utiliser le format : Format */
   SendMessage(hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&Format);  
   /* Restauration de la sélection */
   SendMessage(hEdit, EM_EXSETSEL, 0, (LPARAM)&CurrentSelection);
}

//=============================================================================
//   Fonction qui retourne le nombre de ligne des textes contenant des '\r\n'
//                     jusqu'à la séléction spécifée
//=============================================================================

int retournerNombreDeLigne(char *buffer, int sortie)
{
   int i, arret = sortie, nbrLigne = 0;
   for (i = 0; i <= arret; i++)
   {
      if (buffer[i] == '\r') /* Je calcule les '\r' car ils viennent avant les '\n', comme ça les chances de ne pas calculer une ligne diminuent */
      {
         nbrLigne++;
         arret++; /* On augmente la condition d'arrêt vu qu'il y'a un '\n' après l' '\r' */           
      }
   }
   
   return nbrLigne;
}

//=============================================================================
//       Fonction qui colorie le texte selon la coloration syntaxique
//=============================================================================

void activerColorationSyntaxique(HWND hRichEdit)
{
   int bufferSize = GetWindowTextLength(hRichEdit);
   char buffer[bufferSize+1];
   GetWindowText(hRichEdit, buffer, bufferSize+1); /* On récupère le texte depuis le RichEdit */
   CHARRANGE CurrentSelection;
   int i, len = lstrlen(buffer), nbrDeLigne = 0, saveLigne;
   
   FOLLOW_COLORATION = FALSE;
   
   for (i = '0'; i <= '9'; i++)
      colorierCaractere(hRichEdit, i, buffer, lstrlen(buffer), RGB(140, 0, 140)); /* mauve */
   colorierCaractere(hRichEdit, '(', buffer, lstrlen(buffer), RGB(0, 0, 160)); /* bleu */
   colorierCaractere(hRichEdit, ')', buffer, lstrlen(buffer), RGB(0, 0, 160)); /* bleu */
   colorierCaractere(hRichEdit, '{', buffer, lstrlen(buffer), RGB(255, 0, 0)); /* rouge */
   colorierCaractere(hRichEdit, '}', buffer, lstrlen(buffer), RGB(255, 0, 0)); /* rouge */
   
   //colorierTexte(hRichEdit, '\"', '\"', buffer, lstrlen(buffer), RGB(255, 0, 0)); /* rouge */
   //colorierTexte(hRichEdit, '/', '\r', buffer, lstrlen(buffer), RGB(40, 148, 255)); /* bleu ciel */
   //colorierTexte(hRichEdit, '#', '\r', buffer, lstrlen(buffer), RGB(0, 150, 0)); /* vert */
   
   for (i = 0; i < len; i++)
   {
      if (buffer[i] == '#' && (buffer[i-1] == '\0' || buffer[i-1] == '\n')) /* Si dieze_préprocesseur */
      {
         CurrentSelection.cpMin = i - nbrDeLigne;
         CurrentSelection.cpMax = CurrentSelection.cpMin+1;
         for (i++; i < len; i++)
         {
            if (buffer[i] == '\r') /* Si saut de ligne '\r\n' */
            {
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 150, 0)); /* vert */
               break;
            }
            else if (buffer[i] == '/' && (buffer[i+1] == '/' || buffer[i+1] == '*')) /* Si nn si commentaire */
            {
               i--;
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 150, 0)); /* vert */
               break;     
            }
            else if (buffer[i+1] == '\0') /* Si nn si fin de ligne */
            {
               CurrentSelection.cpMax++;
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(0, 150, 0)); /* vert */
               break;   
            }
            else if (buffer[i] == '\n') /* Si saut de ligne */
               nbrDeLigne++;
            CurrentSelection.cpMax++;
         }          
      }
      else if (buffer[i] == '/' && buffer[i+1] == '/') /* Si commentaire sur une ligne */
      {
         CurrentSelection.cpMin = i - nbrDeLigne;
         CurrentSelection.cpMax = CurrentSelection.cpMin+2;
         for (i += 2; i < len; i++)
         {
            if (buffer[i] == '\r')
            {
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(40, 148, 255)); /* bleu ciel */
               break;
            }
            else if (buffer[i+1] == '\0') /* i+1 car i < lstrlen(buffer) non pas ou égal, autrement on n'atteindra pas le '\0' sans i+1 */
            {
               CurrentSelection.cpMax++;
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(40, 148, 255)); /* bleu ciel */
               break;   
            }
            else if (buffer[i] == '\n') /* Si saut de ligne */
               nbrDeLigne++;
            CurrentSelection.cpMax++;
         }     
      }
      else if (buffer[i] == '/' && buffer[i+1] == '*') /* Si commentaire sur plusieurs lignes */
      {
         saveLigne = nbrDeLigne;
         CurrentSelection.cpMin = i - nbrDeLigne;
         CurrentSelection.cpMax = CurrentSelection.cpMin+2; /* + '/' et '*' */
         for (i += 2; i < len; i++)
         {
            if (buffer[i] == '*' && buffer[i+1] == '/') /* Si fin de commentaire sur plusieurs lignes */
            {
               CurrentSelection.cpMax += (2 - (nbrDeLigne - saveLigne));
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(40, 148, 255)); /* bleu ciel */
               break;
            }
            else if (buffer[i] == '\n') /* Si saut de ligne */
               nbrDeLigne++;
            CurrentSelection.cpMax++;
         }     
      }
      else if (buffer[i] == '\"') /* Si chaine de caractère */
      {
         CurrentSelection.cpMin = i - nbrDeLigne;
         CurrentSelection.cpMax = CurrentSelection.cpMin+1;
         for (i++; i < len; i++)
         {
            if (buffer[i] == '\"')
            {
               CurrentSelection.cpMax++;
               changerCouleurSelection(hRichEdit, CurrentSelection, RGB(255, 0, 0)); /* rouge */
               break;
            }
            else if (buffer[i] == '\n') /* Si saut de ligne */
               nbrDeLigne++;
            CurrentSelection.cpMax++;
         }          
      }
      
      if (buffer[i] == '\n') /* Si saut de ligne */
         nbrDeLigne++;
   }
   
   FOLLOW_COLORATION = TRUE;
}

//=============================================================================
//       Fonction qui met tout le texte en couleur par défaut (le noir)
//=============================================================================

void texteCouleurParDefaut(HWND hRichEdit)
{
   CHARRANGE Selection;
   Selection.cpMin = 0;
   Selection.cpMax = -1;
   changerCouleurSelection(hRichEdit, Selection, RGB(0, 0, 0)); /* Tout le texte en couleur noir */     
}

//=============================================================================
//         Fonction qui récupère l'emplacement actuel de l'application
//=============================================================================

void GetMyDirectory(char *path, unsigned int taille)
{
   char *c = path+GetModuleFileName(0, path, taille);
   while (*c != '\\')
      c--;
   c++;
   *c = 0;
}
