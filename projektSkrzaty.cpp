#include <mpi.h>
#include <stdio.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <vector>
#include <array>

#define AGRAFKI 2
#define TRUCIZNA 6
#define ZLECENIA 8

#define REQ_AGRAFKA 102
#define ACK_AGRAFKA 103
#define REQ_TRUCIZNA 104
#define ACK_TRUCIZNA 105
#define START 106
#define END 107
#define LEPSZY 108
#define MOJ_PRIORYTET 109
#define CZEKAM_NA_ZLECENIA 110

struct zlecenie{
    int id;
    int chomiki;
};

void sort(float tab[],int n)
{
    float help;
	for(int i = 0; i < n; i++)
		for(int j = 1; j < n-i; j++)
		if(tab[j-1]>tab[j]){
			help = tab[j-1];
            tab[j-1] = tab[j];
            tab[j] = help;
        }
}

zlecenie* czekajNaZlecenia(int id, int liczbaProcesow){
    zlecenie *przeslaneZlecenia = new zlecenie[ZLECENIA];
    zlecenie rcv_zlecenie;
    
    for(int i = 0; i < ZLECENIA; i++){
        MPI_Status status;
        MPI_Recv(&rcv_zlecenie, sizeof(zlecenie),MPI_CHAR, 0,START,MPI_COMM_WORLD,&status);
        if(status.MPI_TAG = START){
            przeslaneZlecenia[i] = rcv_zlecenie;
        }
    }
    return (przeslaneZlecenia);
    delete[] przeslaneZlecenia;

    ///### dodane
    float czekam = 1;
    for(int i = 1; i < liczbaProcesow; i++){
        if(i == id)
            continue;
        MPI_Send(&czekam, 1, MPI_FLOAT, i, CZEKAM_NA_ZLECENIA, MPI_COMM_WORLD);
    }
    //###
}

int ubiegajOZlecenie(int id, int liczbaProcesow, float *czasLamporta, float *tab_PriorytetyZlecen, 
                     zlecenie *wygenerowaneZlecenia, int tabSize){

    for (int i = 0; i < tabSize; i++){
        if(tab_PriorytetyZlecen[i] != 2020.0){
            tab_PriorytetyZlecen[i] = -1.0;
        }
    }
    *czasLamporta = *czasLamporta + 1;
    //wyslanie wiadomosci MOJ_PRIORYTET do procesow
    float msg = *czasLamporta + (float)id/100.0; //dzięki temu w tablicy będziemy mieć posortowane wartości wg czasów i wg id
    for (int i = 1; i < liczbaProcesow; i++){
        if(i==id)
            continue; //pomijamy siebie
        MPI_Send(&msg, 1, MPI_FLOAT, i, MOJ_PRIORYTET, MPI_COMM_WORLD);
    }
    float rcv_msg;

    /*
    //przejrzenie tab_PriorytetyZlecen, tam gdzie pierwsze 2020 - wstawic swoj priorytet;
    for(int i = 0; i < tabSize; i++){
        if(tab_PriorytetyZlecen[i] == 2020){
            tab_PriorytetyZlecen[i] = msg;
            break;
        }
    }
    */

    std::vector<float>helperTab;
    helperTab.push_back(msg);

    int procesyKtoreCzekajaNaNoweZlecenia = 0;
    int procesyKtoreWyslalyPriorytet = 0;
    while( procesyKtoreWyslalyPriorytet < liczbaProcesow - 2 - procesyKtoreCzekajaNaNoweZlecenia){ //póki nie ma wiadomości od innych skrzatów (pomija siebie i burmistrza)
        MPI_Status status;
        MPI_Recv(&rcv_msg, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        float id_procesu;

        if(status.MPI_TAG == CZEKAM_NA_ZLECENIA){
            procesyKtoreCzekajaNaNoweZlecenia++;
        }

        else if (status.MPI_TAG == MOJ_PRIORYTET){
            procesyKtoreWyslalyPriorytet++;
            /*
            for(int i = 0; i < tabSize; i++){
                if(tab_PriorytetyZlecen[i] == 2020){
                    tab_PriorytetyZlecen[i] = rcv_msg;
                    break;
                }
            }
            sort(tab_PriorytetyZlecen, tabSize);
            */
            helperTab.push_back(rcv_msg);   
        }

        else if (status.MPI_TAG == REQ_AGRAFKA ||
            status.MPI_TAG == ACK_AGRAFKA){
            /*
            float lamportInnegoProcesu = floor(rcv_msg);
            *czasLamporta = rcv_msg > msg ? lamportInnegoProcesu:*czasLamporta;
            msg = *czasLamporta + (float)id/100.0;
            */
            switch (status.MPI_TAG){
                case REQ_AGRAFKA:
                    //odesłać ACK_Trucizna
                    id_procesu = rcv_msg;
                        while (id_procesu > 1){
                            id_procesu--;
                        }
                    id_procesu = id_procesu*100;
                    MPI_Send(&msg, 1, MPI_FLOAT, id_procesu, ACK_AGRAFKA, MPI_COMM_WORLD);
                    break;
                case ACK_AGRAFKA:
                    //ignore
                    break;
            }
        }
        else if (status.MPI_TAG == REQ_TRUCIZNA ||
                 status.MPI_TAG == ACK_TRUCIZNA){
            /*
            float lamportInnegoProcesu = floor(rcv_msg);
            *czasLamporta = rcv_msg > msg ? lamportInnegoProcesu:*czasLamporta;
            msg = *czasLamporta + (float)id/100.0;
            */
            switch (status.MPI_TAG){
                case REQ_TRUCIZNA:
                    //odesłać ACK_Trucizna
                    id_procesu = rcv_msg;
                        while (id_procesu > 1){
                            id_procesu--;
                        }
                    id_procesu = id_procesu*100;
                    MPI_Send(&msg, 1, MPI_FLOAT, id_procesu, ACK_TRUCIZNA, MPI_COMM_WORLD);
                    break;
                case ACK_TRUCIZNA:
                    //ignore
                    break;
            }
        }
    }
    
    std::sort(helperTab.begin(),helperTab.end(),std::greater<float>());
    for(int i = 0; i < tabSize; i++){
        if(tab_PriorytetyZlecen[i] == 2020 && helperTab.size() > 0){
            float minPriorytet = *min_element(helperTab.begin(), helperTab.end());
            tab_PriorytetyZlecen[i] = minPriorytet;
            helperTab.pop_back();
        }
    }

    //wszystkie procesy które miały wysłać sygnal MOJ_PRIORYTET wyslaly go, wybierz zlecenie
    int wybraneZlecenie = -1;
    for(int i = 0; i < tabSize; i++){
        if(tab_PriorytetyZlecen[i] == msg){
        wybraneZlecenie = i;
        }
    }
    wybraneZlecenie = (ZLECENIA > wybraneZlecenie)?wybraneZlecenie:ZLECENIA;

    /*
    //##debug printf
    printf("STAN ZLECENIA: ");
    for (int i = 0; i < tabSize; i++){
        printf("%.2f, ",tab_PriorytetyZlecen[i]);
        if(i == tabSize - 1)
            printf("\n");
    }
    */
    return(wybraneZlecenie);
}

std::array<std::vector<float>,1> ubiegajOAgrafke(int id, int liczbaProcesow, float* czasLamporta,
                                                 float* tabPriorytetyZlecen, int wielkosctabPrioZlecen)
{
    std::array<std::vector<float>,1> kolejkaPoAgrafke;
    float msg;
    *czasLamporta += 1;
    msg = *czasLamporta + (float)id/100.0;
    //REQ_Agrafka do wszystkich oprócz siebie i burmistrza;
    for(int i = 1; i < liczbaProcesow; i++){
        if(i == id)
            continue;
        MPI_Send(&msg, 1, MPI_FLOAT, i, REQ_AGRAFKA, MPI_COMM_WORLD);
    }
    int pozwolenia_Agrafki = 1;
    float rcv_msg;
    while(pozwolenia_Agrafki < liczbaProcesow-1-AGRAFKI){//burmistrz nie wykonuje obowiązków skrzatów stąd -1
        MPI_Status status;
        MPI_Recv(&rcv_msg, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        float id_procesu;

        if (status.MPI_TAG == REQ_AGRAFKA ||
            status.MPI_TAG == ACK_AGRAFKA){

            float lamportInnegoProcesu = floor(rcv_msg);
            *czasLamporta = rcv_msg > msg ? lamportInnegoProcesu:*czasLamporta;
            msg = *czasLamporta + (float)id/100.0;

            switch (status.MPI_TAG){
                case REQ_AGRAFKA:
                    if(msg > rcv_msg){
                        pozwolenia_Agrafki++;
                        id_procesu = rcv_msg;
                        while (id_procesu > 1){
                            id_procesu--;
                        }
                        id_procesu = id_procesu*100;
                        //zapis id procesu od którego dostał wiadomość do oczekujacych na ACK_AGRAFKA
                        kolejkaPoAgrafke[0].push_back(id_procesu);
                    } else {
                        //wysłanie ACK_Agrafka do procesu który przysłał wiadomość
                        id_procesu = rcv_msg;
                        while(id_procesu > 1){
                            id_procesu--;
                        }
                        id_procesu = id_procesu*100;
                        MPI_Send(&msg, 1, MPI_FLOAT, (int)id_procesu,ACK_AGRAFKA,MPI_COMM_WORLD);
                    }
                    break;
                case ACK_AGRAFKA:
                    pozwolenia_Agrafki++;
                    break;
            }
        } else if (status.MPI_TAG == REQ_TRUCIZNA ||
                   status.MPI_TAG == ACK_TRUCIZNA){

            float lamportInnegoProcesu = floor(rcv_msg);
            *czasLamporta = rcv_msg > msg ? lamportInnegoProcesu:*czasLamporta;
            msg = *czasLamporta + (float)id/100.0;

            switch (status.MPI_TAG){
                case REQ_TRUCIZNA:
                    //odesłać ACK_Trucizna
                    id_procesu = rcv_msg;
                        while (id_procesu > 1){
                            id_procesu--;
                        }
                    id_procesu = id_procesu*100;
                    MPI_Send(&msg, 1, MPI_FLOAT, id_procesu, ACK_TRUCIZNA, MPI_COMM_WORLD);
                    break;
                case ACK_TRUCIZNA:
                    //ignore
                    break;
            }
        } else if (status.MPI_TAG == MOJ_PRIORYTET){
            for(int i = 0; i < wielkosctabPrioZlecen; i++){
                if(tabPriorytetyZlecen[i] == 2020){
                    tabPriorytetyZlecen[i] == rcv_msg;
                    break;
                }
            }
        } else if (status.MPI_TAG == CZEKAM_NA_ZLECENIA){
            //odebralem, zignorowalem
        }
    }

    /*
    //##debug printf
    printf("STAN AGRAFKI: ");
    for (int i = 0; i < wielkosctabPrioZlecen; i++){
        printf("%.2f, ",tabPriorytetyZlecen[i]);
        if(i == wielkosctabPrioZlecen - 1)
            printf("\n");
    }
    */

    printf("Skrzat %d: Mam agrafkę, idę po truciznę. Mój czas lamporta: %.2f\n",id, *czasLamporta);
    return kolejkaPoAgrafke;
}

std::array<std::vector<float>,1> ubiegajOTrucizne(int id, int liczbaProcesow, float* czasLamporta, float truciznaZeZlecenia, 
                                                  float* tabPriorytetyZlecen, int wielkosctabPrioZlecen)
{
    std::array<std::vector<float>,1> kolejkaPoTrucizne;

    std::vector<std::vector <float>> idTrutkaLepszych;
    std::vector<float> row_idTrutkaLepszych;

    float msg;
    *czasLamporta += 1;
    msg = *czasLamporta + (float)id/100.0;
    //REQ_Trucizna do wszystkich oprócz siebie i burmistrza;
    for(int i = 1; i < liczbaProcesow; i++){
        if(i == id)
            continue;
        MPI_Send(&msg, 1, MPI_FLOAT, i, REQ_TRUCIZNA, MPI_COMM_WORLD);
    }
    int pozwolenia_Trucizny = 1;
    int truciznaZabranaPrzezLepszych = 0;
    int procesy_lepsze = 0;
    float rcv_msg;
    while(pozwolenia_Trucizny + procesy_lepsze != liczbaProcesow-2 && truciznaZeZlecenia > TRUCIZNA-truciznaZabranaPrzezLepszych){
        MPI_Status status;
        MPI_Recv(&rcv_msg, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        float id_procesu;

        if (status.MPI_TAG == REQ_TRUCIZNA ||
            status.MPI_TAG == ACK_TRUCIZNA){
            
            float lamportInnegoProcesu = floor(rcv_msg);
            *czasLamporta = rcv_msg > msg ? lamportInnegoProcesu:*czasLamporta;
            msg = *czasLamporta + (float)id/100.0;

            switch(status.MPI_TAG){
                case REQ_TRUCIZNA:
                    if(msg > rcv_msg){
                        pozwolenia_Trucizny++;
                        id_procesu = rcv_msg;
                        while (id_procesu > 1){
                            id_procesu--;
                        }
                        id_procesu = id_procesu*100;
                        //zapis id procesu od którego dostał wiadomość do oczekujacych na ACK_TRUCIZNA
                        kolejkaPoTrucizne[0].push_back(id_procesu);

                        //wysłanie wiadomości LEPSZY do gorszego procesu z info o ilości trucizny
                        float idTrucizna = truciznaZeZlecenia + (float)id/100.0;
                        MPI_Send(&idTrucizna,1,MPI_FLOAT, (int)id_procesu, LEPSZY, MPI_COMM_WORLD);
                    } else {
                        //wysłanie ACK_Trucizna do procesu który przysłał wiadomość
                        id_procesu = rcv_msg;
                        while(id_procesu > 1){
                            id_procesu--;
                        }
                        id_procesu = id_procesu*100;
                        MPI_Send(&msg, 1, MPI_FLOAT, (int)id_procesu,ACK_AGRAFKA,MPI_COMM_WORLD);
                    }
                    break;
                case ACK_TRUCIZNA:
                    //jeśli ACK_TRUCIZNA od procesu od którego dostał LEPSZY
                    pozwolenia_Trucizny++;
                    //sprawdzenie czy proces od którego dostał ACK wysłał mu wcześniej wiadomość LEPSZY
                    id_procesu = rcv_msg;
                        while(id_procesu > 1){
                            id_procesu--;
                        }
                    id_procesu = id_procesu*100;
                    for (int i = 0; i < idTrutkaLepszych.size(); i++){
                        if(id_procesu == idTrutkaLepszych[i][0]){
                            truciznaZabranaPrzezLepszych -= idTrutkaLepszych[i][1];
                            procesy_lepsze--;
                        }
                    }
                    break;
           } 
        }else if (status.MPI_TAG == REQ_AGRAFKA ||
                  status.MPI_TAG == ACK_AGRAFKA){

            float lamportInnegoProcesu = floor(rcv_msg);
            *czasLamporta = rcv_msg > msg ? lamportInnegoProcesu:*czasLamporta;
            msg = *czasLamporta + (float)id/100.0;

            switch (status.MPI_TAG)
            {
            case REQ_AGRAFKA:
                //wyślij ACK_AGRAFKA
                id_procesu = rcv_msg;
                while (id_procesu > 1){
                    id_procesu--;
                }
                id_procesu = id_procesu*100;
                MPI_Send(&msg, 1, MPI_FLOAT, id_procesu, ACK_AGRAFKA, MPI_COMM_WORLD);
                break;
            case ACK_AGRAFKA:
                //ignore
                break;
            }
        } else if (status.MPI_TAG == MOJ_PRIORYTET ||
                   status.MPI_TAG == LEPSZY){
            switch (status.MPI_TAG)
            {
            case MOJ_PRIORYTET:
                for(int i = 0; i < wielkosctabPrioZlecen; i++){
                    if(tabPriorytetyZlecen[i] == 2020){
                        tabPriorytetyZlecen[i] == rcv_msg;
                        break;
                    }
                }
                break;

            case LEPSZY:
                float idLepszegoProcesu = rcv_msg;
                while (idLepszegoProcesu > 1){
                    idLepszegoProcesu--;
                }
                idLepszegoProcesu = idLepszegoProcesu*100;
                float truciznaLepszegoProcesu = floor(rcv_msg);

                procesy_lepsze++;
                truciznaZabranaPrzezLepszych += truciznaLepszegoProcesu;

                row_idTrutkaLepszych.push_back(idLepszegoProcesu);
                row_idTrutkaLepszych.push_back(truciznaLepszegoProcesu);
                idTrutkaLepszych.push_back(row_idTrutkaLepszych);
                break;
            }
        } else if (status.MPI_TAG == CZEKAM_NA_ZLECENIA){
            //odebralem, zignorowalem
        }
    }

    /*
    //##debug printf
    printf("STAN TRUCIZNY: ");
    for (int i = 0; i < wielkosctabPrioZlecen; i++){
        printf("%.2f, ",tabPriorytetyZlecen[i]);
        if(i == wielkosctabPrioZlecen - 1)
            printf("\n");
    }
    */

    printf("Skrzat %d: Mam truciznę, idę zabijać. Mój czas lamporta: %.2f\n",id, *czasLamporta);
    return kolejkaPoTrucizne;
}

void chomikiZabite(int id, int liczbaProcesow, float *czasLamporta, int idZlecenia){
    printf("Skrzat %d: Chomiki zabite. Wykonano zlecenie nr: %d\n",id, idZlecenia);
    *czasLamporta += 1;
    int msg = 1;
    MPI_Send(&msg, 1, MPI_INT, 0 , END, MPI_COMM_WORLD);
}

zlecenie* generujZlecenia(int ilosc){
    zlecenie *tabZlecen = new zlecenie[ilosc];
    srand((unsigned)time(0));
    for(int i = 0; i < ilosc; i++){
        tabZlecen[i].id = i;
        tabZlecen[i].chomiki = 1 + (rand() % TRUCIZNA);
    }
    return tabZlecen;
}

void zostalemBurmistrzem(int id, int liczbaProcesow){
    char decyzja = 'a';
    printf("Starosto, czy wygenerować zlecenia(y/n)?\n");
    //fflush(stdin);
    //scanf(" %c",&decyzja);
    //printf("Zdecydowales: %c\n",decyzja);
    std::cin >> decyzja;
    std::cout << "Zdecydowałeś: " << decyzja << std::endl;
    
    if(decyzja=='y'){
        zlecenie* wygenerowaneZlecenia = generujZlecenia(ZLECENIA);
        for (int i = 0; i < ZLECENIA; i++){
            for(int proces = 1; proces < liczbaProcesow; proces++)
                MPI_Send(&wygenerowaneZlecenia[i], sizeof(wygenerowaneZlecenia[i]),MPI_CHAR,proces,START,MPI_COMM_WORLD);
        }
        printf("Zostalem burmistrzem z id %d, wygenerowalem i wysłałem zlecenia.\n", id);
        delete[] wygenerowaneZlecenia;
    } else if (decyzja=='n'){
        MPI_Finalize();
        exit;
    }
}

int main(int argc, char **argv){
    int id, liczbaProcesow;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &liczbaProcesow); //ilosc watkow
    MPI_Comm_rank( MPI_COMM_WORLD, &id); //przypisanie indywidualnego id

    float czasLamporta = 0;
    
    kolejneZlecenia:
    if(id==0){
        zostalemBurmistrzem(id,liczbaProcesow);
        int liczbaZakonczonychZlecen = 0;
        int rcv_msg;
        while (liczbaZakonczonychZlecen < ZLECENIA){
            MPI_Status status;
            MPI_Recv(&rcv_msg, 1, MPI_INT, MPI_ANY_SOURCE, END, MPI_COMM_WORLD,&status);
            if (status.MPI_TAG == END){
                liczbaZakonczonychZlecen += 1;
            }
        }
        goto kolejneZlecenia;
    } 
    else {
        if (ZLECENIA > liczbaProcesow){
            //#################
            brakWolnychZlecen: 
            //#################
            float tab_PriorytetyZlecen[ZLECENIA];
            for (int i = 0; i < ZLECENIA; i++){
                tab_PriorytetyZlecen[i] = 2020; //oznaczenie zlecenia wolnego
            }
            zlecenie* przeslaneZlecenia = czekajNaZlecenia(id, liczbaProcesow);

            //#################
            wolneZlecenia:
            //#################
            int wybraneZlecenie = ubiegajOZlecenie(id, liczbaProcesow, &czasLamporta, tab_PriorytetyZlecen, przeslaneZlecenia, ZLECENIA);
            if (wybraneZlecenie == -1){
                goto brakWolnychZlecen;
            }
            float idZlecenia,chomikiDoUbicia;
            if (wybraneZlecenie + 1 > ZLECENIA){
                printf("Dla procesu %d zabraklo zlecen. Czeka...",id);
                goto brakWolnychZlecen;
            }
            else {
                for(int i = 0; i < ZLECENIA; i++){
                    if(i == wybraneZlecenie){
                        idZlecenia = przeslaneZlecenia[i].id;
                        chomikiDoUbicia = przeslaneZlecenia[i].chomiki;
                        printf("Skrzat %d: Wybrałem zlecenie %d. Muszę ubić %d chomików. Mój czas lamporta: %.2f\n", id, (int)idZlecenia, (int)chomikiDoUbicia, czasLamporta);
                        
                        auto kolejkaPoAgrafke = ubiegajOAgrafke(id,liczbaProcesow,&czasLamporta, tab_PriorytetyZlecen, ZLECENIA);
                        auto kolejkaPoTrucizne = ubiegajOTrucizne(id, liczbaProcesow, &czasLamporta, chomikiDoUbicia, tab_PriorytetyZlecen, ZLECENIA);

                        //zwolnienie zabranych zasobów
                        for(auto idProcesu = kolejkaPoAgrafke[0].cbegin(); idProcesu != kolejkaPoAgrafke[0].cend(); ++idProcesu){
                            float msg = czasLamporta + (float)id/100.0;
                            MPI_Send(&msg, 1, MPI_FLOAT, *idProcesu, ACK_AGRAFKA, MPI_COMM_WORLD);
                        }
                        for(auto idProcesu = kolejkaPoTrucizne[0].cbegin(); idProcesu != kolejkaPoTrucizne[0].cend(); ++idProcesu){
                            float msg = czasLamporta + (float)id/100.0;
                            MPI_Send(&msg, 1, MPI_FLOAT, *idProcesu, ACK_TRUCIZNA, MPI_COMM_WORLD);
                        }
                        chomikiZabite(id, liczbaProcesow, &czasLamporta, idZlecenia);
                    }
                }
            }
            //sprawdz czy są wolne zlecenia
            for (int i = 0; i < ZLECENIA; i++){
                if(tab_PriorytetyZlecen[i] == 2020){ //jeśli są wolne
                    goto wolneZlecenia;
                    break;
                }
                else if(i == ZLECENIA - 1){
                    goto brakWolnychZlecen;
                }
            }
        }
        else if (ZLECENIA <= liczbaProcesow){
            brakWolnychZlecen_WiecejProcesow:
            float tab_PriorytetyZlecen[liczbaProcesow-1];
            for (int i = 0; i < liczbaProcesow-1; i++){
                tab_PriorytetyZlecen[i] = 2020; //oznaczenie zlecenia wolnego
            }
            zlecenie* przeslaneZlecenia = czekajNaZlecenia(id, liczbaProcesow);
            int wybraneZlecenie = ubiegajOZlecenie(id, liczbaProcesow, &czasLamporta, tab_PriorytetyZlecen, przeslaneZlecenia, liczbaProcesow-1);
            float idZlecenia,chomikiDoUbicia;

            if (wybraneZlecenie + 1 > ZLECENIA){
                printf("Dla procesu %d zabraklo zlecen. Czeka...\n",id);
                goto brakWolnychZlecen_WiecejProcesow;
            } else {
                for(int i = 0; i < ZLECENIA; i++){
                    if(i == wybraneZlecenie){
                        idZlecenia = przeslaneZlecenia[i].id;
                        chomikiDoUbicia = przeslaneZlecenia[i].chomiki;
                        printf("Skrzat %d: Wybrałem zlecenie %d. Muszę ubić %d chomików. Mój czas lamporta: %.2f\n", id, (int)idZlecenia, (int)chomikiDoUbicia, czasLamporta);
                        auto kolejkaPoAgrafke = ubiegajOAgrafke(id, liczbaProcesow, &czasLamporta, tab_PriorytetyZlecen, liczbaProcesow-1);
                        auto kolejkaPoTrucizne = ubiegajOTrucizne(id, liczbaProcesow, &czasLamporta, chomikiDoUbicia, tab_PriorytetyZlecen, liczbaProcesow-1);

                        //zwolnienie zabranych zasobów
                        for(auto idProcesu = kolejkaPoAgrafke[0].cbegin(); idProcesu != kolejkaPoAgrafke[0].cend(); ++idProcesu){
                            float msg = czasLamporta + (float)id/100.0;
                            MPI_Send(&msg, 1, MPI_FLOAT, *idProcesu, ACK_AGRAFKA, MPI_COMM_WORLD);
                        }
                        for(auto idProcesu = kolejkaPoTrucizne[0].cbegin(); idProcesu != kolejkaPoTrucizne[0].cend(); ++idProcesu){
                            float msg = czasLamporta + (float)id/100.0;
                            MPI_Send(&msg, 1, MPI_FLOAT, *idProcesu, ACK_TRUCIZNA, MPI_COMM_WORLD);
                        }
                        chomikiZabite(id, liczbaProcesow, &czasLamporta, idZlecenia);
                        goto brakWolnychZlecen_WiecejProcesow;
                    }
                }            
            }    
        }
    }
    MPI_Finalize();
    return 0;
}