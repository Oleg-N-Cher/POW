/**************************************************************************************************/
/*** Die Datei Obj2Exe.cpp beinhaltet die Implementierung folgender Klassen:																			 ***/
/***			CObj2Exe      																																																																											***/
/**************************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#ifndef __LINKER_H__
#include "Linker.h"
#endif

#ifndef __LINKER_HPP__
#include "Linker.hpp"
#endif

#ifndef __OBJ2EXE_HPP__
#include "Obj2Exe.hpp"
#endif

#ifndef __OBJFILE_HPP__
#include "ObjFile.hpp"
#endif

#ifndef __SECTION_H_HPP
#include "Section.hpp"
#endif

extern char *logFilNam;
extern FILE *logFil; 
extern int		logOn;   

extern void WriteMessageToPow(WORD msgNr, char *str1, char *str2);

extern void FreeCLibFile(CLibFile *aCLibFile);
extern void FreeCObjFile(CObjFile *aCObjFile);
extern void FreeCMapStringToOb(CMapStringToOb *aCMapStringToOb);
extern void FreeCMyMapStringToPtr(CMyMapStringToPtr *aCMyMapStringToPtr);
extern void FreeCMyObList(CMyObList *aCMyObList);
extern void FreeCMyPtrList(CMyPtrList *aCMyPtrList);
extern void FreeCMemFile(CMemFile *aCMemFile);
extern void FreeCMyMemFile(CMyMemFile *aCMyMemFile);
extern void FreeCDllExportEntry(CDllExportEntry *aCDllExportEntry);

extern void TestHeap(void);

extern BOOL	oneLibLst;

IMPLEMENT_DYNAMIC(CObj2Exe, CObject)

/**************************************************************************************************/
/**************************************************************************************************/
/**************************************************************************************************/

/*-------------------*/
/*-- Konstruktoren --*/
/*-------------------*/

CObj2Exe::CObj2Exe()
{
	newExeFil= NULL;
	libLst= NULL;
	objFilLst= NULL;	
	unResSymLst= NULL;
	pubSymLst= NULL; 
	pubLibSymLst= NULL;
	basAdr= 0;
}

/**************************************************************************************************/
/**************************************************************************************************/
/**************************************************************************************************/

/*------------------*/
/*-- Destruktoren --*/
/*------------------*/

CObj2Exe::~CObj2Exe()
{
	FreeUsedMemory();
}

/**************************************************************************************************/
/**************************************************************************************************/
/**************************************************************************************************/

BOOL CObj2Exe::FreeUsedMemory()
{
	CLibFile 				*delLibFil;
	CObjFile 				*delObjFil;

	if (newExeFil)
	{
		newExeFil-> ~CExeFile();
		delete newExeFil;
		newExeFil= NULL;
	}
	if (libLst) 
	{
		while(!libLst-> IsEmpty())
		{
			delLibFil= (CLibFile *)libLst-> RemoveHead();
			FreeCLibFile(delLibFil);
			delete delLibFil;
		}
		FreeCMyObList(libLst);
		delete libLst;
		libLst= NULL;
	}
	if (objFilLst) 
	{
		while(!objLst-> IsEmpty())
		{
			delObjFil= (CObjFile *)objLst-> RemoveHead();
			FreeCObjFile(delObjFil);
			delete delObjFil;
		}
		FreeCMyObList(objLst);
		delete objLst;
		objFilLst= NULL;
	}
 if (srcObjFilLst)
 {
  FreeCMyObList(srcObjFilLst);
  delete srcObjFilLst;
  srcObjFilLst= NULL;
 }
	if (unResSymLst) 
	{
		FreeCMyPtrList(unResSymLst);
		delete unResSymLst;
		unResSymLst= NULL;
	}
	if (pubSymLst) 
	{
		FreeCMyMapStringToPtr(pubSymLst);
		delete pubSymLst;
		pubSymLst= NULL;
	}
	if (pubLibSymLst) 
	{
		FreeCMyMapStringToPtr(pubLibSymLst);
		delete pubLibSymLst;
		pubLibSymLst= NULL;
	}
	return TRUE;
}

/**************************************************************************************************/
/**************************************************************************************************/
/*** Initialisieren des Linkers: Anlegen und Initialisieren der ben�tigten Listen; Anlegen des  ***/
/*** Objekts der zu erstellenden PE-Datei; Laden und Analysieren der zu ber�cksichtigenden      ***/
/*** Bibliotheken und Objektdateien; Zuweisen des Startupsymbols; Intialisieren der Steuerungs- ***/
/*** parameter des Linkers; Verarbeiten zu exportierender Symbole.																														***/  
/**************************************************************************************************/
/**************************************************************************************************/

BOOL CObj2Exe::InitLinker()
{
 mySymbolEntry *expFncSymEnt;
 mySymbolEntry *symBuf;

	CFile										aFil;
	CFileException *pErr= NULL;
	CMyMemFile			  *startUpObjFil;
	CObjFile					  *aObjFil;
 CLibFile					  *aLibFil;

	WORD libFilInd, objFilInd, expFncNamInd;
	BOOL lnkOK;

	char *dllOrExeFilNam;
 char *expFilNam;
 char *dllLibFilNam;
 WORD fulNamLen;

	lnkOK= TRUE;

	/********************************************/
	/*** Initialisieren der ben�tigten Listen ***/
	/********************************************/

	libLst= new CMyObList();
	objLst= new CMyObList(250);                
 srcObjFilLst= new CMyObList();
	unResSymLst= new CMyPtrList(750);
	pubSymLst= new CMyMapStringToPtr(5000);
	pubSymLst-> InitHashTable(500, TRUE);
	pubLibSymLst= new CMyMapStringToPtr(10000);
	pubLibSymLst-> InitHashTable(1000, TRUE);

	/**************************************************************/
	/*** Anlegen und Initialisieren der zu erzeugenden PE-Datei ***/
	/**************************************************************/

	newExeFil= new CExeFile();
	newExeFil-> objFilNam= objFilLst;
 newExeFil-> libFilNam= libFilLst; 
 newExeFil-> exeFilNam= exeFil;
 newExeFil-> resFilNam= resFil;
 newExeFil-> expFncSymLst= expFncLst;
 newExeFil-> subSystem= subSystem;
 newExeFil-> buildExeFile= bldExeFil;
 newExeFil-> buildWinNtFile= bldWinNtFil;
 newExeFil-> includeDebugInfo= incDbgInf;
 newExeFil-> stackSize= stackSize;
	newExeFil-> pubSymLst= pubSymLst;
	newExeFil-> basAdr= basAdr;
 newExeFil-> InitExeFileSec(objLst, srcObjFilLst, pubSymLst);
 
	/*************************************/
	/*** Laden der Librarybibliotheken ***/
	/*************************************/

 libFilInd= 0;

	while(libFilLst[libFilInd] != NULL)
 {
  aLibFil= new CLibFile();
		if (oneLibLst)
		{
			if (!aLibFil-> LoadLibFileFromDiscComPubList(libFilLst[libFilInd], pubLibSymLst))
				lnkOK= FALSE;
		}
		else
		{
			if (!aLibFil-> LoadLibFileFromDiscOwnPubList(libFilLst[libFilInd]))
				lnkOK= FALSE;
		}

		aLibFil-> libFilInd= (WORD )(libFilInd + 1);
		libLst-> AddTail(aLibFil);
		libFilInd++;
	}

	if (!libFilInd)
	{
		WriteMessageToPow(ERR_MSGI_NO_LIB, NULL, NULL);
		return FALSE;
	}

 newExeFil-> libFilNum= libFilInd;

	/***********************************************/
	/*** Laden und Analysieren der Objektdateien ***/
	/***********************************************/

 objFilInd= 0;

 while(objFilLst[objFilInd] != NULL)
 {
	 aObjFil= new CObjFile();
		aObjFil-> SetExeFile(newExeFil);
		aObjFil-> libObjFil= FALSE;
		if (!aObjFil-> LoadObjFileFromDisc(objFilLst[objFilInd], unResSymLst, pubSymLst))
			lnkOK= FALSE;
		else
		{
			newExeFil-> lodObjSecNum++;
			srcObjFilLst-> AddTail(aObjFil);
		}
		objLst-> AddTail(aObjFil);
		objFilInd++;
 }

	if (!lnkOK) return FALSE;
	
	if (!objFilInd)
	{
		WriteMessageToPow(ERR_MSGI_NO_OBJ, NULL, NULL);
		return FALSE;
	}

 newExeFil-> objFilNum= objFilInd;

 /**********************************************************************/
 /****************** Einbinden des CRTStartup Symbols ******************/
	/**********************************************************************/

 if (!pubSymLst-> Lookup(startUpSym, (void *&)symBuf))
 {
			if (oneLibLst)
				startUpObjFil= FndSymInLibsOneList(startUpSym, aLibFil);
			else
				startUpObjFil= FndSymInLibs(startUpSym, aLibFil);
			
			if (startUpObjFil)
			{
				aObjFil= new CObjFile();
				aObjFil-> SetExeFile(newExeFil);
				aObjFil-> objMemFil= startUpObjFil;
				aObjFil-> objMemFil-> SeekToBegin();
				aObjFil-> objFilBuf= (BYTE *)aObjFil-> objMemFil-> ReadWithoutMemcpy();
				aObjFil-> libFilNam= (char *)malloc(strlen(aLibFil-> filNam) + 1);
				strcpy(aObjFil-> libFilNam, aLibFil-> filNam);
				aObjFil->  objFilNam= aLibFil-> lstAccObjFil;
				aObjFil-> libFilInd= aLibFil-> libFilInd;
				aObjFil-> AnalObjFileData(startUpObjFil, unResSymLst, pubSymLst);
				newExeFil-> lodObjSecNum++;
				objLst-> AddTail(aObjFil);			
			}
	}

	/***********************************/
	/*** Zuweisen des Startupsymbols ***/
	/***********************************/

	symBuf = NULL;
	if (pubSymLst-> Lookup(startUpSym, (void *&)symBuf))
		newExeFil-> textSec-> startUpSym= symBuf;
	else
	{
		CString sUpSym("_");
		sUpSym+= startUpSym;

		symBuf = NULL;
		if (pubSymLst-> Lookup(sUpSym, (void *&)symBuf))
			newExeFil-> textSec-> startUpSym= symBuf;
		else
		{
			WriteMessageToPow(ERR_MSGI_NO_STA_SYM, startUpSym, NULL);
			lnkOK= FALSE;
		}
	}

	/***************************************************************************/
	/*** �berpr�fen ob die zu exportierenden Symbole aufgel�st werden k�nnen ***/
	/***************************************************************************/

 if (newExeFil-> edataSec)
 {
  expFncNamInd= 0;
  while(expFncLst[expFncNamInd] != NULL)
  {
   if (pubSymLst-> LookupIncludeString(expFncLst[expFncNamInd], (void *&)expFncSymEnt))
   {
    newExeFil-> edataSec-> AddExpFncEntry(expFncSymEnt);
    expFncSymEnt-> expTabSymNam= expFncLst[expFncNamInd];   
   }
			else
			{
				if (!pubSymLst-> StringIsKeyPart(expFncLst[expFncNamInd], (void *&)expFncSymEnt))
				{
					// Weiterer Versuch mit zus�tzlichen '_' vor dem Symbol
					
					CString nSymNam("_");
     nSymNam+= expFncLst[expFncNamInd];

					BOOL ret = pubSymLst-> StringIsKeyPart(nSymNam.GetBuffer(20), (void *&)expFncSymEnt);
					if (!ret || (expFncSymEnt->symObjFil && expFncSymEnt->symObjFil->libObjFil))
					{
						WriteMessageToPow(ERR_MSGI_NO_EXP_SYM, expFncLst[expFncNamInd], NULL);
						lnkOK= FALSE;
					}
					else
     {
						newExeFil-> edataSec-> AddExpFncEntry(expFncSymEnt);
      expFncSymEnt-> expTabSymNam= expFncLst[expFncNamInd];   
     }
				}
				else
				{
					newExeFil-> edataSec-> AddExpFncEntry(expFncSymEnt);
     expFncSymEnt-> expTabSymNam= expFncLst[expFncNamInd];   
    }
			}
   expFncNamInd++; 
  }                       
 }

	/**************************************************************************/
	/*** �berpr�fen ob eine vorhandene Ressourcendatei ge�ffnet werden kann ***/
	/**************************************************************************/

	if (newExeFil-> resFilNam)
	{
		if (!aFil.Open(newExeFil-> resFilNam, CFile::modeRead | CFile::typeBinary, pErr))
		{
			WriteMessageToPow(ERR_MSGC_OPN_RES, (char *)newExeFil-> resFilNam, NULL);
			return FALSE;
		}
		else
		{
			aFil.Close();
			aFil.~CFile();
		}
	}

	/******************************************************************************************/
	/*** �berpr�fen ob die DLL Library Datei und die DLL-Exportdatei erstellt werden k�nnen ***/
	/******************************************************************************************/

	if (!newExeFil-> buildExeFile)
	{
  fulNamLen= (WORD )strlen(newExeFil-> exeFilNam);
  while(newExeFil-> exeFilNam[--fulNamLen] != '\\');
  dllOrExeFilNam= (char *) &newExeFil-> exeFilNam[fulNamLen + 1];
  expFilNam= (char *) malloc(strlen(newExeFil-> exeFilNam) + 2);
  dllLibFilNam= (char *) malloc(strlen(newExeFil-> exeFilNam) + 2);
  expFilNam= strncpy(expFilNam, newExeFil-> exeFilNam, strlen(newExeFil-> exeFilNam) - 3);
  expFilNam[ strlen(newExeFil-> exeFilNam) - 3]= '\0';
  dllLibFilNam= strcpy(dllLibFilNam, expFilNam);
  expFilNam= strcat(expFilNam, "EXP");
  dllLibFilNam= strcat(dllLibFilNam, "LIB");

		if (!aFil.Open(dllLibFilNam, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, pErr))
			WriteMessageToPow(WRN_MSGC_BLD_IMP_LIB, (char *)dllLibFilNam, NULL);
		else
		{
			aFil.Close();
			aFil.~CFile();
		}
		
		if (!aFil.Open(expFilNam, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, pErr))
			WriteMessageToPow(WRN_MSGC_BLD_EXP_FIL, (char *)expFilNam, NULL);
		else
		{
			aFil.Close();
			aFil.~CFile();
		}
		free(expFilNam);
		free(dllLibFilNam);
	}

	/****************************************************************************************/
	/*** �berpr�fen, ob die Datei mit dem zuk�nfitigen PE-Dateinamen erstellt werden kann ***/
	/****************************************************************************************/

	if (!aFil.Open(newExeFil-> exeFilNam, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, pErr))
	{
	 WriteMessageToPow(ERR_MSGB_OPN_EXE, newExeFil-> exeFilNam, NULL);
		lnkOK= FALSE;
	}
	else
	{
		aFil.Close();
		aFil.~CFile();
	}

	return lnkOK;
}

/**************************************************************************************************/
/**************************************************************************************************/
/*************************************			Aufl�sen der Symoble			***********************************/
/**************************************************************************************************/
/**************************************************************************************************/

BOOL CObj2Exe::ResolveSymbols()
{
	mySymbolEntry 		*curSym, *symBuf;
	CObjFile								*newObjFil;
	CLibFile								*curLibFil;
	CMyMemFile						*newObjFilRawDat;


	BOOL lnkOK;
	char	*symNam;
 int		resSymNum=0;

	lnkOK= TRUE;

	/***********************************************/
	/***   Aufl�sen aller noch offenen Symbole   ***/
	/***********************************************/
  
 while(!unResSymLst-> IsEmpty())
	{
  resSymNum++;
 	curSym= (mySymbolEntry *)unResSymLst-> RemoveHead();
		symNam=  curSym-> symNam;

		if (pubSymLst-> Lookup(symNam, (void *&)symBuf))
		{
			if (symBuf-> dllExpEnt)
			{
				curSym-> dllExpEnt= symBuf-> dllExpEnt;
				curSym-> resSym= symBuf-> resSym;  // Unguter Kunstgriff
			}
			else
				curSym-> resSym= symBuf;			 
		}
		else		// Symbol nicht in der Liste der aufgel�sten Symbole
		{						
			if (oneLibLst)
				newObjFilRawDat= FndSymInLibsOneList(symNam, (CLibFile *&)curLibFil);
			else
				newObjFilRawDat= FndSymInLibs(symNam, (CLibFile *&)curLibFil);
			
			if (newObjFilRawDat)
			{
				newObjFil= new CObjFile();
				newObjFil-> SetExeFile(newExeFil);
    newObjFil-> objMemFil= newObjFilRawDat;
				newObjFil-> objMemFil-> SeekToBegin();
				newObjFil-> objFilBuf= (BYTE *)newObjFil-> objMemFil-> ReadWithoutMemcpy();
    newObjFil-> objFilNam= curLibFil-> lstAccObjFil;

				newObjFil-> libFilNam= (char *)malloc(strlen(curLibFil-> filNam) + 1);
				strcpy(newObjFil-> libFilNam, curLibFil-> filNam);

    newObjFil-> libFilInd= curLibFil-> libFilInd;
				newObjFil-> AnalObjFileData(newObjFilRawDat, unResSymLst, pubSymLst);
    newExeFil-> lodObjSecNum++;
				objLst-> AddTail(newObjFil);				

				if (newObjFil-> incDllFun)
				{
     if (newObjFil-> incExpEnt-> expFunNam)     
     {
      newExeFil-> IncludeDllImport(newObjFil-> incExpEnt);
      curSym-> dllExpEnt= newObjFil-> incExpEnt;
     }
     else
     {
      FreeCDllExportEntry(newObjFil-> incExpEnt); // Dieser Fall tritt nur bei den IMPORT_DESCRIPTOREN auf 
      delete newObjFil-> incExpEnt;
      newObjFil-> incExpEnt= NULL;
     }
                                                 
					curSym-> dllExpEnt= newObjFil-> incExpEnt;

					if (pubSymLst-> Lookup(symNam, (void *&)symBuf)) // Unguter Kunstgriff
					{
      curSym-> resSym= symBuf; 
						pubSymLst-> SetAt(symNam, curSym);  
					}
     else
					{
						WriteMessageToPow(ERR_MSGS_NO_DLL_SYM, symNam, NULL);
						lnkOK= FALSE;
					}
				}                                    
				else
				{
     if (pubSymLst-> Lookup(symNam, (void *&)symBuf))
     {
					 curSym-> resSym= symBuf; 
					 pubSymLst-> SetAt(symNam, symBuf);
     }
     else
     {
						// Symbol wird durch ein zus�ztliches "_" vor dem Symbolnamen aufgel�st
						
      CString newSymNam("_");
      newSymNam+= symNam;
      if (pubSymLst-> Lookup(newSymNam.GetBuffer(20), (void *&)symBuf))
      {
	 				 curSym-> resSym= symBuf; 
		 			 pubSymLst-> SetAt(symNam, symBuf);
      }
      else
      {
							WriteMessageToPow(ERR_MSGS_NO_SYM, symNam, curSym-> symObjFil-> objFilNam);
							lnkOK= FALSE;
						}
     }
				}
			}
			else
			{
    if (curSym-> resSym) // Weak External
    {
     pubSymLst-> SetAt(symNam, symBuf);
    }
    else     
				{
					WriteMessageToPow(ERR_MSGS_NO_SYM, symNam, curSym-> symObjFil-> objFilNam);
					lnkOK= FALSE;
				}
			}				
		} 
	}
	
	return lnkOK;
}

/**************************************************************************************************/
/*** Ansto�en der Methoden zum Zusammensetzen der Sektionsfragmente der .TEXT, .BSS, .RDATA und ***/
/*** .DATA Sektionen.																																																																											***/
/**************************************************************************************************/

BOOL CObj2Exe::ConnectSectionFragments()
{
 return (BOOL )newExeFil-> BuildExeFileRawDataSections();
}

/**************************************************************************************************/
/*** Ansto�en der Methoden zum Aufl�sen der Adressen der .TEXT, .RDATA und .DATA Sektion								***/
/**************************************************************************************************/

BOOL CObj2Exe::ResolveRelocations()
{
	return (BOOL )newExeFil-> ResolveRelocations();
}

/**************************************************************************************************/
/*** Ansto�en der Methoden zum Erstellen von Debuginformationen																																	***/
/**************************************************************************************************/

BOOL CObj2Exe::BuildDebugInformation()
{
	return (BOOL )newExeFil-> BuildDebugInformation();
}

/**************************************************************************************************/
/*** Ansto�en der Methoden zum pyhsischen Erstellen der PE-Datei.																															***/
/**************************************************************************************************/

BOOL CObj2Exe::BuildExeFile()	
{
	if (newExeFil-> BuildExeFileRawData())
		return newExeFil-> WriteExeFileRawDataToFile();
	else
		return FALSE;
}
	
/**************************************************************************************************/
/*** Hilfsmethode zum Debuggen																																																																		***/
/**************************************************************************************************/

void CObj2Exe::WriteSymInSymList()
{
	mySymbolEntry *curSym;
	
	LPCTSTR		nam;
	POSITION	symPos;

	logFil = fopen(logFilNam,"a");
	fprintf(logFil, "\n\nListe aller aufgeloester Symbole:\n");
	fclose(logFil);

	symPos= pubSymLst-> GetStartPosition();

	while(symPos != NULL)
	{
	 pubSymLst-> GetNextAssoc(symPos, nam, (void *&)curSym);
		printf("\n%s", curSym-> symNam);
	}
}

/**************************************************************************************************/
/*** Hilfsmethode zum Debuggen																																																																		***/
/**************************************************************************************************/

void CObj2Exe::WriteUnResSymInSymList()
{
	mySymbolEntry *curSym;

	POSITION	symPos;
		
	logFil = fopen(logFilNam,"a");
	fprintf(logFil, "\n\nListe aller noch offenen Symbole:\n");
	fclose(logFil);

	symPos= unResSymLst-> GetHeadPosition();

	while(symPos)
	{
	 curSym= (mySymbolEntry *)unResSymLst-> GetNext(symPos);
		printf("\n%s", curSym-> symNam);
	} 
}

/**************************************************************************************************/
/*** Hilfsmethode zum Debuggen																																																																		***/
/**************************************************************************************************/

void CObj2Exe::WriteObjSym()
{
	CObjFile *curObjFil;
	POSITION	objPos;
										
	logFil = fopen(logFilNam,"a");
	fprintf(logFil, "Liste aller Symbole der Object-Files:\n");
	fclose(logFil);

	objPos= objLst-> GetHeadPosition();

	while(objPos)
	{
		curObjFil= (CObjFile *)objLst-> GetNext(objPos);
		curObjFil-> WriteObjDataToFile();
		curObjFil-> WriteSymToFile();
	}
}
				
/**************************************************************************************************/
/*** Suchen nach einem Symbol in den Bibliotheksverzeichnissen bei Verwendung von einem Ver-			 ***/
/*** zeichnis pro Bibliothek. R�ckgabe des Objektmoduls, in dem sich das Symbol befindet.							***/
/**************************************************************************************************/

CMyMemFile* CObj2Exe::FndSymInLibs(char *sNam, CLibFile *&curLibFil)
{
	myPublicLibEntry *libEnt;
	CMyMemFile  					*objFilRawDat;
	POSITION 			 				libPos;
	
	libPos= libLst-> GetHeadPosition();
	objFilRawDat= NULL;
			
	while(!objFilRawDat && libPos)
	{
		curLibFil= (CLibFile *)libLst-> GetNext(libPos);
		if (curLibFil-> pubLibSymLst-> Lookup(sNam, (void *&)libEnt))
		{
			objFilRawDat= curLibFil-> ReadLibObjFile(libEnt-> achMemOff);
			if ((DWORD) objFilRawDat != 0xFFFFFFFF)
				return objFilRawDat;
			else
				return NULL;
		}
	}

 // Symbol konnte nicht gefunden werden, m�glicherweise Symbolname nicht richtig.
	// Kommt bei Microsoft manchmal vor.	
	// Neuer Suchversuch mit anderer Anzahl der '_' vor dem Symbolnamen.
	
	CString newSrhNam("_");

	newSrhNam+= sNam;

	libPos= libLst-> GetHeadPosition();
		
	while(!objFilRawDat && libPos)
	{
		curLibFil= (CLibFile *)libLst-> GetNext(libPos);
		if (curLibFil-> pubLibSymLst-> Lookup(newSrhNam.GetBuffer(20), (void *&)libEnt))
		{
			objFilRawDat= curLibFil-> ReadLibObjFile(libEnt-> achMemOff);
			if ((DWORD) objFilRawDat != 0xFFFFFFFF)
			{
				WriteMessageToPow(WRN_MSGS_NO_SYM, sNam, newSrhNam.GetBuffer(20));
				return objFilRawDat;
			}
			else
				return NULL;
		}
	}
	
	return NULL;
}

/**************************************************************************************************/
/*** Suche nach einem Symbol im Gesamtbibliotheksverzeichnis, beim Verwenden eines Verzeichnis- ***/
/*** ses f�r alle Bibliotheken.	R�ckgabe des Objektmoduls, in dem sich das Symbol befindet.					***/
/**************************************************************************************************/

CMyMemFile* CObj2Exe::FndSymInLibsOneList(char *sNam, CLibFile *&curLibFil)
{
	myPublicLibEntry *libEnt;
	CMyMemFile 	 				*objFilRawDat;
	
	if (pubLibSymLst-> Lookup(sNam, (void *&)libEnt))
	{
		curLibFil= libEnt-> myLibFil;
		objFilRawDat= libEnt-> myLibFil-> ReadLibObjFile(libEnt-> achMemOff);
		if ((DWORD) objFilRawDat != 0xFFFFFFFF)
			return objFilRawDat;
		else
			return NULL;
	}

	// Symbol konnte nicht gefunden werden, m�glicherweise Symbolname nicht richtig.
	// Kommt bei Microsoft manchmal vor.	
	// Neuer Suchversuch mit anderer Anzahl der '_' vor dem Symbolnamen.
	
	CString newSrhNam("_");

	newSrhNam+= sNam;

	if (pubLibSymLst-> Lookup(newSrhNam.GetBuffer(256), (void *&)libEnt))
	{
		curLibFil= libEnt-> myLibFil;
		objFilRawDat= libEnt-> myLibFil-> ReadLibObjFile(libEnt-> achMemOff);
		if ((DWORD) objFilRawDat != 0xFFFFFFFF)
		{
			WriteMessageToPow(WRN_MSGS_NO_SYM, sNam, newSrhNam.GetBuffer(20));
			return objFilRawDat;
		}
		else
			return NULL;
	}
	
	return NULL;
}

/**************************************************************************************************/
/*** Freigeben des von den Bibliotheken belegten Speichers																																						***/
/**************************************************************************************************/
				
BOOL CObj2Exe::FreeLibFiles()
{
 CLibFile *curLibFil;
 POSITION libPos;
 	
	libPos= libLst-> GetHeadPosition();
	while(libPos)
	{
 	curLibFil= (CLibFile *)libLst-> GetNext(libPos);
		FreeCLibFile(curLibFil);
		delete curLibFil;
	}
	libLst-> ~CMyObList();
	delete libLst;
	libLst= NULL;	                     				

	return TRUE;
}

