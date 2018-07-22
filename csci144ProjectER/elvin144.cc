#include <iostream>
#include <pthread.h>
#include <cstdlib>
//to set decimal correctly
#include <iomanip>
//sleep
#include <unistd.h>
//write to file
#include <fstream>
//string to int
#include <sstream>
#include <semaphore.h>
//for random
#include <time.h>
using namespace std;

// function prototypes

//generate a value in random
int gsell();
// display
void printPY();
//display balace
void *balancep(void*);
//make random stock names
string gstock();
void *stockBUY(void*);
//generate how much shares
int gshares();
void *stockSELL(void*);
int gcost();


int balance = 10000;

int const MAX = 10000;

int run = MAX;

bool stockempty = 0;

bool donebuying = 0;

bool doneSelling = 0;
//thread to buy
pthread_t buy;
//thread to sell
pthread_t sell;
//thread to display
pthread_t pt;
sem_t semaphore;

string name[MAX];
int cost[MAX];
int shares[MAX];
double percentYield[MAX];

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var = PTHREAD_COND_INITIALIZER;

int main()
{

	sem_init(&semaphore, 0, 1); //init\

	srand(time(NULL)); //randomize

	ofstream myfile;

	// Generate random stocks into stockfilehere.txt file
	myfile.open("stockfilehere.txt");
	for (int i = 0; i < MAX; i++)
	{
		myfile << gstock() << endl;
		myfile << gcost() << endl;
		myfile << gshares() << endl;
	}
	myfile.close();


	pthread_create(&buy, NULL, stockBUY, NULL);
	pthread_create(&sell, NULL, stockSELL, NULL);
	pthread_create(&pt, NULL, balancep, NULL);

	//go in
	pthread_join(buy, NULL);
	pthread_join(sell, NULL);
	pthread_join(pt, NULL);

	return 0;
}

void *stockBUY(void *)
{
	int i = 0;
	ifstream myfile("stockfilehere.txt");
	string name_, tempCost, tempShares;
	int cost_, shares_, totalCost;

	pthread_mutex_lock(&count_mutex);	// get lock
	while (run > 0)
	{
		// info stockfilehere.txt file
		getline(myfile, name_);
		getline(myfile, tempCost);
		getline(myfile, tempShares);
        // convert tempCost to int
		stringstream convert(tempCost);
        //  cost to int value
		convert >> cost_;
		stringstream convert2(tempShares);
        //shares to int
        // have int value
		convert2 >> shares_;


		totalCost = cost_ * shares_;


		while (balance < cost_ && stockempty == 0)
		{
			//Signal2 = 1;
			pthread_cond_signal(&condition_var);	// signal sell thread
			pthread_cond_wait(&condition_var, &count_mutex);	// wait for sell thread to signal
		}

		// If we cannot afford to buy all shares decrement by 1 till we can
		while (totalCost > balance && shares_ != 0)
		{
			totalCost -= cost_;
			shares_--;
		}

		// If we can afford to buy stock and shares are available buy shares
		if (balance >= cost_ && shares_ != 0)
		{
			// BUY stock shares cost
			cout << "BUY " << name_ << " " << shares_ << " " << cost_ << endl;

			sem_wait(&semaphore);
			balance -= totalCost;

			name[i] = name_;
			cost[i] = cost_;
			shares[i] = shares_;
			stockempty = 0;
			sem_post(&semaphore);
		}
		else
			cout << "INSUFFICIENT FUNDS TO BUY" << name_ << " STOCK" << endl;

		i++;
		run--;
		sleep(2);
	}
	donebuying = 1;
	pthread_cond_signal(&condition_var);
	pthread_mutex_unlock(&count_mutex);	// release lock
	pthread_exit(NULL);	// exit thread
}

void *stockSELL(void *pass)
{
	int i = 0, greaterThan, lesserThan, sell;
	double totalSell, totalCost;
	pthread_mutex_lock(&count_mutex);
	while (run > 0)
	{

		while (cost[i] == NULL && donebuying == 0)
		{
			stockempty = 1;
			pthread_cond_signal(&condition_var);
			pthread_cond_wait(&condition_var, &count_mutex);
		}


		if (cost[i] == NULL)
		{
			doneSelling = 1;
			pthread_mutex_unlock(&count_mutex);
			pthread_exit(NULL);
		}

		greaterThan = cost[i] * 1.2;
		lesserThan = cost[i] * 0.8;
        // generate sales
		sell = gsell();
        //how much we paid
		totalCost = cost[i] * shares[i];
        //total price
		totalSell = sell * shares[i];

		//sell if greater
		if (sell > greaterThan || sell < lesserThan)
		{
			/
			cout << "SELL " << name[i] << " " << shares[i] << " " << cost[i] << endl;
			sem_wait(&semaphore);
			balance += totalSell;

			if (totalCost > totalSell)
				totalCost = totalCost * -1;

			// get the percent yield divide cost by the sale
			percentYield[i] = totalCost / totalSell;
			sem_post(&semaphore);
		}

		//if yes, buy
		if (balance > 0)
		{

			pthread_cond_signal(&condition_var);
		}

		i++;
		run--;
		sleep(2);
	}
	doneSelling = 1;
	pthread_cond_signal(&condition_var);
	pthread_mutex_unlock(&count_mutex);
	pthread_exit(NULL);
}
//random stocks
int gshares()
{
	return rand() % 100 + 1;
}
// cost per share
int gsell()
{
	return rand() % 2000 + 1;
}

// returns random cost per share
int gcost()
{
	return rand() % 1000 + 1;
}

//make random stock names
string gstock()
{
	string stocks[5] = { "YHOO", "TSLA", "PDFS", "HPQ", "IBM" };
	return stocks[rand() % 5];
}

void *balancep(void*)
{
	while (donebuying == 0 || doneSelling == 0)
	{
		// sleep for 10 seconds
		sleep(10);

		sem_wait(&semaphore);
		cout << "------------------------" << endl;
		cout << "PRINT BALANCE: " << balance << endl;
		printPY();
		sem_post(&semaphore);
	}

	// sleep for 10 seconds
	sleep(9);
	sem_wait(&semaphore);
	cout << "------------------------" << endl;
	cout << "PRINT BALANCE: " << balance << endl;
	printPY();
	sem_post(&semaphore);

	pthread_exit(NULL);
}


void printPY()
{
	int i = 0;
	do
	{
		cout << "Name of Stock: " << name[i] << endl;
		cout << "The percent yield : " << setprecision(3) << percentYield[i] << endl;
		cout << "# of shares : " << shares[i] << endl;
		i++;
	} while (!(name[i + 1].empty()));
	cout << "------------------------" << endl;
}
