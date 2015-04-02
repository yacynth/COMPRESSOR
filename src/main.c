#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*definition des constantes de l algorithme*/

    /*constant speciaux*/

     #define LZW_EOF 256
     #define LZW_BIT_PLUS 257
     #define LZW_NEW_DICO 258
     #define LZW_BEGIN_DICO 259

    /*parametre de l algorithme*/
      #define MIN_BIT  9
      #define MAX_BIT 12
      #define MAX_DICO_LENGTH 4096
      #define MAX_BUFFER_LENGTH 4096


    /*taille de buffeur pour la lecture et ecriture*/
      #define FILE_BUFFER_LENGTH  32768

    /*mode de fonctionnement*/
      #define COMPRESSER   1
      #define DECOMPRESSER 2

    /*definition de true*/
      #ifndef TRUE
      #define TRUE 1
      #endif

    /*definition de false*/
      #ifndef FALSE
      #define FALSE 0
      #endif

      /*============================================================================*\
        |* Codes de retour des fonctions.                                             *|
      \*============================================================================*/
        #define LZW_SUCCESS			0		/* succ�s                          */
        #define LZW_ERREUR_INFILE	-1		/* erreur sur le fichier d'entr�e  */
        #define LZW_ERREUR_OUTFILE	-2		/* erreur sur le fichier de sortie */
        #define LZW_ERREUR_NAMES	-3		/* noms de fichiers identiques     */
        #define LZW_ERREUR_MEM		-4		/* erreur de m�moire               */
        #define LZW_ERREUR_READ		-5		/* erreur de lecture               */
        #define LZW_ERREUR_WRITE	-6		/* erreur d'�criture               */
        #define LZW_ERREUR_EOF		-7		/* fin de fichier non trouv� (d�compression) */

    /*definition du buffeur*/
    typedef struct tabTBuffer{
        char pdata[MAX_DICO_LENGTH];
        short size;      /*taille du tableau*/
        short nbdata;    /*nombre d elelent dans le tableau*/
    }TBuffer;
    /*===================================================================*/
       /*declairation des fonctions globales*/

    /*===================================================================*/
     int LZW(char*fichier_entree ,char*fichier_sortie);
     int UNLZW(char* fichier_entree ,char* fichier_sortie);
     int ftaille_fichier_lire();
     int ftaille_fichier_traite();
     int ftaille_fichier_entree();
     int ftaille_fichier_sortie();


 TBuffer dico[MAX_DICO_LENGTH];                             /*dictionnaire :tableau de taillle MAX_DICO_LENGTH*/
 short    dicoindex;                                        /*indice de l element dans le dico*/
 short tabindex[MAX_BUFFER_LENGTH][MAX_DICO_LENGTH];        /*tableau des index pour chaque taille,c est un tableau de taille
                                                                MAX_BUFFER_LENGHT * MAX_DICO_LENGHT                         */
 short nbtabindex[MAX_BUFFER_LENGTH];                                         /*nombre de valeur dans chaque sous_tableau,tableau de taille MAX_BUFFER_LENGHT*/


 TBuffer buffer_latent;         /*buffer latent*/
 TBuffer buffer_courant;        /*bufer courant*/
 char    bitbuffer;            /*buffer pour lecture et ecriture par bit*/
 char    bitbuffersize;        /*nombre de bit dans le buffer*/
 TBuffer buffer_ecriture;       /*buffer pour ecriture */
 FILE*   fichier_ou_ecrire  ;     /*fichier ou ecrire */

 TBuffer  buffer_lecture;            /*buffer pour la lecture*/
 short    buffer_lecture_index;      /*position action dans le fichier*/
 FILE*    fichier_ou_lire;           /*fichier ou lire*/

 int   mode          ;                /*mode en cour              */
 long  travail_fait      ;            /*le travail deja fait      */
 long  travail_total    ;              /*le travail total a faire  */
 long  taille_fichier_entree;             /*taille du fichier d entree */
 long  taille_fichier_sortie;             /*taille du  fichier de sortie*/


/*=======================================================================================*/
/*                               prototype des fonctions de module                       */
/*========================================================================================*/
 int	Init			(char* fichier_entree, char* fichier_sortie);
 void libere			();

 int	fait_LZW		();
 int	fait_UNLZW		();

 int	initdico		        ();
 void	reinitialisedico		();
 void	supprimedico		    ();
 int	cherchedansdico		(TBuffer* buffer_r);
 int	ecrire_dans_dico	(TBuffer* buffer_r);

 int	ecrire_bit		(long* pdata, char size);
 int	FlushBits		();
 int	lire_bit		(long* pdata, char size);
 int	ecrire_fichier		(char* pdata);
 int	lire_fichier		(char* pdata);

/*============================================================================
   lzw: compresses le fichier
   entree : fichier_entree :nom du fichier d entree
           :fichier_sortie :nom du ffichier de sortie
   sortie:LZW_SUCCESS 0 si la compression est bien deroulee
/*============================================================================*/

int LZW(char* fichier_entree, char* fichier_sortie)
{
	//variables locales
	int result;

	// initialisation de la compression
	mode = COMPRESSER;
	result = Init(fichier_entree, fichier_sortie);
	if(result != LZW_SUCCESS)
	{
		libere();
		return result;
	}

	// taille du fichier � lire, nombre d'octets d�j� lus
	fseek(fichier_ou_lire, 0, SEEK_END);
	taille_fichier_entree = ftell(fichier_ou_lire);
	travail_total = taille_fichier_entree;
	fseek(fichier_ou_lire, 0, SEEK_SET);
	travail_fait = 0;

	// on effectue le travail
	result = fait_LZW();
	if(result == LZW_SUCCESS)
		taille_fichier_sortie = ftell(fichier_ou_ecrire);
	libere();
	return result;
}


/*=============================================================================\
|* UnLZW : d�compression d'un fichier.                                        *|
|* entr�e : lpszInFile  : nom du fichier d'entr�e.                            *|
|*          lpszOutFile : nom du fichier de sortie.                           *|
|* retour : LZW_SUCCESS (0) si la d�compression a r�ussi, un code d'erreur    *|
|*          (<0) sinon.                                                       *|
\==============================================================================*/
int UNLZW(char* fichier_entree, char* fichier_sortie)
{
	//variables locales
	int result;

	/* initialisation de la d�compression */
	mode= DECOMPRESSER;
	result = Init(fichier_entree,fichier_sortie);
	if(result != LZW_SUCCESS)
	{
		libere();
		return result;
	}

	// taille du fichier � lire, nombre d'octets d�j� lus
	fseek(fichier_ou_lire, 0, SEEK_END);
	travail_total= ftell(fichier_ou_lire);
	fseek(fichier_ou_lire, 0, SEEK_SET);
	travail_fait = 0;

	// on effectue le travail
	result = fait_UNLZW();
	libere();
	return result;
}
/*=================================================================================*/
/* taille_fichier_entree:retourne la taille du fichier  a lire                     */

/*=================================================================================*/
int ftaille_fichier_lire()
{

    return travail_total;
}
/*=================================================================================*/
/*taille_fichier_traite:retourne la taille du fichier deja traite en octet         */
/*=================================================================================*/
int ftaille_fichier_traite()
{
    return travail_fait;
}

/*===========================================================================================/
|* taillle_fichier_entree(): retourne la taille du fichier d entree                         /
|*==========================================================================================*/
int ftaille_fichier_entree()
{
    return taille_fichier_entree;

}

/*============================================================================================*\
|* taille_fichier_sortie():retourne la taille du fichier de sortie                            /
/*===========================================================================================*/
 int ftaille_fichier_sortie()
 {
     return taille_fichier_sortie;
 }


/******************************************************************************\
|* fait_LZW : effectue le travail de compression apr�s que l'initialisation soit *|
|*         faite.                                                             *|
|* retour : LZW_SUCCESS (0) si la compression a r�ussi, un code d'erreur (<0) *|
|*          sinon.                                                            *|
\******************************************************************************/
int fait_LZW()
{
	// variables locales
	char bit_courant, nbbit;
	short nbdata, rest;
	long data;
	int index;

	// nombre de bits utilis�s pour le codage (MIN_BITS � MAX_BITS)
	nbbit = MIN_BIT;

	// lecture premier octet du fichier, on le met dans le buffer latent
	if(!lire_fichier(&bit_courant))
		return LZW_ERREUR_READ;
	buffer_latent.pdata[0]	= bit_courant;
	buffer_latent.nbdata	= 1;


	// boucle de lecture du fichier
	while(1)
	{
		//r�cup�ration prochain octet
		if(!lire_fichier(&bit_courant))
			break;

		// ajout de l'octet lu et du buffer latent dans le buffer courant
		nbdata = buffer_latent.nbdata;
		memcpy(buffer_courant.pdata, buffer_latent.pdata, nbdata);
		buffer_courant.pdata[nbdata]	= bit_courant;
		buffer_courant.nbdata			= nbdata + 1;

		// recherche de la chaine dans le dictionnaire
		if(cherchedansdico(&buffer_courant) >= 0)
		{
			//latent = buffer  quant la chaine existe deja d    ns le dictionnaire
			memcpy(buffer_latent.pdata, buffer_courant.pdata, buffer_courant.nbdata);
			buffer_latent.nbdata = buffer_courant.nbdata;
		}
		else
		{
			// �crire la cha�ne dans le dictionnaire
			if(!ecrire_dans_dico(&buffer_courant))
				return LZW_ERREUR_MEM;

			// rechercher la valeur du buffer latent dans le dictionnaire
			index = cherchedansdico(&buffer_latent);

			// si latent n'est pas dans le dico, �crire latent (octet simple)
			if(index < 0)
				index = buffer_latent.pdata[0];
			else
			{
				// v�rifier si le nombre de bits est suffisant
				rest = index >> nbbit;
				while(rest)
				{
					// Ecrire LZW_BIT_PLUS sur le fichier destination
					data = LZW_BIT_PLUS;
					if(!ecrire_bit(&data, nbbit))
						return LZW_ERREUR_WRITE;
					nbbit++;
					rest >>= 1;
				}
			}

			// �citure des bits, latent = octet lu
			if(!ecrire_bit(&index, nbbit))
				return LZW_ERREUR_WRITE;
			buffer_latent.pdata[0]	= bit_courant;
			buffer_latent.nbdata	= 1;

			// si on arrive au bout du dico
			if(dicoindex == MAX_DICO_LENGTH)
			{
				// Ecrire LZW_NEW_DIC sur le fichier destination
				data = LZW_NEW_DICO;
				if(!ecrire_bit(&data, nbbit))
					return LZW_ERREUR_WRITE;

				// on r�initialise le dictionnaire
				reinitialisedico();

				// on r��crit le buffer dans le dico et on r�initialise le nombre de bits
				if(!ecrire_dans_dico(&buffer_courant))
					return LZW_ERREUR_MEM;
				nbbit = MIN_BIT;
			}
		}
	}

	// on �crit le buffer latent, s'il n'est pas dans le dico, �crire un octet simple
	index = cherchedansdico(&buffer_latent);
	if(index < 0)
		index = buffer_latent.pdata[0];
	else
	{
		// v�rifier si le nombre de bits est suffisant
		rest = index >> nbbit;
		while(rest)
		{
			// Ecrire LZW_BIT_PLUS sur le fichier destination
			data = LZW_BIT_PLUS;
			if(ecrire_bit(&data, nbbit))
				return LZW_ERREUR_WRITE;
			nbbit++;
			rest >>= 1;
		}
	}

	// �criture des bits, ajout fin de fichier, on termine le dernier octet �ventuel
	data = LZW_EOF;
	if(!ecrire_bit(&index, nbbit) || !ecrire_bit(&data, nbbit))
		return LZW_ERREUR_WRITE;
	if(!FlushBits())
		return LZW_ERREUR_WRITE;

	// lecture et �criture termin�e
	if(!ecrire_fichier(NULL))
		return LZW_ERREUR_WRITE;
	if(!lire_fichier(NULL))
		return LZW_ERREUR_READ;

	// compression r�ussie
	return LZW_SUCCESS;
}


/******************************************************************************\
|* fait_UNLZWZ : effectue le travail de d�compression apr�s que l'initialisation  *|
|*           soit faite.                                                      *|
|* retour : LZW_SUCCESS (0) si la d�compression a r�ussi, un code d'erreur    *|
|*          (<0) sinon.                                                       *|
\******************************************************************************/
int fait_UNLZW()
{
	// variables locales
	short nbdata;
	long currentdata;
	char nbbit = MIN_BIT;

	// lecture premi�re donn�e du fichier (c'est obligatoirement un caract�re normal), on le met dans le buffer latent
	if(!lire_bit(&currentdata, nbbit))
		return LZW_ERREUR_READ;
	buffer_latent.pdata[0]	= (char) currentdata;
	buffer_latent.nbdata	= 1;

	// boucle de traitement
	while(1)
	{
		//lecture donn�e
		if(!lire_bit(&currentdata, nbbit))
			break;

		//si fin de fichier
		if(currentdata == LZW_EOF)
			break;
		// si nouveau dico
		else if(currentdata == LZW_NEW_DICO)
		{
			reinitialisedico();
			nbbit = MIN_BIT;
			continue;
		}
		// si augmentation nombre de bits
		else if(currentdata == LZW_BIT_PLUS)
		{
			nbbit++;
			continue;
		}

		// buffer = latent
		nbdata = buffer_latent.nbdata;
		memcpy(buffer_courant.pdata, buffer_latent.pdata, nbdata);

		   // si donn�e pas dans le dico alors, le caract�re � ajouter au buffer
		   //est la latence. Se produit quand succession du m�me caract�re
		if(currentdata == (long) dicoindex)
			buffer_courant.pdata[nbdata] = buffer_latent.pdata[0];
		else if(currentdata < (long) dicoindex)
			buffer_courant.pdata[nbdata] = dico[currentdata].pdata[0];
		else
			break;
		buffer_courant.nbdata = nbdata + 1;

		// ajout de ce mot dans le dico
		if(!ecrire_dans_dico(&buffer_courant))
			return LZW_ERREUR_MEM;

		// on �crit les caract�res latents dans le fichier de sortie
		if(fwrite(buffer_latent.pdata, 1, nbdata, fichier_ou_ecrire) != nbdata)
			return LZW_ERREUR_WRITE;

		// nouveu buffer latent
		memcpy(buffer_latent.pdata, dico[currentdata].pdata,dico[currentdata].nbdata);
		buffer_latent.nbdata = dico[currentdata].nbdata;
	}

	// si on n'a pas lu la fin du fichier
	if(currentdata != LZW_EOF)
		return LZW_ERREUR_EOF;

	// on �crit les caract�res latents dans le fichier de sortie
	if(fwrite(buffer_latent.pdata, 1, buffer_latent.nbdata, fichier_ou_ecrire) != buffer_latent.nbdata)
		return LZW_ERREUR_WRITE;

	// lecture et �criture termin�e
	if(!ecrire_fichier(NULL))
		return LZW_ERREUR_WRITE;
	if(!lire_fichier(NULL))
		return LZW_ERREUR_READ;

	// d�compression r�ussie
	return LZW_SUCCESS;
}


/*======================================================================================================/
       Init(): initialise la compression ou la decompression
          entree:fichier_entree
          sortie:fichier_sortie
       return:LZW_SUCCESS 0 si l initialisation s est bien passee
/*====================================================================================================*/
       Init(char*fichier_entree,char*fichier_sortie)
           {
             //fichier d entree et de sortie
                fichier_ou_ecrire = NULL;
                fichier_ou_lire   = NULL;

            //les variables du dictionnaire
              dico = NULL;
              dicoindex =0;
              tabindex = NULL;
              nbtabindex =NULL;

            //buffers latent et courant
               buffer_latent.pdata = NULL;
               buffer_courant.pdata =NULL;

            //buffers pour la lecture et ecriture
               buffer_ecriture.pdata = NULL;
               buffer_lecture.pdata  =NULL;
               bitbuffer      =0;
               bitbuffersize  =0;

             //ouverture des fichiers
             fichier_ou_ecrire = fopen(fichier_entree,"rb") ;
             fichier_ou_lire    =fopen(fichier_sortie,"wb");

             //creation buffer pour la lecture
             buffer_lecture.pdata = malloc(FILE_BUFFER_LENGTH);
               buffer_lecture.size = FILE_BUFFER_LENGTH;
               buffer_lecture.nbdata=0;
               buffer_lecture_index =0;

            // creation du buffer d ecriture
              buffer_ecriture.pdata = malloc(FILE_BUFFER_LENGTH);
              buffer_ecriture.size  =FILE_BUFFER_LENGTH;
              buffer_ecriture.nbdata=0;

            // creation du dictionnaire
                if( !initdico())
                   return LZW_ERREUR_MEM;

            //creation du buffer latent et courant
               buffer_latent.pdata =malloc(FILE_BUFFER_LENGTH);
               buffer_latent.size  =FILE_BUFFER_LENGTH;
               buffer_latent.nbdata =0;
               buffer_courant.pdata =malloc(FILE_BUFFER_LENGTH);
               buffer_courant.size  = FILE_BUFFER_LENGTH;
               buffer_courant.nbdata=0;


                return LZW_SUCCESS;
           }

    /*=====================================================================================================\
       libere(): libere les memoires allou�e pendant la compresion et la decompression
    \=====================================================================================================*/
     void libere()
     {
       //fermeture des fichiers
        fclose(fichier_ou_ecrire);
        fclose(fichier_ou_lire);
        fichier_ou_ecrire =NULL;
        fichier_ou_lire   =NULL;

        //destruction du dictionnaire
         supprimedico();

        //destruction des buffers courants et lalents

         free(buffer_courant.pdata);
         free(buffer_latent.pdata);
         buffer_courant.pdata = NULL;
         buffer_latent.pdata  =NULL;

         //destruction des buffers de lecture et d ecriture
         free(buffer_ecriture.pdata);
         free(buffer_lecture.pdata);
          buffer_ecriture.pdata =NULL;
          buffer_lecture.pdata  =NULL;
     }

     /*==========================================================================================\
       initdico(): initialise le dictionnaire
        retour   : vrai  si l initialisation est bien fait
     \*=========================================================================================*/
     int initdico()
     {
         //variable locales
          int i;

         //creation du dictionnaire
         dico =malloc(MAX_DICO_LENGTH*sizeof(TBuffer));
         memset(dico,0,MAX_DICO_LENGTH*sizeof(TBuffer));
         //initialise les 256 premiers caracteres
          for(i=0;i<256;i++)
          {
            dico[i].nbdata=malloc(1);
            dico[i].pdata[1] =i;
            dico[i].nbdata=1;
            dico[i].size  =1;
          }

          //indice en cour dans le dictionnaire
          dicoindex=LZW_BEGIN_DICO;

          //tableau des index ,nombre d element dans les sous_tableau
          tabindex =malloc(MAX_BUFFER_LENGTH*sizeof(short*));
          nbtabindex=malloc(MAX_BUFFER_LENGTH*sizeof(short));

          memset(tabindex ,0,MAX_BUFFER_LENGTH*sizeof(short*));
          memset(nbtabindex,0,MAX_BUFFER_LENGTH*sizeof(short));
          return TRUE;
     }

     /*============================================================================================\
      reinitialisedico(): reinitialise le dico .les buffers du dico ne sont pas detruits dans
      \                   cette fonctio nmais seulement lorsqu il sont ecrasee.les sous -tableau
                          sont initialise
      \===========================================================================================*/
 void reinitialisedico()
{
	// variables locales
	int i;

	// la reinitialisation du dico en mode DECOMPRESSER revient � remettre l'indice courant
    // dans le dictionnaire � sa valeur min
	dicoindex = LZW_BEGIN_DICO;
	if(mode==DECOMPRESSER)
		return;

	// initialisation des tableau des index
	for(i = 0; i < MAX_BUFFER_LENGTH; i++)
	{
		free(tabindex[i]);
		tabindex[i]	= NULL;
		nbtabindex[i]	= 0;
	}
}
/*=====================================================================================================\
    supprimmerdico():  supprimer le dictionnaire

 \=====================================================================================================*/
void supprimedico()
{
    //variables locales
	  int i;

	// suppression des buffers du dictionnaire
	if(dico != NULL)
	{
		for(i = 0; i < MAX_DICO_LENGTH; i++)
			free(dico[i].pdata);
	}

	// supression du dictionnaire, en mode decompression, on s'arr�te l�
	free(dico);
	dico = NULL;
	if(mode == DECOMPRESSER)
		return;

	// destruction sous-tableaux des index
	if(tabindex != NULL)
	{
		for(i = 0; i < MAX_BUFFER_LENGTH; i++)
			free(tabindex[i]);
	}

	// destruction tableaux des index
	free(tabindex);
	free(nbtabindex);
    tabindex    = NULL;
	nbtabindex	= NULL;

}

/*========================================================================================================\
 |*      cherchedansdico():  recherche si un buffer est pressent dans le dictionnaire
 |*      entree           :   buffer_r  c est le buffer a rechercher dans le dico                         |
 |*      sortie           :   retourne la position de buffer dans le dico et -1 si elle n exite pas       *|
 \*======================================================================================================*/
  int cherchedansdico(TBuffer *buffer_r)
   {
      // variables locales
	short nbdata, nbbuffer;
	int i, index;
	TBuffer* dicobuffer;

	// nombre de donn�es dans le buffer et nombre de buffers recens�s pour cette taille
	nbdata		= buffer_r->nbdata;
	nbbuffer	= nbtabindex[nbdata];

	// si aucun buffer de cette taille recens�  on renvoie -1
	if(nbbuffer == 0)
		return -1;

	// on parcourt les indices du tableau en fonction de la taille du buffer

	for(i = 0; i < nbbuffer; i++)
	{
		// r�cup�ration buffer du dico � comparer
		index = tabindex[nbdata][i];
		dicobuffer = &dico[index];

		// comparaison des buffers
		if(memcmp(buffer_r->pdata, dicobuffer->pdata, nbdata) == 0)
			return index;
	}

	/* buffer non pr�sent dans le dico */
	return -1;
   }


/*================================================================================\
|* ecrire_dans_dico : �crit un buffer � la fin du dictionnaire.                  *|
|* entr�e :    buffer_r : le buffer � �crire                                     *|
|* retour :    vrai si l'ajout a r�ussi, faux sinon.                             *|
\=================================================================================*/
int ecrire_dans_dico(TBuffer* buffer_r)
{
	//variables locales
	  short nbdata;

	//nombre de donn�es dans le buffer
	 nbdata = buffer_r->nbdata;

	//on d�truit le buffer d�j� pr�sent dans le tableau
	free(dico[dicoindex].pdata);

	//cr�ation nouveau buffer
	dico[dicoindex].pdata = malloc(nbdata);
	if(dico[dicoindex].pdata == NULL)
		return FALSE;
	dico[dicoindex].size		= nbdata;
	dico[dicoindex].nbdata	    = nbdata;

	//recopie
	memcpy(dico[dicoindex].pdata, buffer_r->pdata, nbdata);

	//ajout de l'index dans le tableau des index en fonction de la taille du buffer seulement en mode COMPRESSER
	if(mode == COMPRESSER)
	{
		// si c'est le premier buffer de cette taille, cr�er un nouveau sous-tableau
		if(tabindex[nbdata] == NULL)
			tabindex[nbdata] = malloc(MAX_DICO_LENGTH * sizeof(short));
		if(tabindex[nbdata] == NULL)
			return FALSE;

		// on m�morise l'index du dictionnaire
		tabindex[nbdata][nbtabindex[nbdata]] = dicoindex;
		tabindex[nbdata]++;
	}

	// un �l�ment de plus dans le dico
	dicoindex++;
	return TRUE;
}

/*=============================================================================*\
|* ecrire_bit : �crit un nombre cod� sur un certain nombre de bits.            *|
|* entr�e : pdata   les donn�es � �crire.                                    *|
|*          nbbit  le nombre de bit � prendre en compte.                   *|
|* retour : vrai si l'�criture a r�ussi, faux sinon.                          *|
|*---------------------------------------------------------------------------*|
|* Remarque : les bits inutiles doivent �tre nuls.                            *|
\==============================================================================*/
int ecrire_bit(long* pdata, char nbbit)
{
	// variables locales
	char byte;
	int dec;


	// d�calage � effectuer
	dec = nbbit - (8 - bitbuffersize);
	if(dec >= 0)
	{
		// on rempli le buffer en entier, �criture dans le fichier
		bitbuffer |= (char)(*pdata >> dec);

		if(!ecrire_fichier(&bitbuffer))
			return FALSE;
	}
	else
	{
		// on ne fait que rajouter des bits au buffer
		bitbuffer		|= (char)(*pdata << (-dec));
		bitbuffersize	+= nbbit;
		return TRUE;
	}

	//donn�es restantes
	nbbit -= 8 - bitbuffersize;


	// tant que des octets entiers restent � �crire
	while(nbbit >= 8)
	{
		// calcul de l'octet � �crire
		byte = (char)((*pdata >> (nbbit - 8)) & 0xFF);
		if(!ecrire_fichier(&byte))
			return FALSE;

		// 8 bits de moins � �crire;
		nbbit -= 8;
	}

	// le reste des donn�es est mis dans le buffer
	bitbuffer		= (char)((*pdata << (8 - nbbit)) & 0xFF);
	bitbuffersize	= nbbit;
	return TRUE;
}


/*=============================================================================\
|* FlushBits : �crit les bits restants dans le buffer.                        *|
|* retour : vrai si l'op�ration a r�ussi, faux sinon.                         *|
\==============================================================================*/
int FlushBits()
{
	// variables locales
	int result;

	// si le buffer est vide, on renvoie vrai
	if(bitbuffersize== 0)
		return TRUE;

	//on �crit le buffer
	result			= ecrire_fichier(&bitbuffer);
	bitbuffer       = 0;
	bitbuffersize   = 0;
	return result;
}


/*=============================================================================\
|* lire_bit() : lit un nombre cod� sur un certain nombre de bits depuis un fichier.                                                        *|
|* entr�e : pdata   variable qui va recevoir le nombre lu.                   *|
|*          nbbit   nombre de bits � lire.                                   *|
|* retour : vrai si la lecture a r�ussi, faux sinon.                          *|
\******************************************************************************/
int lire_bit(long* pdata, char nbbit)
{
	//variables locales
	char byte;


	// init des donn�es, r�cup�ration des donn�es situ�s dans le buffer
	*pdata = 0;
	if(nbbit >= 8)
	{
		*pdata = bitbuffer >> (8 - bitbuffersize);
	}
	else
	{
		//on ne r�cup�re qu'une partie
		*pdata			= bitbuffer >> (8 - nbbit);
		bitbuffer		<<= nbbit;
		bitbuffersize	-= nbbit;
		return TRUE;
	}

	// donn�es restantes � lire, r�init du buffer
	nbbit			-= bitbuffersize;
	bitbuffer		= 0;
	bitbuffersize	= 0;


	//tant qu'il reste des octets entier � lire
	while(nbbit >= 8)
	{
		if(!lire_fichier(&byte))
			return FALSE;

		// ajout dans les donn�es, 8 bits de moins � lire
		*pdata = (*pdata << 8) | byte;
		nbbit -= 8;
	}

	// s'il ne reste plus rien � lire
	if(nbbit == 0)
		return TRUE;

	// lecture d'un octet suppl�mentaire
	if(!ecrire_fichier(&byte))
		return FALSE;

	//on rajoute ce qu'il faut aux donn�es, on met le reste dans le buffer
	*pdata			= (*pdata << nbbit) | (byte >> (8 - nbbit));
	bitbuffer		= byte << nbbit;
	bitbuffersize	= 8 - nbbit;
	return TRUE;
}


/*==================================================================================\
|* ecrire_fichier: �crit un octet dans le buffer d'�criture du fichier.            *|
|* entr�e : pdata : donn�e � �crire ou NULL si on purge le buffer.                 *|
|* retour : vrai si l'�criture a r�ussi, faux sinon.                               *|
\*=================================================================================*/
int ecrire_fichier(char* pdata)
{
	//variables locales
	long NbBytesWritten;

	// si le buffer d'�criture est plein ou si on le purge
	if(buffer_ecriture.nbdata == buffer_ecriture.size || pdata == NULL)
	{
		NbBytesWritten = fwrite(buffer_ecriture.pdata, 1,buffer_ecriture.nbdata, fichier_ou_ecrire);
		if(NbBytesWritten != buffer_ecriture.nbdata)
			return FALSE;
		buffer_ecriture.nbdata = 0;
	}

	//s'il s'agisait d'une purge, d�truire les buffers
	if(pdata == NULL)
	{
		free(buffer_ecriture.nbdata);
		buffer_ecriture.pdata	= NULL;
		buffer_ecriture.size	= 0;
		buffer_ecriture.nbdata	= 0;
		return TRUE;
	}

	// ajout de la donn�e dans le buffer d'�criture
	buffer_ecriture.pdata[buffer_ecriture.nbdata] = *pdata;
	buffer_ecriture.nbdata++;
	return TRUE;
}


/******************************************************************************\
|* lire_fichier: lit un octet depuis le buffer de lecture du fichier.             *|
|* entr�e : pData : donn�e � �crire ou NULL si on purge le buffer.            *|
|* retour : vrai si la lecture a r�ussi, faux sinon.                          *|
\******************************************************************************/
int lire_fichier(char* pdata)
{
	// variables locales
	long NbBytesRead;

	// s'il s'agit d'une purge, d�truire les buffers
	if(pdata == NULL)
	{
		free(buffer_lecture.pdata);
		buffer_lecture.pdata	= NULL;
		buffer_lecture.size		= 0;
		buffer_lecture.nbdata	= 0;
		buffer_lecture_index		= 0;
		return TRUE;
	}

	// si le buffer de lecture est termin�
	if(buffer_lecture_index == buffer_lecture.nbdata)
	{
		NbBytesRead = fread(buffer_lecture.pdata, 1,buffer_lecture.size, fichier_ou_lire);
		if(NbBytesRead == 0)
			return FALSE;
		buffer_lecture.nbdata	= (long) NbBytesRead;
		buffer_lecture_index	= 0;
	}

	// r�cup�ration de la donn�e
	*pdata = buffer_lecture.pdata[buffer_lecture_index];
	buffer_lecture_index++;
	travail_fait++;
	return TRUE;
}


int main()
{
    printf("BIENVENUE DANS NOTRE LOGICIEL DE COMPRESSION\n");
    char*entree="text.txt";
    char*sortie="text1.txt";
   LZW(entree,sortie);

    return 0;
}
