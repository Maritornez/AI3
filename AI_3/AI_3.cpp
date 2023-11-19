#include <iostream>

using namespace std;

const int MaxTowns = 30;             // число городов
const int MaxAnts = 30;              // число муравьев
double Alfa = 1.0;                   // вес фермента
double Beta = 5.0;                   // эвристика
double Rho = 0.5;                    // испарение
double Q = 100.0;                    // константа
double InitOdor = 1.0 / MaxTowns;    // начальный запах
int    MaxWay = 100;                 // предел координат
int    MaxTour = MaxTowns * MaxWay;  // предел пути
int    MaxTime = 20 * MaxTowns;      // предел итераций

using TVector = int[MaxTowns];

using TMatrix = double[MaxTowns][MaxTowns];

using TTown = struct {  // город
    double x;      // абсцисса города
    double y;      // ордината города
};

using TAnt = struct {  // муравей
    int TekTown;   // текущий город
    TVector Tabu;  // список табу
    TVector Path;  // маршрут муравья
    int NumTown;   // число городов в маршруте
    double Len;    // общая длина пути
};


TTown   Towns[MaxTowns];  // города
TAnt    Ants[MaxAnts];    // муравьи
TMatrix DistMap;          // карта расстояний
TMatrix OdorMap;          // карта запахов
TAnt    Best;             // лучший путь


double Random(double min, double max)  // случайное число от min до max
{
    return (double)(rand()) * (max - min) / RAND_MAX + min;
}


void MakeTowns()  // создание городов
{
    int i, j;
    // создание городов
    for (i = 0; i < MaxTowns; i++) {
        Towns[i].x = Random(0, MaxWay - 1);
        Towns[i].y = Random(0, MaxWay - 1);
        for (j = 0; j < MaxTowns; j++) OdorMap[i][j] = InitOdor;
        DistMap[i][i] = 0.0;
    }
    // вычисление расстояний между городами    
    for (i = 0; i < MaxTowns - 1; i++) {
        for (j = i + 1; j < MaxTowns; j++) {
            double xd = Towns[i].x - Towns[j].x;
            double yd = Towns[i].y - Towns[j].y;
            DistMap[i][j] = sqrt(xd * xd + yd * yd);
            DistMap[j][i] = DistMap[i][j];
        }
    }
}


void MakeAnts(int r)  // 0-создание, 1-повторное создание муравьев
{
    int k = 0;  // текущий город
    for (int i = 0; i < MaxAnts; i++) {
        if ((r > 0) && (Ants[i].Len < Best.Len)) Best = Ants[i];
        Ants[i].TekTown = k;  k++;  if (k >= MaxTowns) k = 0;
        for (int j = 0; j < MaxTowns; j++) {
            Ants[i].Tabu[j] = 0;
            Ants[i].Path[j] = 0;
        }
        Ants[i].Tabu[Ants[i].TekTown] = 1;
        Ants[i].Path[0] = Ants[i].TekTown;
        Ants[i].NumTown = 1;
        Ants[i].Len = 0;
    }
}


double Chance(int i, int j)  // вероятность грани
{
    return pow(OdorMap[i][j], Alfa) * pow((1.0 / DistMap[i][j]), Beta);
}


int NextTown(int k)  // выбор следующего города
{
    int i = Ants[k].TekTown;   // текущий город
    int j;                     // следующий город
    double d = 0.0;            // суммарная вероятность
    double p;                  // текущая вероятность    
    for (j = 0; j < MaxTowns; j++)
        if (Ants[k].Tabu[j] == 0) d += Chance(i, j);
    j = MaxTowns - 1;
    if (d > 0.0)
        do {
            j++;  if (j >= MaxTowns) j = 0;
            if (i != j) p = Chance(i, j) / d;
        } while ((Ants[k].Tabu[j] != 0) || (Random(0, 1) > p));
        return j;
}


bool AntsMoving()  // перемещение муравьев
{
    bool m = false;   // флаг перемещений    
    for (int k = 0; k < MaxAnts; k++) {   // k-номер муравья
        if (Ants[k].NumTown < MaxTowns) {
            int Next = NextTown(k);  // следующий город
            Ants[k].Path[Ants[k].NumTown] = Next;
            Ants[k].NumTown++;
            Ants[k].Tabu[Next] = 1;
            Ants[k].Len += DistMap[Ants[k].TekTown][Next];
            if (Ants[k].NumTown == MaxTowns)
                Ants[k].Len += DistMap[Ants[k].Path[MaxTowns - 1]][Ants[k].Path[0]];
            Ants[k].TekTown = Next;  m = true;
        }
    }
    return m;
}


void UpdateOdors()  // испарение и нанесение фермента
{
    int i, j, k, ant;
    for (i = 0; i < MaxTowns; i++)         // испарение фермента
        for (j = 0; j < MaxTowns; j++)
            if (i != j) {
                OdorMap[i][j] *= (1 - Rho);
                if (OdorMap[i][j] < InitOdor) OdorMap[i][j] = InitOdor;
            }
    for (ant = 0; ant < MaxAnts; ant++) {  // нанесение фермента
        for (k = 0; k < MaxTowns; k++) {
            i = Ants[ant].Path[k];
            if (k < MaxTowns - 1) j = Ants[ant].Path[k + 1];
            else j = Ants[ant].Path[0];
            OdorMap[i][j] += Q / Ants[ant].Len;
            OdorMap[j][i] = OdorMap[i][j];
        }
    };
    for (i = 0; i < MaxTowns; i++)
        for (j = 0; j < MaxTowns; j++) OdorMap[i][j] *= Rho;
}


int main() {
    srand(time(0));
    //setlocale(LC_ALL, "Russian");
    int CurTime = 0;     // текущее время
    Best.Len = MaxTour;  // лучший путь  
    MakeTowns();  MakeAnts(0);
    while (CurTime < MaxTime) {
        if (!AntsMoving()) {
            UpdateOdors();  MakeAnts(1);
            cout << "Время = " << CurTime << "  Путь = " << Best.Len << endl;
        }
        CurTime++;
    }
    cout << "Оптимальный путь = " << Best.Len << endl;
    system("pause");
    return 0;
}