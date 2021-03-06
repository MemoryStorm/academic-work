// Курсовой проект.cpp : Defines the entry point for the console application.
//
//проверка github
#include "stdafx.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <ostream>
#include <random>
using namespace std;

int FeatureNumber;
bool fl=true;

struct limit 
{
	int feature;
	double value, rmse , left, right;
};

class Welford
{
public:
	float average = 0;
	int count = 0;
	void Add(float val)
	{
		++count;
		average += (val - average) / count;
	}
};



//Шаблон скользящего контроля.
template <typename T>
limit CrossValidation(int Folds, vector<vector<T>> dataset)
{
	vector<vector<T>> data;
	vector<vector<vector<T>>> CrossVal;
	vector<limit> answer,check;
	vector<T> featureleft, featuresright;
	vector<T> targright, targleft;
	limit ans1,ans2;
	shuffle(dataset.begin(), dataset.end(), mt19937_64{ random_device{}() });//64 разрядный вихрь Мерсенна
	for (int i = 0; i < Folds; ++i)
	{

		vector<vector<T>> v(dataset.begin() + i*dataset.size() / Folds, dataset.begin() + (i + 1)*dataset.size() / Folds);
		CrossVal.push_back(v);
		v.clear();

	}
	//Обучение.
	FeatureNumber = 0;

	//Перебор всех выборок в скользящем контроле.
	for (int j = 0; j < Folds; ++j)

	{

		//Cоставление выборки.
		for (int k = 0; k < Folds; ++k)

		{

			if (k != j) data.insert(data.end(), CrossVal[k].begin(), CrossVal[k].end());

		}

		//Перебор всех признаков.
		for (int i = 0; i < dataset[0].size() - 1; ++i)

		{

			answer.push_back(Decision(data, CrossVal[j]));
			++FeatureNumber;

		}


		//Выбор наилучшего признака
		ans1 = answer[0];
		for (int val = 0; val < answer.size(); ++val)
		{
			if (answer[val].rmse < ans1.rmse) ans1 = answer[val];
			
		}
		
		answer.clear();
		FeatureNumber = 0;
		data.clear();
		check.push_back(ans1);
		
	}

	//Выбор наилучшего результата для каждого разбиения.
	ans2.rmse = -1;
	for(int itera = 0; itera<Folds; ++itera)
	{
		FeatureNumber = check[itera].feature;
		for(int parts=0;parts<CrossVal[itera].size();++parts)
		{
			if (CrossVal[itera][parts][FeatureNumber] < check[itera].value)
			{
				featureleft.push_back(CrossVal[itera][parts][FeatureNumber]);
				targleft.push_back(CrossVal[itera][parts][6]);
			}
			else
			{ 
				featuresright.push_back(CrossVal[itera][parts][FeatureNumber]);
				targright.push_back(CrossVal[itera][parts][6]);
			}
		}
		
		check[itera].rmse = RMSE(targleft, targright, check[itera].left, check[itera].right);
		

		if(check[itera].rmse<ans2.rmse || ans2.rmse==-1)
		{
			ans2.rmse = check[itera].rmse;
			ans2.value = check[itera].value;
			ans2.left = check[itera].left;
			ans2.right = check[itera].right;
			ans2.feature = check[itera].feature;
		}
		
	}
	
	return ans2;

}

double Average(vector<int> values)
{
	double sum = 0;

	for (int i=0; i < values.size(); ++i)
	{sum += values[i];}

	return sum / values.size();
}


//Среднеквадратическая ошибка.
double RMSE(vector<int> values1, vector<int> values2, double a, double b)
{
	double  c = 0, d = 0;

	for(int i=0; i < values1.size(); ++i)
	{c += pow(values1[i]-a,2);}

	for (int i = 0; i < values2.size(); ++i)
	{d += pow(values2[i] - b, 2);}	

	return sqrt((c+d)/(values1.size()+values2.size()));
}

float Dispersion(vector<int> values)
{
	Welford c;
	float sum=0;

	while (c.count<values.size())
	{
		sum += values[c.count];
		c.Add(values[c.count]* values[c.count]);
	}

	return c.average - pow(sum / c.count, 2);
}

float Metric(vector<int> values1, vector<int> values2)
{
	return Dispersion(values1) + Dispersion(values2);
}


//Предикат для сортировки вектора по признаку.
bool Compare(vector<int> a, vector<int> b)
{return a[FeatureNumber] < b[FeatureNumber];}


//Обучение пня.
limit Decision(vector<vector<int>> data, vector<vector<int>> answer)
{

	limit lim;
	vector<int> left, right, targetleft, targetright;
	lim.feature = FeatureNumber;
	lim.rmse = -1;

	sort(data.begin(), data.end(), Compare);
	int cur1 = data[0][FeatureNumber];

	for(int i=0; i < data.size(); ++i)
	{
		right.push_back(data[i][FeatureNumber]);
		targetright.push_back(data[i][6]);
	}

	for(int i=1; i<data.size();++i)
	{

		left.push_back(right[0]);
		right.erase(right.begin());

		targetleft.push_back(targetright[0]);
		targetright.erase(targetright.begin());

		if(left.back() != right[0])
		{

			
			if(Metric(left,right) < lim.rmse || lim.rmse == -1)//Проверка на наличие присвоенного значения.  
			{
				lim.left = Average(targetleft);
				lim.right = Average(targetright);
				lim.rmse = Metric(left,right);
				
				lim.value = (left.back()+ right[0])/2;

			}

		}

	}


	return lim;

}


//Проверка пня.
int Check(int a)
{
	return a;
}


void main()
{
	vector<vector<int>> dataset, data;
	vector<vector<vector<int>>> CrossVal;//5 векторов-датасетов
	vector<limit> answer;
	ifstream fin;
	string str;
	limit ans;



	fin.open("input1.txt");//Отсюда и до следующего коммента чтение базы.
	if (fin)
	{

		if (!fin.eof())
		{

			int j = 0;
			vector<int> object;

			do
			{

				fin >> str;
				for (int i = 0; i < 7; ++i)
				{

					object.push_back(stoi(str.substr(0, str.find(',')).c_str()));
					str = str.substr(str.find(',') + 1);

				}

				dataset.push_back(object);
				object.clear();
				++j;

			} while (!fin.eof());

		}

	}
	//Конец чтения базы.


	ans=CrossValidation(5, dataset);
	cout << "Feature: " << ans.feature << "\n";
	cout << "Limit: " << ans.value << "\n";
	cout << "RMSE: " << ans.rmse << "\n";
	cout << "l,r " << ans.left << " " << ans.right;
	fin.close();
	system("pause");
	return;
}
