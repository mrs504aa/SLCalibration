#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>
#include <numbers>
#include "RapidCSV.h"

using namespace std;

double BandEnergy(double Delta_c, double Omega, double k, int Flag)
{
    return 0.5 * (Delta_c + sqrt(8.0 * (1.0 + cos(k * 2.0)) * pow(Omega, 2) + pow(Delta_c, 2)) * Flag);
}

int GenerateDatabase()
{
    vector<double> RatioArray(20000, 0.0);
    vector<double> BCArray(20000, 0.0);
    double k, SumResult;

    for (int i = 0; i < 20000; i++)
    {
        RatioArray[i] = double(2.0 * i / 20000);
    }

    for (int i = 0; i < 20000; i++)
    {
        SumResult = 0.0;
        for (int j = 0; j < 1000; j++)
        {
            k = numbers::pi * j / 1000;
            SumResult += BandEnergy(1.0, RatioArray[i], k, 1);
        }
        SumResult /= 1000;
        BCArray[i] = SumResult;
    }

    ofstream DatabaseFile;
    DatabaseFile.open("Database.csv");
    DatabaseFile.precision(15);
    for (int i = 0; i < 20000; i++)
    {
        DatabaseFile << RatioArray[i];
        DatabaseFile << ", ";
        DatabaseFile << BCArray[i];
        DatabaseFile << "\n";
    }
    DatabaseFile.close();

    return 0;
}

bool isNumber(const string &s)
{
    int PointCount = 0;
    for (char const &ch : s)
    {
        if ((isdigit(ch) == 0) && (ch != '.'))
            return false;

        if (ch == '.')
            PointCount += 1;

        if (PointCount > 1)
            return false;
    }
    return true;
}

double Interpolate(vector<double> &xData, vector<double> &yData, double x, bool extrapolate)
{
    int size = xData.size();

    int i = 0;
    if (x >= xData[size - 2])
    {
        i = size - 2;
    }
    else
    {
        while (x > xData[i + 1])
            i++;
    }
    double xL = xData[i], yL = yData[i], xR = xData[i + 1], yR = yData[i + 1];
    if (!extrapolate)
    {
        if (x < xL)
            yR = yL;
        if (x > xR)
            yL = yR;
    }

    double dydx = (yR - yL) / (xR - xL);

    return yL + dydx * (x - xL);
}

int main()
{
    bool Flag = false, ExistFlag;
    string Str;
    double BandCenter, Delta_c;

    ExistFlag = filesystem::exists("Database.csv");

    if (ExistFlag == false)
    {
        cout << "Generating New Database" << endl;
        GenerateDatabase();
    }

    rapidcsv::Document doc("Database.csv", rapidcsv::LabelParams(-1, -1));
    vector<double> Ratio = doc.GetColumn<double>(0);
    vector<double> BC = doc.GetColumn<double>(1);

    while (Flag == false)
    {
        cout << "-------------------------------------------" << endl;

        cout << "Input \"exit\" to exit." << endl;
        cout << "Input Peak Position, unit Gamma or MHz:" << endl;
        cin >> Str;

        if (Str == "exit")
            break;
        if (!isNumber(Str))
        {
            cout << "not a number" << endl;
            continue;
        }
        BandCenter = stof(Str);

        ////////////////////////////////////////////////////////////

        cout << "Input Laser Detuning, unit Gamma or MHz:" << endl;
        cin >> Str;

        if (Str == "exit")
            break;
        if (!isNumber(Str))
        {
            cout << "not a number" << endl;
            continue;
        }
        Delta_c = stof(Str);

        ////////////////////////////////////////////////////////////

        double min = *min_element(BC.begin(), BC.end());
        double max = *max_element(BC.begin(), BC.end());

        if (BandCenter / Delta_c <= min)
        {
            cout << "Too small number !! Plz Remake database!" << endl;
            continue;
        }
        if (BandCenter / Delta_c >= max)
        {
            cout << "Too small number !! Plz Remake database!" << endl;
            continue;
        }

        ////////////////////////////////////////////////////////////

        double Result = Interpolate(BC, Ratio, BandCenter / Delta_c, false);
        cout << "The Rabi Frequency is " << Result * Delta_c << " Gamma or MHz." << endl;
    }

    return 0;
}