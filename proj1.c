/*
 * Soubor:  proj1.c
 * Datum:   24.10.2011
 * Autor:   Radim Kubis, xkubis03@stud.fit.vutbr.cz
 * Projekt: Prevod casovych udaju, projekt c. 1 pro predmet IZP
 * Popis:   Program prevadi ciselnou hodnotu na lidsky citelny casovy udaj.
 *  
 *          Na standardnim vstupu ocekava nulu nebo prirozene cislo, ktere
 *          vyjadruje pocet sekund, a na standardni vystup vypise tutez hodnotu,
 *          ale v lidsky citelne podobe, jako pocet tydnu, dnu, hodin, minut
 *          a sekund, pritom nulove udaje pri vystupu vynecha, krome pripadu,
 *          kdy je vstupni cislo nulove.
 *           
 *          Pro modifikaci vystupu lze pouzit na prikazovem radku
 *          jeden z parametru (nelze kombinovat):
 *          -t  - nejvyssi vypisovany udaj je tyden (stejna funkcnost pri
 *                spusteni bez parametru)
 *          -d  - nejvyssi vypisovany udaj je den
 *          -h  - mejvyssi vypisovany udaj je hodina
 *          -m  - nejvyssi vypisovany udaj je minuta
 *          -s  - nejvyssi vypisovany udaj je sekunda
 *          
 *          Programu lze zadat na prikazove radce i parametr rozsireni --extra
 *          pro opacny prevod lidsky citelneho vstupu na vystupni pocet sekund
 *          v ciselne podobe. Tento parametr nelze kombinovat s zadnym jinym
 *          parametrem. Pri prevodu je provedena kontrola maximalni vstupni
 *          hodnoty unsigned long, sklonovani a maximalni mozne hodnoty
 *          u mensich jednotek nez nejvyssi jednotky vstupu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* Stavy pro funkci readData */
#define NUMBER 0  // Stav pro nacitani cisla ze vstupu
#define STRING 1  // Stav pro nacitani retezce ze vstupu

/* Pocty sekund v poradi tyden, den, hodina, minuta, sekunda */
unsigned long values[5] = {604800, 86400, 3600, 60, 1};

unsigned long count[5] = {1, 7, 24, 60, 60};

/* Sklonovane nazvy casovych udaju */
char *names[5][3] = {{"tyden", "tydny", "tydnu"},
                     {"den", "dny", "dnu"},
                     {"hodina", "hodiny", "hodin"},
                     {"minuta", "minuty", "minut"},
                     {"sekunda", "sekundy", "sekund"}};

/* Kody chyb a stavu programu */
enum error
{
  OK = 0,    /* Bez chyby */
  UNIT_OVER, /* Pokud je presah u jednotky */
  BAD_CMD,   /* Chyba parametru prikazoveho radku */
  MORE_CMD,  /* Prilis mnoho parametru */
  BAD_IN,    /* Chybny vstup */
  BAD_CHAR,  /* Chybny znak na vstupu */
  BAD_STRING,/* Chybny retezec na vstupu */
  BAD_VALUE, /* Vstupni cislo je vetsi nez lze vyjadrit v unsigned long */
  OTHER,     /* Neznama neocekavana chyba */
  HELP,      /* Tisk napovedy */
  NO_VALUE,  /* Nebyla zadana hodnota na vstupu */
  EXTRA,     /* Byl zadan parametr rozsireni */
  E_END      /* Zarazka tohoto seznamu */
};

/* Kody maximalnich zobrazovanych hodnot */
enum unit
{
  WEEK = 0, /* Tyden */
  DAY,      /* Den */
  HOUR,     /* Hodina */
  MINUTE,   /* Minuta */
  SECOND,   /* Sekunda */
  U_END     /* Zarazka hodnot */
};

/* Pole textovych retezcu vypisovanych funkci printErr */
const char *ERRMSG[] =
{
 /* OK */
 "Vse je v poradku.\n",
 /* UNIT_OVER */
 "Vetsi nez mozna hodnota u mensi jednotky nez u nejvyssi jednotky vstupu.\n",
 /* BAD_CMD */
 "Chybne parametry prikazoveho radku.\n",
 /* MORE_CMD */
 "Prilis mnoho parametru prikazoveho radku.\n",
 /* BAD_IN */
 "Chyba ve vstupnich datech.\n",
 /* BAD_CHAR */
 "Nepovoleny znak na vstupu.\n",
 /* BAD_STRING */
 "Chybny retezec na vstupu.\n",
 /* BAD_VALUE */
 "Vstupni hodnota je vetsi nez unsigned long.\n",
 /* OTHER */
 "Neznama neocekavana chyba.\n",
 /* HELP */
 "Program Prevod casovych udaju\n"
 "Autor: Radim Kubis, xkubis03 (c) 2011\n"
 "\n"
 "Program prevadi ciselnou hodnotu na lidsky citelny casovy udaj.\n"
 "\n"
 "Na standardnim vstupu ocekava nulu nebo prirozene cislo, ktere\n"
 "vyjadruje pocet sekund, a na standardni vystup vypise tutez hodnotu,\n"
 "ale v lidsky citelne podobe, jako pocet tydnu, dnu, hodin, minut\n"
 "a sekund, pritom nulove udaje pri vystupu vynecha, krome pripadu,\n"
 "kdy je vstupni cislo nulove.\n"
 "\n"
 "Pouziti: proj1 \n"
 "         proj1 --help\n"
 "         proj1 -t\n"
 "         proj1 -d\n"
 "         proj1 -h\n"
 "         proj1 -m\n"
 "         proj1 -s\n"
 "         proj1 --extra\n"
 "\n"
 "Popis parametru prikazoveho radku programu:\n"
 "  --help  - vypise tuto napovedu\n"
 "  -t      - nejvyssi vypisovany udaj je tyden,\n"
 "            stejna funkcnost pri spusteni bez parametru\n"
 "  -d      - nejvyssi vypisovany udaj je den\n"
 "  -h      - nejvyssi vypisovany udaj je hodina\n"
 "  -m      - nejvyssi vypisovany udaj je minuta\n"
 "  -s      - nejvyssi vypisovany udaj je sekunda\n"
 "  --extra - prevod textu v lidsky citelne podobe na cislo\n",
 /* NO_VALUE */
 ""
};

/**
 * Struktura obsahujici hodnoty parametru prikazove radky
 */
typedef struct params
{
 int maxUnit;   /* Nejvyssi vypisovany udaj */
 int stateCode;  /* Stavovy kod programu */
} TParams;

/**
 * Vytiskne text popisujici stav nebo chybu programu
 * @param code - kod chyby nebo stavu programu, ktery nastal
 */
void printErr(int code)
{
 if(code < OK || code >= E_END)
 {
  code = OTHER;
 }

 FILE *outstream = stdout;
 if(code <= OTHER)
 {
  outstream = stderr;
 }

 fprintf(outstream, "%s", ERRMSG[code]);
}

/**
 * Zpracuje parametry prikazoveho radku a vrati je ve strukture TParams
 * Pokud je format parametru chybny, ukonci program s chybovym kodem
 * @param argc - pocet parametru
 * @param argv - pole textovych retezcu s parametry
 * @return - vraci analyzovane parametry prikazoveho radku.
 */
TParams getParams(int argc, char *argv[])
{
  /* Parametry prikazoveho radku */
  char *opts[5] = {"-t", "-d", "-h", "-m", "-s"};
  
  TParams result =
  { // inicializace struktury
   .maxUnit = WEEK,
   .stateCode = OK,
  };
  
  if(argc > 2)
  { // Spatny pocet parametru
    result.stateCode = MORE_CMD;
    return result;
  }

  if(argc == 2) {  // Byl zadan parametr
    if(strcmp("--help", argv[1]) == 0) {  // Jde o napovedu?
      result.stateCode = HELP;
      return result;
    } else if(strcmp("--extra", argv[1]) == 0) {  // Jedna se o rozsireni?
      result.stateCode = EXTRA;
      return result;
    } else {  // Byl zadan parametr nejvyssiho vypisovaneho udaje nebo neznamy
      for(int maxUnit = WEEK; maxUnit < U_END; maxUnit++) {
        if(strcmp(opts[maxUnit], argv[1]) == 0) {
          result.maxUnit = maxUnit;
          return result;
        }
      }
      result.maxUnit = U_END;  // Neznamy parametr prikazoveho radku
    }
  }
  
  if(result.maxUnit == U_END)
  { // Spatny obsah parametru
    result.stateCode = BAD_CMD;
  }

  return result;
}

/**
 * Nacte ciselny udaj ze standardniho vstupu
 * @param value - odkaz na vyslednou ciselnou hodnotu
 * @return - vraci stav/chybu nacitani cisla ze standardniho vystupu
 */
int readValue(unsigned long *value) {

  char c;  // Pomocny znak pro nacitani po znacich
  unsigned long number = 0;  // Pomocna promenna pro ulozeni aktualne nacitane
                             // cislice

  c = getchar();  // Nacteni znaku

  if(c == '\n') return NO_VALUE;  // Pokud je vstup prazdny - konec programu

  do {  // Zacatek cyklu nacitani cisla ze vstupu
    if(isdigit(c)) {
      number = (unsigned long)c - '0';  // Prevod cislice na cislo
      if((ULONG_MAX - number)/10 >= *value) {  // Kontrola preteceni rozsahu
        *value = *value * 10 + number;
      } else {  // Preteceni rozsahu vstupniho cisla - chyba
        return BAD_VALUE;
      }
    } else {  // Na vstupu nebyla cislice - chyba
      return BAD_CHAR;
    }
  } while((c = getchar()) != '\n');  // Konec cyklu - koncim s koncem radku

  return OK;  // Nacitani probehlo v poradku
}

/**
 * Nacte ciselny udaj v lidsky citelne podobe a provede jeho kontrolu 
 * @param value - odkaz na vyslednou ciselnou hodnotu
 * @return - vraci stav/chybu nacitani lidsky citelne podoby casoveho udaje
 */
int readData(unsigned long *value) {

  int kind = NUMBER;    // Prvni nacitana hodnota musi byt cislo
  int index = WEEK;        // Pomocny index
  char c[2];            // Pomocne policko pro konkatenaci
  char string[8] = "";  // Konkatenovany retezec
  unsigned long number = 0;  // Prave nactena ciselna hodnota
  unsigned long result = 0;  // Vysledna ciselna hodnota
  int maxUnit = -1;

  c[1] = '\0';  // Koncovy znak pomocneho policka

  c[0] = getchar();  // Nacteni prvniho znaku

  if(c[0] == '\n') return NO_VALUE;  // Pokud je vstup prazdny - konec programu

  while (1) {  // Zacatek cyklu analyzy vstupnich udaju
    if(kind == NUMBER && isdigit(c[0])) {  // Pokud se ma nacitat cislice
      number = (unsigned long)c[0] - '0';  // Prevod cislice na cislo
      if((ULONG_MAX - number)/10 >= result) {  // Kontrola preteceni
        result = result * 10 + number;
      } else {
        return BAD_VALUE;  // Preteceni rozsahu
      }
    } else if(kind == STRING && islower(c[0])) {  // Pokud se ma nacitat retezec
      if(strlen(string) == sizeof(string)-1) {
          return BAD_STRING;  // Delsi retezec, nez muze byt - chyba a konec
      }
      strcat(string, c);  // Konkatenace aktualniho znaku
    } else if(c[0] == ' ' || c[0] == '\n') {  // Mezera mezi udaji nebo konec
      if(kind == STRING) {  // Pokud se jednalo o retezec
        for(; index < U_END; index++) {  // Vyhledam, jaky udaj prave zpracuji
          // Podminka, pokud sklonovani vstupu je v poradku
          if((strcmp(names[index][0], string) == 0 && result == 1) ||
             (strcmp(names[index][1], string) == 0 && result <= 4 && result > 1) ||
             (strcmp(names[index][2], string) == 0 && (result > 4 ||
             (result == 0 && index == SECOND && *value == 0)))
            ) {
            if(result == 0) return OK;  // Vstup byl nula sekund - v poradku
            
            // Kontrola presahu u mensich jednotek vstupu
            if(maxUnit == -1) {
              maxUnit = index;
            } else {
              if(result >= count[index]) {
                return UNIT_OVER;
              }
            }
            
            // Pokud byla vstupni casova v jejim rozsahu - vypoctu
            if((ULONG_MAX / result) >= values[index] &&
               (ULONG_MAX - (result * values[index])) >= *value
              ) {
              *value = *value + (result * values[index]);
            } else {  // Pokud casova jednotka neni ve svem rozsahu - chyba
              return BAD_VALUE;
            }          
            // Vynuluji pomocne promenne pro dalsi udaj    
            number = 0;
            result = 0;
            strcpy(string, "");
            break;  // Zpracoval jsem ciselnou hodnotu i jeji casovou jednotku
          }
        }
        if(index == U_END) {  // Pokud retezec nebyl casova jednotka nebo spatny
          return BAD_STRING;
        }
      }
      // Prepnu na nasledujici ocekavany vstup cislo/retezec
      kind = (kind == NUMBER) ? STRING : NUMBER;
      if(c[0] == '\n') {  // Pokud byl posledni znak konec radku
        if(kind == NUMBER) {  // a nacital jsem v predchozim kroku retezec
          return OK;  // konec vstupu
        } else {  // a nacital jsem v predchozim kroku cislo
          return BAD_IN;  // spatny vstup
        }
      }
    } else {  // Pokud je na vstupu neocekavany znak
      return BAD_CHAR;  // chyba a konec programu
    }
    
    c[0] = getchar();
  }

  return OK;  // Analyza probehla v poradku
}

/**
 * Analyzuje vstupni ciselnou hodnotu a provadi vypis v lidsky citelne podobe
 * @param maxUnit - nejvyssi vypisovana casova jednotka
 * @param value - vstupni ciselna hodnota 
 * Nema navratovou hodnotu
 */
void printData(int maxUnit, unsigned long value) {

  unsigned long number = 0;  // Vypisovane cislo
  unsigned long remaind = value;  // Zbytek - prozatim cela vstupni hodnota
  int actUnit = WEEK;  // Aktualne zpracovavany casovy udaj

  if(value == 0) {  // Pokud je nulovy vstup
    fprintf(stdout, "0 sekund");
  }
  
  while (actUnit < U_END) {  // Pokud jsem nevycerpal vsechny casove udaje
    // Spocitam hodnotu aktualni casove jednotky
    number = (maxUnit == actUnit) ? value/values[actUnit] : (maxUnit < actUnit) ? remaind/values[actUnit] : 0;
    // a zbytek cisla
    remaind = remaind%values[actUnit];
    
    switch(number) {  // Switch pro rozhodnuti sklonovani jednotky a vypis
      case 0:
        break;
      case 1:
        fprintf(stdout, "%lu %s%s", number, names[actUnit][0], (remaind == 0) ? "" : " ");
        break;
      case 2:
      case 3:
      case 4:
        fprintf(stdout, "%lu %s%s", number, names[actUnit][1], (remaind == 0) ? "" : " ");
        break;
      default:
        fprintf(stdout, "%lu %s%s", number, names[actUnit][2], (remaind == 0) ? "" : " ");
        break;
    }
    
    actUnit++;  // Posun na dalsi jednotku
  }

  fprintf(stdout, "\n");  // Konec vystupu
}

/**
 * Vypise ciselnou hodnotu casoveho udaje zadaneho v lidsky citelne podobe
 * @param value - ciselna hodnota
 * Nema navratovou hodnotu
 */
void printValue(unsigned long value) {
  fprintf(stdout, "%lu\n", value);
}

/**
 * Hlavni funkce main, obsahuje volani funkci programu
 * @param argc - obsahuje pocet predanych parametru
 * @param argv - obsahuje retezce zadane jako parametry programu 
 * @return - vraci vysledek celeho programu - uspech/neuspech
 */
int main(int argc, char* argv[]) {

  int code = OK;  // Inicializace stavu programu
  unsigned long value = 0;  // Inicializace vstupniho cisla

  TParams params = getParams(argc, argv);  // Analyza parametru programu

  if (params.stateCode != OK && params.stateCode <= NO_VALUE)
  { // Pokud nastalo neco nestandardniho, oznamim chybu nebo tisknu napovedu 
    printErr(params.stateCode);
    return (params.stateCode >= HELP) ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  // Nacteni hodnoty cisla/lidsky citelneho udaje podle parametru
  if(params.stateCode != EXTRA) {
    code = readValue(&value);
  } else {
    code = readData(&value);
  }
  
  // Pokud nastalo neco nestandardniho, tisknu chybu
  if(code != OK) {
    printErr(code);
    return (code >= HELP) ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  // Pokud byl zadan parametr pro rozsireni
  if(params.stateCode == EXTRA) {
    printValue(value);
  } else {  // jinak standardni funkce
    printData(params.maxUnit, value);
  }

  return EXIT_SUCCESS;
}
