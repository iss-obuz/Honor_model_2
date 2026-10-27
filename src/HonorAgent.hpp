/// @file
/// @brief Agent for "culture of honor" model - declarations
/// @date 2026-10-27 (last modifications)
//*/////////////////////////////////////////////////////////

//#define TESTING_RULE_LITERALS 1    ///< VERY SLOW!!!
//#define HONOR_WITHOUT_REPUTATION 1 ///< Testing of "null hypothesis"

typedef double FLOAT;
#include "iostream"
#define USES_STDC_RAND
#include "../INCLUDE/Randoms.h"
#include "wb_ptr.hpp"
#include "symshell.h" //Types from this file are used here.

extern unsigned population_growth; ///< How population growth. PL.: SPOSOBY ROZMNAŻANIA:
                                   ///< 0-as initial distribution; PL.: 0 - wg. inicjalnych proporcji
                                   ///< 1-as local distribution;   PL.: 1 - lokalne rozmazanie losowe sąsiad
                                   ///< 2-NOT IMPLEMENTED;         PL.: 2 - lokalne rozmazanie proporcjonalne do siły
                                   ///< 3-as global distribution;  PL.: 3 - globalne, losowy agent z całości populacji
                                   ///< Default: 1; @note ONLY #1 IS USED FOR THE PAPER in 2015

// Parameters for the world of agents:
//*///////////////////////////////////
const unsigned short        SIDE = 100;    ///< SIDE*SIDE is the size of matrix.
                                           ///< PL.: SIDE*SIDE to rozmiar macierzy.
const short int        MOORE_RAD = 3;      ///< Should be int not unsigned because of randomization formula.
                                           ///< PL.: Powinno być int, nie unsigned ze względu na formułę randomizacji.
const auto            MOORE_SIZE = (4*MOORE_RAD*MOORE_RAD)+4*MOORE_RAD;   ///< Size of Moore vicinity. PL.: Wielkość otoczenia Moore'a.

/// How many extra long links as a fraction of the number of agents.
/// PL.: ile jest dodatkowych dalekich linków jako ułamek liczby agentów.
const FLOAT            FAR_LINKS_PER_AGENT=0.5; ///< Default:0.5;

/// So how many distant links will there be?
/// PL.: Ile zatem będzie dalekich linków?
const auto             FAR_LINKS = static_cast<short int>(SIDE * SIDE * FAR_LINKS_PER_AGENT * 2);

/// Maximum size of agent's link list.
/// PL.: ile maksymalnie może miec agent linków.
const auto             MAX_LINKS = MOORE_SIZE + 2/*for sure*/ + (FAR_LINKS/(SIDE*SIDE)? FAR_LINKS/(SIDE*SIDE):2);

// AGENT POPULATION PARAMETERS
// PL.: PARAMETRY POPULACJI AGENTÓW:
//*/////////////////////////////////

/// Ratio of aggressive agents. PL.: Współczynnik agresywnych agentów.
/// TODO "minus" jest sygnałem zafiksowania w trybie batch (?!?!?!)
extern FLOAT      BULLIES_RATIO; ///< Default is `-0.25`;

/// Ratio of honor agents.
/// PL.: Jaka część agentów populacji jest honorowa.
extern FLOAT      HONOR_RATIO; ///< Default is `0.25`;

/// Ratio of police callers.
/// PL.: Jaka część wzywa policję, zamiast walczyć.
extern FLOAT      CALLER_POPU; ///< Default is `0.25`;

/// Without rational strategy. If not, the missing fraction is filled in by the "rational" strategy.
/// PL.: Czy tylko 3 strategie? Jeśli nie to brakujący ułamek dopełnia strategia "racjonalna".
extern bool        ONLY3STRAT; ///< Default is `false`;

/// Probability of authority efficient interventions.
/// Z jakim prawdopodobieństwem wezwana policja obroni agenta.
extern FLOAT    POLICE_EFF; ///< Default:=0.050;

extern bool     Inherit_MAX_POWER;  ///< Default = false; Is it power-inheritable. NOT USED?
                                    ///< Czy nowi agenci dziedziczą (z szumem) "maximum power" po rodzicu?
extern FLOAT    NOISE_LIMIT;        ///< For adding to `parent.PowLimit`. Default=0.3; ==> `powerRecoveryStep()`
                                    ///< Szum dodawany do `PowLimit` rodzica. Domyślnie = 0,3;

//const FLOAT   BULLYISM_LIMIT = -1; ///< Nie używany.
//                                   ///< Gdy BULLIES_RATIO = 1 to decyduje rozkład sterowany BULLYISM_LIMIT.
//                                   ///< Jak ujemne to rozkład Pareto lub brak rozkładu, a jak dodatnie to dzwonowy.

const FLOAT     RATIONALITY=0.0;  ///< How realistically the agent evaluates their own strength.
                                  ///< Used in HonorAgent::checkPartner().
                                  ///< PL.: Jak realistycznie ocenia własną siłę vs. według własnej reputacji.

// OTHER GLOBAL PARAMETERS OF THE MODEL:
// PL: INNE GLOBALNE PARAMETRY MODELU:
//*/////////////////////////////////////
extern FLOAT    RECOVERY_POWER;   ///< Default = 0.005; What part of the agent force recovers after one step.
                                  ///< PL.: Jaką część siły odzyskuje po jednym kroku
extern FLOAT    USED_SELECTION;   ///< Default = 0.05; How easily losers die.
                                  ///< PL.: Jak łatwo przegrani umierają. Gdy 0 to brak selekcji w ogóle!
extern FLOAT    MORTALITY;        ///< Default = 0.01; How easy it is to die for reasons of chance or nonsocial — illness, accident, etc. WHEN ARE 0 = "ELVES"
                                  ///< PL.: Jak łatwo można zginąć z przyczyn losowych, czyli niespołecznych — choroba, wypadek itp. JAK 0 TO SĄ JAK "ELFY"
extern FLOAT    EXTERNAL_REPLACE; ///< Default = 0.001; What is the probability of random exchange for a new agent with the initial distribution.
                                  ///< PL.: Jakie jest prawdopodobieństwo wymiany na losowego agenta.

extern FLOAT    BULLY_AGGRESSION; ///< Random aggression of AGGRESSIVE/BULLIES. Default=0.0250;
                                  ///< PL.: POZIOM PRZYPADKOWEJ AGRESJI AGRESYWNYCH - bez kalkulacji kto silniejszy!)
extern FLOAT    HONOR_AGGRESSION; ///< Random aggression of HONOR. Default=0.0250;
                                  ///< PL.: Bazowy poziom agresji tylko dla HONOROWYCH.


extern bool     FAMILY_HONOR; ///< Switch for sharing a reputation with family.
                              ///< PL.: Czy reputacja przenosi się na członków rodziny?
extern bool     RESTRICT_FH;  ///< Limits family honor to honorable agents only.
                              ///< PL.: Ogranicza honor rodzinny tylko do honorowych agentów.

/// ONLY FOR ADVANCED ROBUSTNESS STUDIES.
/// PL.: Służy do modyfikacji stałych liczbowych używanych w regułach reakcji agenta.
#ifdef TESTING_RULE_LITERALS
extern FLOAT    TEST_DIVIDER; //default = 1.0
#else
const  FLOAT    TEST_DIVIDER=1;
#endif

/// DEFINITION OF AGENT FOR "EVOLUTION OF HONOR".
/// PL.: DEFINICJA AGENTA "EWOLUCJI HONORU".
class HonorAgent
{
 public: 
    static  wbrtm::wb_dynmatrix<HonorAgent> World; //!< One common world of agents. PL.: Wspólny świat agentów.

    // INTERNAL TYPES of HonorAgents:
    // PL.: WEWNĘTRZNE TYPY HonorAgents:
    //*/////////////////////////////////
    enum Decision {NOTHING=-1,WITHDRAW=0,GIVE_UP=1,HOOK=2,FIGHT=3,CALL_AUTH=4}; //!< Possible decisions. PL.: Możliwe decyzje.

    static const char* Decision2str(Decision Deci) //!< Textual form of possible decisions. PL.: Tekstowa forma możliwych decyzji.
    {
       static const char* Names[]={"NOTHING","WITHDRAW","GIVE_UP","HOOK","FIGHT","CALL_AUTH","?????"};
       return Names[Deci+1];
    }

    /// Structure for counting activities.
    /// PL.: Struktura do zliczania działań.
    struct Actions
    {
        unsigned   Counter=0; //!< How many times he decided. PL.: Ile razy decydował.
        unsigned     Fails=0; //!< How many times lost. PL.: Ile razy przegrał.
        unsigned Successes=0; //!< How many times he won. PL.: Ile razy wygrał.

        union //!< Action counters. PL.: liczniki akcji.
        {
          struct { unsigned nothing=0, withdraw=0, give_up=0, hook=0, fight=0, call_auth=0;};
          unsigned as_array[6];
        };

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"
        Actions() { Reset();} //!< Constructor. PL.: Konstruktor.
#pragma clang diagnostic pop
        /// Resetting counters. PL.: Zerowanie liczników.
        void Reset(){ nothing= withdraw= give_up= hook= fight= call_auth= Fails= Successes= Counter=0;}

        void Count(HonorAgent::Decision Deci); ///< Counting of decisions. PL.: Zliczanie decyzji.

        //void operator () (HonorAgent::Decision Deci) { Count(Deci);} ///< NOT USED (Operator wywołania!)

        /// @name Ratios of different actions
        // PL.: Proporcje różnych działań:
        /// @{
        //*//////////////////////////////////
        double R_NOTHING() const  { return  (Counter ==0 ? -1:(nothing == 0?0:nothing / double(Counter)));}
        double R_WITHDRAW() const { return  (Counter ==0 ? -1:(withdraw == 0?0:withdraw / double(Counter)));}
        double R_GIVE_UP() const  { return  (Counter ==0 ? -1:(give_up == 0?0:give_up / double(Counter)));}
        double R_HOOK() const     { return  (Counter ==0 ? -1:(hook == 0?0:hook / double(Counter)));}
        double R_FIGHT() const    { return  (Counter ==0 ? -1:(fight == 0?0:fight / double(Counter)));}
        double R_CALL_AUTH() const { return (Counter ==0 ? -1:(call_auth == 0?0:call_auth / double(Counter)));}
        /// @}
    };//End of struct `Actions`

    /// \brief Connection to the vicinity of an agent.
    struct LinkTo {
        unsigned X,Y;      //!< Target coordinates. PL.: Współrzędne celu.
        unsigned Parent:1; //!< This is a parent of the agent. PL.: To jest "rodzic" agenta.
        unsigned Child:1;  //!< This is one of the children.  PL.: To jest jedno z dzieci agenta.
                           //!< @note TU WAŻNA ZMIANA. Nie był to bit, ale caly `int`!
        LinkTo() {X=Y=-1;Parent=0;Child=0;}
    };

 public: 
    // ATTRIBUTES PL.: WŁAŚCIWOŚCI:
    //*////////////////////////////
    static unsigned  LifeCounter; //!< To create unique identifiers of agents. PL.: Do tworzenia unikalnych identyfikatorów agentów.
    static bool         UseTorus; //!< Is the geometry of the torus or the island with the banks. PL.: Czy geometria torusa czy wyspa z brzegami.

    unsigned          ID; //!< The unique identifier, just subsequent agents in the particular course. PL.: Unikalny identyfikator, po prostu kolejni agenci w danym przebiegu.
    unsigned HisLifeTime; //!< Life span. PL.: Czas życia.
    Actions   HisActions; //!< Counters of actions. PL.: Liczniki akcji.

    double         Power; //!< Real power or strength (0..1).
    double      PowLimit; //!< What strength can reach up. PL.: Jaką siłę może osiągnąć maksymalnie.
    double        Malice; //!< == Aggressiveness — Propensity to attack (0..1), ONLY 0 or 1 used currently. PL.: Agresywność, czyli skłonność do atakowania)
    double         Honor; //!< Propensity to be honor (0..1), ONLY 0 or 1 used currently. PL.: Skłonność do podjęcia obrony niezależnie od siły przeciwnika)
    double    CallPolice; //!< The probability of calling the police (0..1), ONLY 0 or 1 used currently. PL.: Prawdopodobieństwo wzywania policji)

    // METHODS OF CLASS AGENT:
    // PL: METODY AGENTA:
    //*///////////////////////

    /// CONSTRUCTOR. PL.: KONSTRUKTOR.
    HonorAgent():
        Neighbourhood(MAX_LINKS), HowManyNeigh(0)
       , Power(0), PowLimit(0), HisLifeTime(0)
       , Malice(0), Honor(0), HonorFeiRep(0.5), CallPolice(0.25)
       , MemOfLastDecision(HonorAgent::NOTHING), ID(0)
       , Color(RGB(25+RANDOM(220),25+RANDOM(220),25+RANDOM(220)))
       , FamColor(RGB(0,0,0))
    {}

    void randomReset(float POW_LIMIT=0); //!< Drawing attribute values. PL.: Losowanie wartości atrybutów.

    // Neighbourhood Support PL.: Obsługa połączeń:
    //*////////////////////////////////////////////
    unsigned getNeighSize() const; //!< How many registered neighbors.
                                   //!< PL.: Ile ma zarejestrowanych sąsiadów.
    bool addNeigh(unsigned x,unsigned y); //!< Adds the specified neighbor at coordinates in the world, as far as can fit on the list.
                                          //!< PL.: Dodaje sąsiada o określonych współrzędnych w świecie, o ile zmieści na liście.
    bool getNeigh(unsigned i,unsigned& x,unsigned& y) const; //!< Reads the coordinates of the i-th neighbor.
                                                             //!< PL.: Odczytuje współrzędne sąsiada i-tego sąsiada.

    //bool getNeigh(HonorAgent* Ptr,unsigned i,unsigned& x,unsigned& y) const; //!< Looking for a neighbor via the pointer PL.: Szuka sąsiada po wskaźniku.
    void forgetAllNeigh(); //!< He forgets all added neighbors, which does not mean that they forget his.
                           //!< PL.: Zapomina wszystkich dodanych sąsiadów, co nie znaczy, że oni zapominają jego.
    static bool areNeigh(int x1, int y1, int x2, int y2); //!< Checking whether the two agents are adjacent in some way.
                                                          //!< PL.: Sprawdzanie, czy dwaj agenci sąsiadują w jakiś sposób.

    // Methods for making decision. PL.: Metody podejmowania decyzji.
    Decision  checkPartner(unsigned& x, unsigned& y); //!< Choice of interaction partner. PL.: Wybór partnera interakcji.
    Decision  answerIfHooked(unsigned x, unsigned y); //!< The answer to provocation. PL.: Odpowiedź na zaczepkę.
    void      changeReputation(double delta, HonorAgent* reason, int level= 0); //!< Reputation increase or decrease with scope checking, optionally with families.
                                                      //!< PL.: Wzrost lub spadek reputacji z zabezpieczeniem zakresu i ewentualnym dziedziczeniem.
    void      lossOfPower(double delta);  //!< A decrease of the power with range protection.
                                         //!< PL.: Spadek, zużycie siły z zabezpieczeniem zera.

    /// Determines whether the first or second agent was the winner in the confrontation.
    /// PL.: Ustala czy pierwszy, czy drugi agent zwyciężył w konfrontacji.
    static bool firstWin(HonorAgent& In,HonorAgent& Ho);

    // The main dynamics of the simulation:
    //*////////////////////////////////////
    friend void initAttributes(FLOAT HowMany);      //!< First part of initialization. PL.: pierwsza część inicjalizacji.
    friend void initConnections(FLOAT HowManyFar);  //!< Second part of initialization. PL.: druga część inicjalizacji.
    friend void powerRecoveryStep();                //!< step helper. PL.: pomocnik dla kroku modelu.
    friend void deleteAllConnections();             //!< step helper. PL.: pomocnik dla kroku modelu.
    friend void resetActionMemories();              //!< step helper. PL.: pomocnik dla kroku modelu.

    /// Single simulation step. PL.: pojedynczy krok symulacji
    friend void one_step(unsigned long& step_counter);

    // Support of family relationships PL.: Obsługa stosunków rodzinnych.
    //*//////////////////////////////////////////////////////////////////
    bool    isParent(unsigned i); //!< Is this neighbor is the parent. PL.: Czy dany sąsiad jest rodzicem)
    bool    isChild(unsigned i);  //!< Is this neighbor is a child. PL.: Czy dany sąsiad jest dzieckiem)
    bool    isUsingFamilyHonor() const; //!< The use of family honor depends on the parameters and optional properties of the agent.
                                        //!< PL.: Użycie honoru rodzinnego zależy od parametrów i opcjonalnie właściwości agenta.
    bool    isMyFamilyMember(HonorAgent& Other, HonorAgent*& Cappo, int MaxLevel= 2); //!< Is another "belongs to the family"?
                                                                                      //!< By the way, we set the "Godfather".
                                                                                      //!< PL.: Czy inny "należy do rodziny"?
                                                                                      //!< Przy okazji ustalamy "Ojca chrzestnego".

    void    changeReputationThruFamily(double Delta); //!< Recursive change of the reputation of the agent down the best of 'Cappo' of the family. PL.: Rodzinna, rekurencyjna zmiana reputacji od agenta w dół najlepiej z "Cappo" rodziny.
    void    whenGodfatherDeath(); //!< It removes family links. PL.: Usuwa powiązania rodzinne.
    friend void connectWithFamily(HonorAgent& Parent, HonorAgent& Ag);  //!< It builds a family connections. PL.: Buduje powiązanie rodzinne.

    // Methods for statistics and visualization:
    // Metody statystyki i wizualizacji:
    //*/////////////////////////////////////////
    void      makeColor(HonorAgent& Parent);   //!< Creates an agent color as a variation of the parent color. Pl.: Tworzy kolor agenta jako wariacje koloru rodzica.
    ssh_rgb   getColor() const {return Color;} //!< Individual color non modifiable from the outside. PL.: Nie modyfikowalny z zewnątrz indywidualny kolor.
    ssh_rgb   getFamColor() const {return FamColor;} //!< Family color. PL.: Kolor rodzinny.
    double    getFeiReputation() const { return HonorFeiRep;} //!< Current reputation. PL.: Aktualna reputacja agenta.

    wbrtm::wb_pchar  getAgentCultureStr() const; //!< Creates textual representation of culture Agent.
                                                 //!< PL.: Zwraca tekstową reprezentację kultury agenta.
    unsigned  getAgentCultureMask() const; //!< Creates bitwise representations of the agent culture.
                                           //!< PL.: Zwraca maskę 3 bitów, pokazującą gdzie jest różne od 0.
                                           //!< Aggr: 1. bit, Honor: 2. bit, CallPoll: 3. bit.
    Decision  getLastDecision(bool clean=false); //!< The recent decision of the MC step.
                                                 //!< PL.: Ostatnia decyzja z kroku MC.

    friend void PrintHonorAgentInfo(std::ostream& o,const HonorAgent& H); //!< For human friendly inspection of the agent. 
                                                                          //!< PL.: Do łatwej i przyjaznej dla człowieka inspekcji agenta.

 private:
    [[maybe_unused]]
    const LinkTo* getNeigh(unsigned i); //!< Raw access to link data.

    wbrtm::wb_dynarray<LinkTo> Neighbourhood; //!< List of neighbors. PL.:
    unsigned                    HowManyNeigh; //!< Number of neighbors. PL.:
    Decision               MemOfLastDecision; //!< Memory of the last decision. PL.:

    double                       HonorFeiRep; //!< Reputation as a warrior (0..1).
                                              //!< PL.: Reputacja wojownika.
    ssh_rgb                            Color; //!< Unchangeable, or calculated by any of the functions for color coding.
                                              //!< PL.: Indywidualny i niezmienny lub obliczony którąś z funkcji kodujących kolor.
    ssh_rgb                         FamColor; //!< Family color, copied from the last checked cappo.
                                              //!< PL.: Kolor rodziny, skopiowany od ostatnio sprawdzonego cappo.
}; // End of class `Agent`

// Global simulation functions defined in "HonorAgent.cpp":
//*////////////////////////////////////////////////////////
void initAttributes(FLOAT HowMany);
void initConnections(FLOAT HowManyFar);
void deleteAllConnections();
void one_step(unsigned long& step_counter);
void powerRecoveryStep();
void resetActionMemories();

// INLINE IMPLEMENTATIONS:
//*///////////////////////

inline
bool HonorAgent::areNeigh(int x1, int y1, int x2, int y2) // TODO Dlaczego to jest int a nie unsigned?
{
    HonorAgent& A=HonorAgent::World[y1][x1];
    for(unsigned i=0;i< A.getNeighSize(); i++)
    {
       if(A.Neighbourhood[i].X==x2 && A.Neighbourhood[i].Y==y2) // Already on the list

                return true;
    }
    return false; // not found
}

// Accessors for statistics and visualization: PL.: Akcesory do statystyk i wizualizacji:
//-//////////////////////////////////////////////////////////////////////////////////////

/// Counting shares. PL.: Zliczanie akcji.
inline void HonorAgent::Actions::Count(HonorAgent::Decision Deci)
{
    Counter++; //Which count of actions
    switch(Deci)
    {
        case HonorAgent::WITHDRAW:  this->withdraw++; break;
        case HonorAgent::GIVE_UP:   this->give_up++; break;
        case HonorAgent::HOOK:      this->hook++; break;
        case HonorAgent::FIGHT:     this->fight++; break;
        case HonorAgent::CALL_AUTH: this->call_auth++; break;
        default: this->nothing++; break;
    }
}

/// Makes a 3-bit mask showing where it is different from 0. Aggr: 1st bit, Honor: 2nd bit, CallPoll: 3rd bit.
/// PL.: Produkuje maskę 3 bitów, pokazującą gdzie jest różne od 0. Aggr: 1. bit, Honor: 2. bit, CallPoll: 3. bit.
inline unsigned  HonorAgent::getAgentCultureMask() const
{
   unsigned Pom=0;
   if(Malice > 0)
        Pom|=0x01;

   if(Honor>0)
        Pom|=0x02;

   if(CallPolice>0)
        Pom|=0x04;

   return Pom;
}

/// Creates textual representation of culture Agent.
/// PL.: Zwraca reprezentację tekstową kultury agenta.
inline wbrtm::wb_pchar  HonorAgent::getAgentCultureStr()  const
{
   wbrtm::wb_pchar Pom(128);
   if(Malice == 0 && Honor == 0 && CallPolice == 0) //Racjonalny jest
        Pom.add("Rational");

   if(Malice > 0)
        Pom.add("Aggressive");

   if(Honor>0)
        Pom.add("Honor");

   if(CallPolice>0)
        Pom.add("Dignity");

   return Pom;
}

inline void HonorAgent::makeColor(HonorAgent& Parent)
{
    static unsigned where=0;
    where=(where+1)%3; //Losowość i tak jest w kolejności powstawania nowych agentów.
    auto pco= Parent.getColor();
    switch(where){
    case 0:pco.r+=1; break;
    case 1:pco.g+=1; break;
    case 2:pco.b+=1; break;
    default:
#ifdef NDEBUG
        pco=RGB(255,100,0); break;
#else
        assert("SHOULD NOT HAPPEN!"==nullptr);
#endif
    }
    this->Color=pco;
}

// Forward declarations of global variables and functions for statistics:
//*//////////////////////////////////////////////////////////////////////

extern double MeanFeiReputation;
extern double MeanCallPolice;
extern double MeanPower;
extern double MeanAggre;
extern double MeanHonor;

extern unsigned NumberOfKilled;
extern unsigned NumberOfKilledToday;

extern HonorAgent::Actions CountersForAll;
extern HonorAgent::Actions CountersForBullies;
extern HonorAgent::Actions CountersForHonors;
extern HonorAgent::Actions CountersForCPolice;
extern HonorAgent::Actions CountersForRationals;

/// Struct for statistical adders (kind of counters).
/// PL.: Struktura dla sumatorów statystycznych (rodzaj liczników).
struct Adder
{
    double summ;
    unsigned N;

    Adder():summ(0),N(0){}
    void Reset(){summ=0;N=0;}

    static void Reset(Adder t[],unsigned N)
    {
        for(unsigned i=0;i<N;i++)
            t[i].Reset();
    }
};

extern Adder  MnStrength[]; ///< Adders of the strengths of extreme types of agents. PL.: Sumatory sił skrajnych typów agentów.

// FOR FAMILIES VERSION - NOT USED FOR PAPER 2015:
// WERSJA DLA RODZIN - NIEUŻYWANA W ARTYKULE Z 2015:
//*/////////////////////////////////////////////////

// Is this neighbor a parent?
// PL.: Czy dany sąsiad jest rodzicem?
inline bool    HonorAgent::isParent(unsigned i)
{
    if(i< this->getNeighSize()
    && this->Neighbourhood[i].Parent==1)
        return true;
    else
        return false;
}

// Is this neighbor his child?
// PL: Czy dany sąsiad jest dzieckiem?
inline bool    HonorAgent::isChild(unsigned i)
{
    if(i< this->getNeighSize()
    && this->Neighbourhood[i].Child==1)
        return true;
    else
        return false;
}

// The use of family honor depends on the parameters and optional properties of the agent.
// PL.: The use of family honor depends on the parameters and optional properties of the agent.
inline bool    HonorAgent::isUsingFamilyHonor() const
{
    if(!FAMILY_HONOR)
        return false; //Wcale nie ma honoru rodzinnego

    if(!RESTRICT_FH)
        return true; //Honor rodzinny nie jest ograniczony — każdy go używa

    if(this->Honor>0.9999) // Tylko naprawdę honorowi używają honoru rodzinnego
        return true;
    else
        return false;
}


// It removes family links. PL.: Usuwa powiązania rodzinne.
inline void     HonorAgent::whenGodfatherDeath()
{
    for(unsigned i=0;i< this->getNeighSize(); i++)
    {
        if(Neighbourhood[i].Parent==1) //Rodzic agenta, jeśli żyje, zostaje powiadomiony, że stracił dziecko.
        {
            Neighbourhood[i].Parent=0; //Dla porządku
            HonorAgent& Rodzic=World[Neighbourhood[i].Y][Neighbourhood[i].X];
            for(unsigned j=0;j< Rodzic.getNeighSize(); j++)
            if( this==&World[Rodzic.Neighbourhood[j].Y][Rodzic.Neighbourhood[j].X] ) //Siebie znalazł na liście...
            {                                                                 assert(Rodzic.Neighbourhood[j].Child==1);
                Rodzic.Neighbourhood[j].Child=0;
                //Rodzic.notifyChildDeath(j); //?
                break;
            }
        }
        else
        if(Neighbourhood[i].Child==1) //Dzieci agenta, jeśli żyją, zostają powiadomione, że straciły rodzica
        {
            Neighbourhood[i].Child=0; //Dla porządku
            HonorAgent& Dziecko=World[Neighbourhood[i].Y][Neighbourhood[i].X];
            for(unsigned j=0;j< Dziecko.getNeighSize(); j++)
            if( this==&World[Dziecko.Neighbourhood[j].Y][Dziecko.Neighbourhood[j].X] ) //Siebie znalazł na tej liście...
            {                                                                assert(Dziecko.Neighbourhood[j].Parent==1);
                Dziecko.Neighbourhood[j].Parent=0;
                //Rodzic.notifyChildDeath(j); //?
                break;
            }
        }
    }
}

// It builds a family connections. PL.: Buduje powiązanie rodzinne. Anty `whenGodfatherDeath()` - buduje powiązania.
inline void connectWithFamily(HonorAgent& Parent, HonorAgent& Ag)
{
    for(unsigned i=0; i < Ag.getNeighSize(); i++)
        if(&Parent == &HonorAgent::World[Ag.Neighbourhood[i].Y][Ag.Neighbourhood[i].X] ) //Znalazł rodzica na swojej liście...
        {                                                                       assert(Ag.Neighbourhood[i].Parent == 0);
            Ag.Neighbourhood[i].Parent=1;
            break;
        }

    for(unsigned j=0; j < Parent.getNeighSize(); j++)
        if(&Ag == &HonorAgent::World[Parent.Neighbourhood[j].Y][Parent.Neighbourhood[j].X] ) //Siebie odnalazł na liście u rodzica...
        {                                                                    assert(Parent.Neighbourhood[j].Child == 0);
            Parent.Neighbourhood[j].Child=1;
            //Parent.notifyChildBirth(j); //?
            break;
        }
}

//*//////////////////////////////////////////////////////////////////////////////
// Culture of honor evolution
//
// "The Evolutionary Basis of Honor Cultures"
//
// Contributors:
//    Andrzej Nowak Michele Gelfand Wojciech Borkowski
//*//////////////////////////////////////////////////////////////////////////////
