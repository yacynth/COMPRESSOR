#ifndef LZWTEST_H_INCLUDED
#define LZWTEST_H_INCLUDED
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
        #define LZW_SUCCESS			0		/* succès                          */
        #define LZW_ERREUR_INFILE	-1		/* erreur sur le fichier d'entrée  */
        #define LZW_ERREUR_OUTFILE	-2		/* erreur sur le fichier de sortie */
        #define LZW_ERREUR_NAMES	-3		/* noms de fichiers identiques     */
        #define LZW_ERREUR_MEM		-4		/* erreur de mémoire               */
        #define LZW_ERREUR_READ		-5		/* erreur de lecture               */
        #define LZW_ERREUR_WRITE	-6		/* erreur d'écriture               */
        #define LZW_ERREUR_EOF		-7		/* fin de fichier non trouvé (décompression) */

    /*definition du buffeur*/
    typedef struct tabTBuffer{
        char* pdata;
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


#endif // LZWTEST_H_INCLUDED
