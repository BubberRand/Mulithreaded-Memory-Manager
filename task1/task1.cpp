#include <iostream> 
#include <fstream>  
#include <string> 
#include <vector>
#include <functional>
#include <map>
#include <pthread.h>
#include <semaphore.h>

using namespace std; 

//Declaring variables
int pageId;
string readWrite;
int clientId; 
string pageContents; 

int clientThreads; 
bool finished = false; 

sem_t clientSemaphore [10]; 
sem_t serverSemaphore; 

map<int, string> memoryBuffer; 

//split helper method
void split(const string& s, char c, vector<string>& v) 
{
   string::size_type i = 0;
   string::size_type j = s.find(c);

   while (j != string::npos) 
   {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

//Client Thread
void * func(void *arg) 
{
	
	//Each client thread should keep a log of the pages that it has read inside 
	//a file called “client_log_n” where n is its thread id.   
    ofstream clientFile("client_log_" + to_string(((int)(long)arg)) + ".dat");  

    while (!finished){
        sem_wait(&clientSemaphore[(int)(long)arg]); 

        if (finished)
        {
            break; 
        } 

        if (clientFile.is_open()) 
        {
            clientFile << pageId << " " <<  memoryBuffer[pageId] << endl; 
        } 

        sem_post(&serverSemaphore); 
    } 

    clientFile.close(); 
}

//Server Thread
int main (int argc, char* argv[]) 
{
	string line;
	int i;
	
	//The server thread starts and reads in the init_buffer_pages.dat file to 
	//initialise the memory buffer. 
	
	//Open buffer file
	ifstream bufferFile("init_buffer_pages.dat"); 

	if(bufferFile.is_open()) 
	{
		//Map data from buffer file
		while (getline(bufferFile, line))
		{
			vector<string> v;
			split(line, ' ', v);
			
			pageId = stoi(v[0]);
			pageContents = v[1];
			
			memoryBuffer[pageId] = pageContents; 
		}
		
		//Close buffer file
		bufferFile.close(); 
	}
	else
	{
		cout << "The file init_buffer_pages.dat could not be opened."; 
	}
	
	//Create N client threads, where the number N is taken from the command line.
	clientThreads = *argv[1] - 48; 
	pthread_t threads[clientThreads]; 

	for(i = 0; i < clientThreads; i++)
	{
		pthread_create(&threads[i], NULL, func, (void *)(long)(i)); 
	}  
	
	//The server thread reads from all_requests.dat file to get the page read or write requests.
	ifstream requestFile("all_requests.dat"); 

	if (requestFile.is_open()) 
	{
		while (getline(requestFile, line)) 
		{
			vector<string> v;
			split(line, ' ', v);
			
			clientId = stoi(v[0]);
			readWrite = v[1];
			pageId = stoi(v[2]);

			if (readWrite.compare("read") == 0) 
			{ 
				sem_post(&clientSemaphore[clientId]);
				
				//the server must wait for the current client thread to have finished writing 
				//its log entry before getting the next thread to log its read request.
				sem_wait(&serverSemaphore); 
			}
			else 
			{
				pageContents = v[3];
				memoryBuffer[pageId] = pageContents;
			}
		} 
		
		requestFile.close(); 
		
		finished = true; 

		//When all the clients finish their requests the server thread must end and cause 
		//the entire process to end.
		for ( i = 0; i < clientThreads; i++) 
		{
			sem_post(&clientSemaphore[i]); 
		}
	} 
	else 
	{	
		cout << "The file all_requests.dat could not be opened."; 
	}
	
	//Join Threads
	for (i = 0; i < clientThreads; i++) 
	{
		pthread_join(threads[i], NULL); 
	}
	return 0;
}