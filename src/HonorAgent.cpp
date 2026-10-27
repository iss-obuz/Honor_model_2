/// @file
/// @brief Agent for "Culture of Honor" - IMPLEMENTATIONS of METHODS
/// @date 2026-10-27 (last modifications)
//*/////////////////////////////////////////////////////////////////

#include <iostream>

#ifdef WINDOWS
#define _USE_MATH_DEFINES //bo M_PI
#endif
#include <cmath>


#define HIDE_WB_PTR_IO 1
#include "wb_ptr.hpp"

#include "symshell.h"
#include "HonorAgent.hpp"
#include "asserted.h"

using namespace wbrtm;
using namespace std;

// HonorAgent class static fields (Pola statyczne klasy HonorAgent):
//*/////////////////////////////////////////////////////////////////

bool     HonorAgent::UseTorus=false; ///< World topology. PL.: Topologia świata.
unsigned HonorAgent::LifeCounter=0;  ///< How many living agents. PL.: Ile żywych agentów.

wb_dynmatrix<HonorAgent> HonorAgent::World; ///< WORLD OF AGENTS. PL.: ŚWIAT AGENTÓW.

// Counters for statistics inside simulation procedures:
// (Liczniki do statystyk wewnątrz procedur symulacji)
//*/////////////////////////////////////////////////////
unsigned HonorRandomAttack=0;  ///< Honor Culture Random Attack Counter.
unsigned AggreRandomAttack=0; ///< Counter for random aggressive "culture" attacks.

// HANDLING AGENTS' WORLD INITIALIZATION (OBSŁUGA INICJALIZACJI ŚWIATA AGENTÓW):
//*/////////////////////////////////////////////////////////////////////////////

/// Random initialization of agents' attributes. PL.: Losowa inicjalizacja atrybutów agentów.
void initAttributes(FLOAT /*HowMany*/)
{
    for(int y=0;y<SIDE;y++)
        for(int x=0;x<SIDE;x++)
            HonorAgent::World[y][x].randomReset();
}

/// Randomizing the agent's attribute values. PL.: Losowanie wartości atrybutów agenta.
///\default POW_LIMIT==0
void HonorAgent::randomReset(float POW_LIMIT)
{
        this->ID=++LifeCounter;
        this->HisActions.Reset();
        this->HisLifeTime=0;

        this->Color=RGB(25+RANDOM(220),25+RANDOM(220),25+RANDOM(220));
        this->FamColor=RGB(0,0,0);

        // Sets the maximum strength it can achieve without losing power.
        // PL.: Ustawia jaką siłę może osiągnąć maksymalnie, gdy nie traci.
        if(POW_LIMIT > 0)
            PowLimit=POW_LIMIT;
        else // Or it draws to determine the maximum strength it can achieve.
            // PL.: Albo losuje jaką siłę może osiągnąć maksymalnie
            PowLimit=(DRAND()+DRAND()+DRAND()+DRAND()+DRAND()+DRAND())/6;

        Power=(0.5+DRAND()*0.5)*PowLimit; // Always at least 1/2 strength limit. PL.: Zawsze przynajmniej 1/2 limitu siły.

        // A warrior's reputation. Initially "in appearance," then from actual confrontations.
        // Initialization: Used to be `DRAND()*Power`, and used to be `DRAND()*0.5` as well. Later simplified to `Power`;
        //PL.: Reputacja wojownika. Najpierw "z wyglądu", a potem z realnych konfrontacji
        // Inicjalizacja: Kiedyś `DRAND()*Power`, a kiedyś było `DRAND()*0.5` też. Potem uproszczone do `Power`;
        HonorFeiRep=Power;
                                                                                                 assert(0<=HonorFeiRep);
                                                                                                 assert(HonorFeiRep<=1);

        // Malice, Honor, CallPolice...
        //-/////////////////////////////

        // Alternate INITIALIZATION, i.e. one with probability 1 and the remainder with 0.
        // PL.: INICJOWANE alternatywnie, czyli jeden z prawdopodobieństwem 1 a reszta 0.
        //*///////////////////////////////////////////////////////////////////////////////////////////////////
        Honor=0;CallPolice=0; //Default values (Wartości domyślne)
                                                                                         assert(fabs(BULLIES_RATIO) < 1);

        if(fabs(BULLIES_RATIO) < 1.0)
        {
            Malice=(DRAND() < fabs(BULLIES_RATIO)?1.0:0.0); // Either he is AGGRESSIVE or he is not.
                                                            //PL.: Albo jest AGRESYWNY, albo nie jest.

           if(Malice != 1) //If not... PL.: Jak nie jest...
           {
             Honor=(DRAND()*(1-fabs(BULLIES_RATIO)) < fabs(HONOR_RATIO)?1.0:0.0 ); //Jest HONOROWY albo nie jest

             //NOT AGGRESSIVE and NOT HONORABLE, therefore "dignified" or rational.
             //PL.: NIE AGRESYWNY i NIE HONOROWY, zatem "godnościowy" (CallPolice) albo racjonalny.
             if(Honor!=1)
             {                 //If 'TRUE', then RATIONALS disappear completely, i.e., calling the police is then a "residual strategy".
               if(!ONLY3STRAT) //PL.: Jeśli TRUE to całkiem znikają RACJONALNI, czyli wzywanie policji jest wtedy "strategią resztową".
                    CallPolice=(DRAND()*(1 - fabs(BULLIES_RATIO) - fabs(HONOR_RATIO)) < fabs(CALLER_POPU)?1.0:0.0); //Jest "dignified" albo nie.
               else
                    CallPolice=1; //Nie ma już strategii "racjonalnej"
             }
             else
             {
#ifndef NDEBUG 
              // cerr<<"?"; //  When? Not Aggressive but honor
#endif
             }
           }
        }
        else
        {
            Malice=1; //Ma sens tylko wtedy jak 1 to sami agresywni. ALE CZY TAK KIEDYŚ ROBILIŚMY! (TODO???)
        }
}

/// Random-matrix connections initialization. This means that they are local but also distant.
/// PL.: Losowo-matrycowa inicjalizacja połączeń. To oznacza, że są lokalne, ale też dalekie.
void initConnections(FLOAT HowManyFar)
{
    // Connections within nearest vicinity. PL.: Połączenia z najbliższymi sąsiadami.
    for(int x=0;x<SIDE;x++)
        for(int y=0;y<SIDE;y++)
        {   /// Reference to the agent so as not to constantly count indexes.
            /// PL.: Referencja do agenta, żeby ciągle nie liczyć indeksów.
            HonorAgent& Ag=HonorAgent::World[y][x];

            for(int xx=x-MOORE_RAD;xx<=x+MOORE_RAD;xx++)
                for(int yy=y-MOORE_RAD;yy<=y+MOORE_RAD;yy++)
                    if(!(xx==x && yy==y)) // Self-neglect 😉 PL.: Pominięcie samego siebie
                    {
                        if(HonorAgent::UseTorus) //WHAT TOPOLOGY OF THE WORLD? PL.: JAKA TOPOLOGIA ŚWIATA?
                            Ag.addNeigh((xx+SIDE)%SIDE,(yy+SIDE)%SIDE); //TORUS!
                        else
                        if(0<=xx && xx<SIDE && 0<=yy && yy<SIDE)
                            Ag.addNeigh(xx,yy); //bez boków
                    }
        }
    // Long distance connections must be created after local ones to avoid duplication.
    // PL.: Dalekie połączenia muszą być tworzone po lokalnych, żeby się nie dublowały.
    for(int f=0;f<HowManyFar;f++)
    {
        unsigned x1,y1,x2,y2;

        // Searching for agents with free slots, i.e. those initiating long-distance connections.
        // PL.: Poszukanie agentów z wolnymi slotami, czyli inicjujących dalekie połączenia.
        do{
            x1=RANDOM(SIDE);
            y1=RANDOM(SIDE);
        }while(HonorAgent::World[y1][x1].getNeighSize() >= MAX_LINKS);

        // The "passive" recipient of a distant connection cannot be the same agent or a close neighbor!
        // PL.: "Bierny" odbiorca dalekiego połączenia nie może być tym samym agentem ani bliskim sąsiadem!
        do{
            x2=RANDOM(SIDE);
            y2=RANDOM(SIDE);
        }while((x1==x2 && y1==y2)                            //Self-neglect again! PL.: Nie sam na siebie, znowu!
                || HonorAgent::World[y2][x2].getNeighSize() >= MAX_LINKS //Must have a free slot. PL.: Musi mieć wolny slot.
                || HonorAgent::areNeigh(asserted<int>(x1), asserted<int>(y1),   // HOWEVER, CAN ANYONE BE ON THE LIST TWICE!?
                                        asserted<int>(x2), asserted<int>(y2))); // PL.: JEDNAK CZY KTOŚ MOŻE BYĆ DWA RAZY NA LIŚCIE!?

        //Połączenie ich linkami w obie strony
        HonorAgent::World[y1][x1].addNeigh(x2,y2);
        HonorAgent::World[y2][x2].addNeigh(x1,y1);
    }
}

//  HANDLING OF THE SIMULATION DYNAMICS (OBSŁUGA DYNAMIKI SYMULACJI):
//*//////////////////////////////////////////////////////////////////

/// \brief
/// The choice of an interaction partner by the agent who was given a random initiative, and the decision of what the agent with initiative should do.
/// PL.: Wybór partnera interakcji przez agenta, który dostał losową inicjatywę, oraz decyzja co agent z inicjatywą ma zrobić.
/// \details
///     Both the Honor and the Aggressive have spontaneous aggression, which can be differently probable for each culture.
///     In both cases, it plays the role of a "challenge" - it verifies whether the existing reputation is true.
///     Without it, someone "strong by nature" could never be brought to reality.
///     Besides, Aggressive people have their own strategic aggression, i.e., attacking the weaker ones.
///     When the Aggressiveness attribute is less than 1, the attack may not always be performed, but
///     we did not use this option in the article, as at the end of family/clan honor.
/// PL:
///     Zarówno Honorowi, jak i agresywni mają spontaniczną agresję, która może być różnie prawdopodobna dla każdej z kultur.
///     W obu przypadkach odgrywa ona rolę "challenge-u" - weryfikuje czy istniejąca reputacja jest prawdziwa.
///     Bez tego ktoś "silny z natury" mógłby nigdy nie zostać sprowadzony do realiów.
///     Poza tym "Agresywni" mają swoją agresję strategiczną, czyli atakowanie tylko słabszych.
///     Gdy atrybut agresywności jest mniejszy niż 1, to atak może być przeprowadzany nie zawsze
///     , ale z tej możliwości nie korzystaliśmy w artykule, podobnie jak w końcu z honoru rodzinnego/klanowego.
///
HonorAgent::Decision  HonorAgent::checkPartner(unsigned& x, unsigned& y)
{
    this->MemOfLastDecision=WITHDRAW; //DEFAULT DECISION; PL.: DOMYŚLNA DECYZJA

    unsigned L=RANDOM(HowManyNeigh); //Determining which neighbor will be the partner. PL.: Ustalenie, który sąsiad będzie partnerem.

    if(getNeigh(L,x,y) ) //Getting neighbor coordinates. PL.: Pobranie współrzędnych sąsiada.
    {
        HonorAgent& Ag=World[y][x];    //Remembering a neighbor's reference. PL.: Zapamiętanie referencji do sąsiada.

        // MAIN DECISIONS RULES START BELOW. PL.: GŁÓWNE REGUŁY DECYZYJNE ZACZYNAJĄ SIĘ PONIŻEJ.
        //*/////////////////////////////////////////////////////////////////////////////////////

        // DOES BULLY WANT TO ATTACK? PL.: CZY PRZEŚLADOWCA CHCE ATAKOWAĆ?
        if(
            ( this->Malice == 1 //FULLY AGGRESSIVE ALWAYS WANTS TO ATTACK! PL.: W PEŁNI AGRESYWNY ZAWSZE MA CHĘĆ ATAKOWAĆ!
              ||
                     // If not fully aggressive, he randomly wants to! PL.: Jak nie w pełni agresywny to losowo ma chęć!
              (this->Malice > 0.0 && DRAND() < this->Malice) //NOT USED REALLY in PAPER 2015!
            )
        &&  //However, they decide when the opponent seems WEAKER! PL.: Jednak decydują się, gdy przeciwnik wydaje się SŁABSZY!
            ( Ag.HonorFeiRep <= (RATIONALITY * this->Power + (1 - RATIONALITY) * HonorFeiRep)
        ||
            ( DRAND() < BULLY_AGGRESSION))  // OR Spontaneous aggression, without "mental" calculation.
        )                                   // PL.: Agresywność spontaniczna, bez "umysłowej" kalkulacji.
        {                                                                                       assert(this->Malice > 0.0);
            this->MemOfLastDecision=HOOK; //Zaczepka od agresywnego
            if(  Ag.HonorFeiRep>this->HonorFeiRep )
                AggreRandomAttack++; //Zliczanie zaczepek na "silniejszych".
        }
        else if( //Apart from the aggressive ones, perhaps only the HONORABLE ones have non-zero aggression due to misunderstandings.
                 //PL.: Poza agresywnymi może tylko HONOROWI mają niezerową agresję z powodu nieporozumień.
            //TODO??? Czy coś tu było w poprzednich wersjach?
            (Honor>0 && DRAND()<Honor*HONOR_AGGRESSION)
        )
        {
            this->MemOfLastDecision=HOOK; //Ewentualna zaczepka losowa honorowych
            HonorRandomAttack++; //Zliczanie ataków honorowych
        }
        //DRAND()<RANDOM_AGGRESSION)                    //+losowe przypadki agresji u wszystkich. POMYSŁ PORZUCONY.
    }

    this->HisActions.Count(this->MemOfLastDecision);

    return this->MemOfLastDecision;
}

/// \brief
/// The passive agent's response to the taunt.
/// PL.: Odpowiedź pasywnego agenta na zaczepkę.
/// \details
///  It also depends on the culture. The default decision is to give up, and it applies to both the "aggressive" and the rational.
///  Which makes sense since the attacker has ALWAYS a higher reputation than the attacked one. Or so he thinks.
///  However:
///  Agents of "DIGNITY" culture call the police with probability depending on how much they identify with this culture.
///  But in the 2015 article, this identification is always 1, so the police are always called by them.
///  Honor culture agents ALWAYS respond to the taunt (unless they are not 100% honorable, but we haven't tested that either).
/// PL:
///  To też zależy od kultury. Domyślną decyzją jest się poddać i dotyczy zarówno "agresywnych" jak i racjonalnych.
///  Co jest o tyle sensowne, że atakujący ma i tak ZAWSZE wyższą reputację niż atakowany. Albo tak mu się przynajmniej wydaje.
///  Jednakże:
///  Agenci kultury "DIGNITY" wzywają policję z prawdopodobieństwem zależnym od tego, jak bardzo się z tą kulturą identyfikują.
///  W artykule z 2015 ta identyfikacja zawsze wynosi 1, czyli policja jest zawsze przez nich wzywana.
///  Agenci kultury honoru ZAWSZE odpowiadają na zaczepkę (chyba że nie są w 100% honorowi, ale tego też nie testowaliśmy).
///
HonorAgent::Decision  HonorAgent::answerIfHooked(unsigned x, unsigned y)
{
     this->MemOfLastDecision=GIVE_UP;  //DEFAULT DECISION; PL.: DOMYŚLNA DECYZJA

     HonorAgent& Ag=World[y][x];     //Remembering a neighbor's reference. PL.: Zapamiętanie referencji do sąsiada.

     // RULE DEPENDS ON POLICE "EDUCATION" OR HONOR AND OWN SENSE OF STRENGTH.
     // PL.: REGUŁA ZALEŻNA OD "WYCHOWANIA" POLICYJNEGO LUB OD HONORU i WŁASNEGO POCZUCIA SIŁY.
     if(this->CallPolice>0.999999
     || (this->CallPolice>0 && DRAND()<this->CallPolice)
     )
     {
        this->MemOfLastDecision=CALL_AUTH; // DIGNITY CULTURE CALLS THE POLICE. PL.: KULTURA DIGNITY WZYWA POLICJĘ.
     }
     else if(this->Honor>0.999999 // W PEŁNI HONOROWI ZAWSZE DECYDUJĄ SIĘ WALCZYĆ.
          || (this->Honor>0 && DRAND()<this->Honor) // Less honorable only sometimes... (Not in the 2015 article).
                                                    // PL.: Mniej honorowi tylko czasem... (Nie w artykule z 2015).
          || Ag.HonorFeiRep<this->HonorFeiRep       // BUT EVERYONE EXCEPT THE DIGNITY WHEN THE OPPONENT SEEMS WEAKER
                                                    // PL.: ALE WSZYSCY POZA POLICYJNYMI GDY PRZECIWNIK WYDAJE SIĘ SŁABSZY.
   //     || Ag.HonorFeiRep<this->Power /*HonorFeiRep*/   //Tylko do wersji 0.2.60
          )
          {
             this->MemOfLastDecision=FIGHT; // FIGHTING IS A DECISION OF THE HONORABLE, AS WELL AS THE AGGRESSIVE AND RATIONAL WHEN THE ATTACKER IS WEAKER.
                                            // PL.: WALKA TO DECYZJA HONOROWYCH, ORAZ AGRESYWNYCH I RACJONALNYCH GDY ZACZEPIAJĄCY SŁABSZY.
          }

     this->HisActions.Count(this->MemOfLastDecision);
     return this->MemOfLastDecision;
}

/// \brief
/// The main dynamics of contacts.
/// PL.: Główna dynamika kontaktów.
/// \details
/// The Monte Carlo loop takes place:
///     * Draw an active agent.
///     * Setting who is interacting with it and what kind of interaction it is.
///     * Check the hooked agent's response.
///     * Handling the results of the interaction.
/// PL:
/// W pętli Monte Carlo się odbywa:
///     * Losowanie agenta aktywnego.
///     * Ustalanie z kim wchodzi w interakcje i jakiego rodzaju to interakcja.
///     * Sprawdzenie odpowiedzi agenta zaczepionego.
///     * Obsługa rezultatów interakcji.
///
void one_step(unsigned long& step_counter)
{
    unsigned N=(SIDE*SIDE)/2; //PL.: Ile losowań w kroku MC? Połowa (!), bo w każdej interakcji biorą udział dwaj agenci
    for(unsigned i=0;i<N;i++)
    {
        unsigned x1=RANDOM(SIDE);
        unsigned y1=RANDOM(SIDE);
        HonorAgent& AgI=HonorAgent::World[y1][x1];  //PL.: Zapamiętanie referencji do agenta inicjującego

        unsigned x2,y2; //PL.: Współrzędne sąsiada ustawiane w wywołaniu funkcji poniżej.
        HonorAgent::Decision Dec1= AgI.checkPartner(x2, y2); //PL.: Kto jest partnerem i jaka akcja?

        if(Dec1==HonorAgent::HOOK) //If that one bothers you. PL.: Jeżeli tamten zaczepia.
        {                                                                             assert(AgI.Malice > 0 || AgI.Honor > 0);
           HonorAgent& AgH=HonorAgent::World[y2][x2]; //PL.: Zapamiętanie referencji do agenta zaczepionego

           AgI.changeReputation(+0.05 * TEST_DIVIDER, AgH); //PL.: Od razu dostaje podwyższenie reputacji za samo wyzwanie
           HonorAgent::Decision Dec2= AgH.answerIfHooked(x1, y1);

           switch(Dec2){
           case HonorAgent::FIGHT:
                       if(HonorAgent::firstWin(AgI,AgH))
                       {
                           AgI.changeReputation(+0.35 * TEST_DIVIDER, AgH); //PL.: Zyskał, bo wygrał
                           AgI.HisActions.Successes++;
                           AgI.lossOfPower(-0.75 * TEST_DIVIDER * DRAND()); //PL.: Stracił siły, bo walczył

                           AgH.changeReputation(+0.1 * TEST_DIVIDER, AgI);  //PL.: Zyskał, bo stanął do walki
                           AgH.lossOfPower(-0.95 * TEST_DIVIDER * DRAND()); //PL.: Stracił siły, bo przegrał walkę
                           AgH.HisActions.Fails++;
                       }
                       else
                       {
                           AgI.changeReputation(-0.35 * TEST_DIVIDER, AgH); //PL.: Stracił, bo zaczepił i dostał bęcki
                           AgI.HisActions.Fails++;
                           AgI.lossOfPower(-0.95 * TEST_DIVIDER * DRAND()); //PL.: Stracił siły, bo przegrał walkę

                           AgH.changeReputation(+0.75 * TEST_DIVIDER, AgI); //PL.: Zyskał, bo wygrał choć był zaczepiony
                           AgH.HisActions.Successes++;
                           AgH.lossOfPower(-0.75 * TEST_DIVIDER * DRAND()); //PL.: Stracił siłę, bo walczył
                       }
                    break;
           case HonorAgent::GIVE_UP:
                       {
                           AgI.changeReputation(+0.5 * TEST_DIVIDER, AgH); //PL.: Zyskał, bo wygrał bez walki.
                                                                           //PL.: Czy na pewno więcej niż w walce???
                           AgI.HisActions.Successes++;

                           AgH.changeReputation(-0.5 * TEST_DIVIDER, AgI); //PL.: Stracił, bo się poddał
                           AgH.HisActions.Fails++;

                           AgH.lossOfPower(-0.5 * TEST_DIVIDER * DRAND()/*DRAND()*/); //PL.: Ale trochę dostał w ucho dla przykładu   (!!!)
                       }
                    break;
           case HonorAgent::CALL_AUTH:
                      if(DRAND() < POLICE_EFF) //Czy pomoc przybyła?
                      {
                          AgI.changeReputation(-0.35 * TEST_DIVIDER, AgH); //PL.: Stracił, bo zaczepił i dostał bęcki od policji
                          AgI.HisActions.Fails++;
                          AgI.lossOfPower(-0.99 * TEST_DIVIDER * DRAND()); //PL.: Stracił siły, bo walczył i przegrał z przeważającą siłą

                          AgH.HisActions.Successes++;                     //PL.: Zaczepionemu udało się uniknąć "bęcek", ale...
                          AgH.changeReputation(-0.75 * TEST_DIVIDER, AgI); //    Stracił reputację, bo wezwał policje, zamiast walczyć
                      }
                      else //PL.: A może policja nie przybyła?
                      {
                          AgI.changeReputation(+0.5 * TEST_DIVIDER, AgH); //PL.: Zyskał, bo wygrał bez walki.
                          AgI.HisActions.Successes++;

                          AgH.changeReputation(-0.75 * TEST_DIVIDER, AgI); //PL.: Zaczepiony stracił, bo wezwał policje
                          AgH.lossOfPower(-0.99 * TEST_DIVIDER * DRAND()); //PL.: Stracił, bo się nie bronił, a wkurzył, wzywając policję.
                          AgH.HisActions.Fails++;
                      }
                    break;

           //Responses to provocation that shouldn't happen. PL.: Odpowiedzi na zaczepkę, które się nie powinny zdarzać.
           case HonorAgent::WITHDRAW: /* IT CAN'T BE HERE! PL.: TU NIE MOŻE BYĆ! */
                  cout<<"?";
           case HonorAgent::HOOK:     /* IT CAN'T BE HERE! PL.: TU NIE MOŻE BYĆ! */
                  cout<<"??";
           default:                   /* TU NIE MOŻE BYĆ! PL.: MOCNO Podejrzane! Nieznane akcje nie powinny się zdarzać! */
                  cout<<"??? ";
           break;
           }
        }
        else  //When the active one didn't bother? PL.: Jak aktywny nie zaczepiał?
        {
            AgI.changeReputation(-0.0001 * TEST_DIVIDER, *(HonorAgent*)(nullptr) ); //PL.: Minimalnie traci w swoich oczach
        }
    }

    step_counter++; // KONIEC KROKU M-C
}

// (Statystyki przeżyciowe) Survival statistics:
//*/////////////////////////////////////////////

unsigned      NumberOfKilled=0; ///< Counter of all deaths. PL.:
unsigned NumberOfKilledToday=0; ///< Death counter per single step. PL.:


/// \brief
/// A strength recovering or declaring death, also optional migration.
/// PL.: Odzyskiwanie sił albo stwierdzanie zgonu. Także opcjonalna migracja.
/// \details
///     "Post-step" or rest after a hard day ;-)
///
///     If the agent did not survive because he lacked strength, he is replaced in one of several ways.
///     Its place may be taken by:
///     1) New random agent
///     2) A descendant of a neighbor
///     3) A descendant of any agent in the population
///
///     IN ADDITION:
///     If EXTERNAL_REPLACE is nonzero, a completely random agent may disappear and be replaced by a random agent.
///     This is a way of ensuring that extinct strategies can come back and test in an environment where they were lost before.
///
/// PL.:
///     "Post-krok" czyli odpoczynek po ciężkim dniu 😉
///
///     Jeśli agent nie przeżył, bo zabrakło mu siły, to jest podmieniany na jeden z kilku sposobów.
///     Jego miejsce może zająć:
///     1) Nowy losowy agent
///     2) Potomek sąsiada
///     3) Potomek dowolnego agenta z populacji
///
///     PONADTO:
///     Jeżeli EXTERNAL_REPLACE jest niezerowe, to zupełnie przypadkowy agent może zniknąć i zostać zastąpiony przez losowego.
///     To sposób zagwarantowania, że wymarłe strategie mogą wracać i testować się w środowisku, w którym wcześniej przepadły.
///
void powerRecoveryStep()
{
  NumberOfKilledToday=0;
  for(int x=0;x<SIDE;x++)
    for(int y=0;y<SIDE;y++)
    {
        HonorAgent& Ag=HonorAgent::World[y][x];    // PL.: Zapamiętanie referencji do agenta

        if(Ag.getLastDecision(false) == HonorAgent::NOTHING)  // PL.: Nie był w tym kroku ani razu wylosowany!
                Ag.HisActions.Count(HonorAgent::NOTHING); // To mu trzeba doliczyć do liczników,bo inaczej NOTHING nie byłoby tam nigdy rejestrowane!
                                                          // PL.:

        Ag.HisLifeTime++;  //PL.: Zwiększenie licznika długości życia

        if(MORTALITY>0)   //PL.: Losowa śmierć z "chorób, starości i wypadków losowych"
        {
           if(DRAND()<MORTALITY)
               Ag.Power=-1;
        }

        if( USED_SELECTION>=0 //PL.: Selekcja 0 oznacza śmierć tylko od MORTALITY i EXTERNAL_REPLACE
        && Ag.Power<USED_SELECTION)  //PL.: Siła jest poniżej progu przeżycia.
        {
           if(population_growth==0) //PL.: Tryb z prawdopodobieństwami inicjalnymi
           {                                                                                assert(FAMILY_HONOR==false);
               Ag.randomReset(); //PL.: Po prostu w jego miejsce losowy agent
                               //PL.: KOLOR POZOSTAJE LOSOWY A KOLOR RODZINNY PUSTY.
             NumberOfKilled++;
             NumberOfKilledToday++;
           }
           else
           if(population_growth==1) //PL.: Tryb lokalny. Z losowym sąsiadem jako rodzicem
           {
                unsigned which=RANDOM(Ag.getNeighSize()),xx=0xffff,yy=0xffff; //PL.:  xx i yy zostaną wypełnione zaraz, chyba że nie...
                bool pom=Ag.getNeigh(which,xx,yy);                assert(pom); //PL.: losuje z listy sąsiadów, wiec zawsze cos tam powinno być!
                HonorAgent& Rodzic=HonorAgent::World[yy][xx];

                if(Rodzic.Power>0) //PL.: Tylko wtedy może się rodzić, gdy jest i ma energie! NEW TODO Check  - JAK NIE TO ROZLICZENIE NA PÓŹNIEJ?
                {
                     if(FAMILY_HONOR) //PL.: Jeżeli są stosunki rodzinne to śmierć ma różne konsekwencje społeczne
                         Ag.whenGodfatherDeath();

                     if(Inherit_MAX_POWER)
                     {
                        float NewLimit=Rodzic.PowLimit + NOISE_LIMIT
                                      *Rodzic.PowLimit*(( (DRAND()+DRAND()+DRAND()+DRAND()+DRAND()+DRAND())/6 ) - 0.5);
                                                                                                             assert(NewLimit>0);
                        if(NewLimit>1) NewLimit=1; //PL.: Zerowy nie będzie, ale może przekraczać 1
                         Ag.randomReset(NewLimit);
                     }
                     else
                         Ag.randomReset();

                     Ag.Malice=Rodzic.Malice;
                     Ag.Honor=Rodzic.Honor;
                     Ag.CallPolice=Rodzic.CallPolice;

                     if(FAMILY_HONOR) //PL.: I urodziny także mają konsekwencje rodzinne
                     {
                        Ag.HonorFeiRep=Rodzic.HonorFeiRep; //PL.: Ma reputacje rodzica, bo on go chroni.
                        connectWithFamily(Rodzic, Ag);  //PL.:  anty `whenGodfatherDeath()`;
                     }

                     Ag.makeColor(Rodzic); //PL.: Kolor zmodyfikowanym kolorem rodzica.
                     Ag.FamColor=Rodzic.Color; //PL.: A kolor rodzinny wzięty od rodzica. Może się zmienić przy interakcjach sterowanych przez FAMILY_HONOR

                     //PL.: Aktualizacja liczników
                     NumberOfKilled++;
                     NumberOfKilledToday++;
                }
           }
           else
           if(population_growth==3) //PL.: Tryb GLOBALNY z losowym członkiem populacji jako rodzicem
           {                                                                               // assert(FAMILY_HONOR==false);
                unsigned xx=RANDOM(SIDE),yy=RANDOM(SIDE);
                HonorAgent& Drugi=HonorAgent::World[yy][xx]; //!< Keep handle to second agent. PL.: Uchwyt do drugiego agenta.
                if(Drugi.Power>0)  //PL.: Może postać do rozliczenia na później.
                {
                  Ag.randomReset();
                  Ag.Malice=Drugi.Malice;
                  Ag.Honor=Drugi.Honor;
                  Ag.CallPolice=Drugi.CallPolice;
                  Ag.makeColor(Drugi);  //PL.: Kolor zmodyfikowanym kolorem wzorca.
                  Ag.FamColor=Drugi.Color;
                  NumberOfKilled++;
                  NumberOfKilledToday++;
                }
           }
        }
        else //PL.: Ewentualnie, jeżeli jednak nie umarł, to...
        if(Ag.Power<Ag.PowLimit) //Może się leczyć ("poprawiać").
        {
            Ag.Power+=(Ag.PowLimit-Ag.Power)*RECOVERY_POWER;
        }
    }

  // Random exchange for randomly born from the default proportion
  //      i.e., an exchange of agents with a much larger external environment with stable proportions.
  // PL.: Losowa wymiana na przypadkowo narodzone (z domyślnej proporcji),
  //      czyli jakby wymiana agentów z dużo większym środowiskiem zewnętrznym o stabilnych proporcjach.
  if(EXTERNAL_REPLACE>0)
  {
    unsigned randomExchanges = SIDE * EXTERNAL_REPLACE * SIDE; //How many random exchanges? PL.: Ile Losowych Wymian?

    for(; randomExchanges > 0; randomExchanges--)
    {
      unsigned x=RANDOM(SIDE),y=RANDOM(SIDE);
      HonorAgent& Ag=HonorAgent::World[y][x];

      //Exchange according to the initial distribution can therefore bring extinct strategies back to life!
      //PL.: Wymiana według inicjalnej dystrybucji więc może przywracać do istnienia już wymarłe strategie!
      Ag.randomReset();
    }

  }
}

// CONNECTION CLEANING OPERATION. Especially important for repetitions.
// PL.: OBSŁUGA CZYSZCZENIA POŁĄCZEŃ. Ważna zwłaszcza przy powtórzeniach.
//*//////////////////////////////////////////////////////////////////////

/// Delete connections to registered neighbors.
/// PL.: Usuwanie połączeń z zarejestrowanymi sąsiadami.
void deleteAllConnections()
{
    for(int x=0;x<SIDE;x++)
        for(int y=0;y<SIDE;y++)
        {
           HonorAgent& Ag=HonorAgent::World[y][x]; //PL.: Zapamiętanie referencji do agenta.
           Ag.forgetAllNeigh(); //PL.: Bezwarunkowe zapomnienie.
        }
}


// Auxiliary agent methods and accessors PL.: Pomocnicze metody i akcesory agentów.
//*////////////////////////////////////////////////////////////////////////////////

// What was the last decision from MC step. For visualization and statistical purposes.
// PL.: Jaka była ostatnia decyzja z kroku MC. Do celów wizualizacyjnych i statystycznych.
HonorAgent::Decision HonorAgent::getLastDecision(bool clean)
{
    Decision Tmp=MemOfLastDecision;
    if(clean) MemOfLastDecision=NOTHING;
    return Tmp;
}

/// Behavior memories clearing.
/// PL.: Czyszczenie pamięci zachowań.
void resetActionMemories()
{
    for(unsigned v=0;v<SIDE;v++)
    {
        for(unsigned h=0;h<SIDE;h++)
        {
            HonorAgent::World[v][h].getLastDecision(true); //PL.: Zapamiętanie referencji do agenta.
        }
    }
}

/// Add a neighbor with specific coordinates in the world.
/// PL.: Dodawanie sąsiada o określonych współrzędnych w świecie.
bool HonorAgent::addNeigh(unsigned x,unsigned y)
{
    if(HowManyNeigh<Neighbourhood.get_size()) //PL.: Czy jest jeszcze miejsce wokół?
    {
        Neighbourhood[HowManyNeigh].X=x;
        Neighbourhood[HowManyNeigh].Y=y;
        HowManyNeigh++;
        return true;
    }
    else return false;
}

/// How many registered neighbors have. PL.: Ile ma zarejestrowanych sąsiadów.
unsigned HonorAgent::getNeighSize()const
{
    return HowManyNeigh;
}

/// He forgets all added neighbors, which doesn't mean they forget him.
/// PL.: Zapomina wszystkich dodanych sąsiadów, co nie znaczy, że oni zapominają jego.
void HonorAgent::forgetAllNeigh()
{
   HowManyNeigh=0;
}

/// Reads the coordinates of the neighbor if there is any at the index 'i'.
/// PL.: Czyta współrzędne sąsiada, o ile jest jakiś pod indeksem 'i'.
/// \return true/false, if was there a neighbor. PL.: t/f - czy był tam sąsiad?
bool HonorAgent::getNeigh(unsigned i,unsigned& x,unsigned& y) const
{
   if(i<HowManyNeigh)
   {
       x=Neighbourhood[i].X;
       y=Neighbourhood[i].Y;
       return true;
   }
   else return false;
}

/// A pointer to the link data of the i-th neighbor.
/// PL.: Wskaźnik do danych powiązania z i-tym sąsiadem.
MAYBE_UNUSED const HonorAgent::LinkTo* HonorAgent::getNeigh(unsigned i)
{
  if(i<HowManyNeigh)
      return &Neighbourhood[i];
  else
      return nullptr;
}

/// \brief
/// Determines whether the first or second agent will win the confrontation.
/// PL.: Ustala czy pierwszy, czy drugi agent zwycięży w konfrontacji.
/// \note
/// IMPORTANT SIMPLIFICATION: There is always one to win, which is not realistic!
/// PL.: WAŻNE UPROSZCZENIE: Zawsze któryś musi zwyciężyć, co nie jest realistyczne!
bool HonorAgent::firstWin(HonorAgent& In,HonorAgent& Ho)
{
    if( In.Power*(1+DRAND())  >  Ho.Power*(1+DRAND()) ) //DRAND() --> random number from 0 to 1
        return true;
    else
        return false;
}

/// Strength fall with range protection
/// PL.: Spadek siły z zabezpieczeniem zakresu.
void HonorAgent::lossOfPower(double delta)
{                                                                                                      assert(Power<=1);
    delta=fabs(delta);
    Power*=(1-delta);
    if(Power<0)
        Power=0;     //To be sure...
}

/// \brief
/// Reputation increase or decrease with range securing.
/// (Wzrost lub spadek reputacji z zabezpieczeniem zakresu)
/// \details
/// As used FAMILY_HONOR and the agent is from honor culture, it also changes for the whole family!
/// PL.: Jak używane FAMILY_HONOR i agent jest honorowy, to zmienia się tak samo całej rodzinie!
void HonorAgent::changeReputation(double delta, HonorAgent& reason, int /*level==0*/)
{                                                 //He cannot attack himself! PL.: Sam siebie nie może atakować!
                                                  assert(this!=&reason); assert(0<=HonorFeiRep); assert(HonorFeiRep<=1);
#ifdef HONOR_WITHOUT_REPUTATION //For "null hypothesis" tests PL.:
    if(this->Honor==1) //Honor agent
     {
        return; //nothing to do
     }
#endif // HONOR_WITHOUT_REPUTATION
    HonorAgent* Cappo=nullptr; //PL.: Przy okazji sprawdzenia, czy ktoś jest w tej rodzinie, ustalamy "Ojca chrzestnego".
    if( &reason != nullptr
        && isUsingFamilyHonor() //The use of family honor depends on the parameters and optional properties of the agent. PL.:
    && !isMyFamilyMember(reason, Cappo) ) //PL.: Jeżeli działa honor rodzinny to ...
    {
        if(Cappo==nullptr) //PL.: Nie ma żyjącego ojca. Tu ślad się urywa.
        {
            Cappo = this;
        }
        this->FamColor=Cappo->Color; //PL.: Jak mamy jeszcze jakiegoś cappo to przyjmujemy jego kolor.
        Cappo->changeReputationThruFamily(delta); //PL.: Ale może mieć dzieci...
    }
    else
    if(delta > 0) //PL.: Wzrost reputacji.
    {
        HonorFeiRep+= (1-HonorFeiRep) * delta;                                                 assert(HonorFeiRep <= 1);
    }
    else       //PL.: Spadek reputacji.
    {
        HonorFeiRep-=HonorFeiRep*fabs(delta);                                                  assert(0 <= HonorFeiRep);
        /* Spr.-> */                                                                             assert(HonorFeiRep<=1);
    }
}

// FAMILY RELATIONS (not used in paper from 2015):
//*///////////////////////////////////////////////

/// It checks if this "Other" agent belong to this agent's "family". By the way, we set the "Godfather".
/// PL.: Czy ten "Other" agent należy do "rodziny" danego agenta.
///      Przy okazji sprawdzenia ustalamy "Ojca chrzestnego" (Cappo) na później.
bool HonorAgent::isMyFamilyMember(HonorAgent& Other, HonorAgent*& Cappo, int MaxLevel/*=2*/)
{
    if(MaxLevel>0) //PL.: Jak jeszcze nie szczyt zadanej hierarchii, to szuka ojca
        for(unsigned i=0; i < getNeighSize(); i++)
            if(Neighbourhood[i].Parent==1) //PL.: Jeżeli ma ojca to idziemy w górę
            {
                HonorAgent& Rodzic=World[Neighbourhood[i].Y][Neighbourhood[i].X];
                Cappo=&Rodzic; //PL.: Może "Rodzic" to zmieni, ale jak nie to już znaleziony!
                return Rodzic.isMyFamilyMember(Other, Cappo, MaxLevel - 1);  //PL.: REKURENCJA!
            }

    //PL.: Jak sprawdzenie nie poszło, z tego czy innego powodu, w górę to może nadal szukać:
    if(this==&Other) //PL.: On sam okazał się szukanym!
        return true;

    //PL.: Jest najwyższym ojcem, teraz może szukać potomków...
    for(unsigned j=0; j < getNeighSize(); j++)
        if(Neighbourhood[j].Child==1) //PL.: Jest dzieckiem
        {
            HonorAgent& Dziecko=World[Neighbourhood[j].Y][Neighbourhood[j].X];
            bool res= Dziecko.isMyFamilyMember(Other, Cappo, -1);  //PL.: -1 zabezpiecza przed dalszą REKURENCJĄ
            if(res)
                return true;
        }

    //PL.: Nigdzie nie znalazł. To znaczy, że w jego pod-drzewie nie ma.
    return false;
}

/// Family reputation change. For the agent, and for his children recursively.
/// PL.: Rodzinna zmiana reputacji. Dla siebie oraz dla dzieci rekurencyjnie.
void HonorAgent::changeReputationThruFamily(double Delta)
{
    if(Delta>0) //PL.: Wzrost reputacji.
    {
        HonorFeiRep+=(1-HonorFeiRep)*Delta;                                                      assert(HonorFeiRep<=1);
    }
    else       //PL.: Spadek reputacji.
    {
        HonorFeiRep-=HonorFeiRep*fabs(Delta);                                                    assert(0<=HonorFeiRep);
        /* Spr.-> */                                                                             assert(HonorFeiRep<=1);
    }
    // A teraz dla dzieci
    for(unsigned j=0; j < getNeighSize(); j++) //PL.: Czy jest może ojcem? Trzeba szukać potomków
        if(Neighbourhood[j].Child==1)   //PL.: Ten jest dzieckiem
        {
            HonorAgent& Dziecko=World[Neighbourhood[j].Y][Neighbourhood[j].X];
            Dziecko.changeReputationThruFamily(Delta); //PL.: REKURENCJA W DLA RODZINY
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
